#include "grid.h"
#include <iostream>
#include <unordered_set>
#include <queue>
#include <set>
#include <algorithm>
using namespace std;


/* ---------------------------------------------------------------------[<]-
 Function: solve
 Synopsis: main recursive backtracking solver. applies deduction first,
           then branches on the area with fewest possible moves.
 ---------------------------------------------------------------------[>]-*/
bool Grid::solve(unordered_set<string>& seen)
{
    deduction_until_stable();

    if (is_invalid_state()) return false;
    if (is_solved()) return true;

    string state = serialize();
    if (seen.count(state)) return false;
    seen.insert(state);

    int best = choose_branch_area();

    if (best == -1 && has_zeros()) {
        Grid backup = *this;

        if (!fill_holes_by_size()) return false;

        deduction_until_stable();

        if (is_invalid_state() || (!is_solved() && (best = choose_branch_area()) == -1)) {
            *this = backup;
            return false;
        }

        if (is_solved()) return true;
    }

    if (best == -1) return false;

    auto moves = get_allowed_moves_for_area(best);
    if (moves.empty()) return false;

    moves.erase(
        remove_if(moves.begin(), moves.end(), [&](const Cell& c) {
            return move_completion_score(best, c) >= 1000000;
        }),
        moves.end()
    );

    if (moves.empty()) return false;

    sort(moves.begin(), moves.end(), [&](const Cell& a, const Cell& b) {
        return move_completion_score(best, a) < move_completion_score(best, b);
    });

    Grid backup = *this;
    for (const auto& move : moves) {
        *this = backup;
        apply_fill(best, move);
        if (solve(seen)) return true;
    }

    *this = backup;
    return false;
}


/* ---------------------------------------------------------------------[<]-
 Function: count_shape_completions_dfs
 Synopsis: counts how many ways the shape can be completed to target size
           using DFS. limited by the limit parameter to avoid slowdowns.
 ---------------------------------------------------------------------[>]-*/
int Grid::count_shape_completions_dfs(
    int value,
    vector<Cell>& shape,
    vector<vector<bool>>& used,
    int target_size,
    int limit
) const {
    if ((int)shape.size() == target_size) return 1;
    if (limit <= 0) return 0;

    vector<Cell> candidates;
    for (const auto& c : shape) {
        for (const auto& nb : neighbors(c)) {
            if (!used[nb.x][nb.y] && grid[nb.x][nb.y] == 0) {
                used[nb.x][nb.y] = true;
                candidates.push_back(nb);
            }
        }
    }
    for (const auto& c : candidates) used[c.x][c.y] = false;

    int total = 0;
    for (const auto& next : candidates) {
        used[next.x][next.y] = true;
        shape.push_back(next);

        total += count_shape_completions_dfs(value, shape, used, target_size, limit - total);

        shape.pop_back();
        used[next.x][next.y] = false;

        if (total >= limit) break;
    }

    return total;
}


/* ---------------------------------------------------------------------[<]-
 Function: count_shape_completions_after_move
 Synopsis: counts completions for an area after hypothetically
           placing a move cell.
 ---------------------------------------------------------------------[>]-*/
int Grid::count_shape_completions_after_move(int areaIndex, const Cell& move, int limit) const
{
    if (!areas[areaIndex].alive) return 0;
    if (grid[move.x][move.y] != 0) return 0;

    int value = areas[areaIndex].value;
    vector<Cell> shape = areas[areaIndex].cells;

    vector<vector<bool>> used(n, vector<bool>(m, false));
    for (const auto& c : shape) used[c.x][c.y] = true;

    if (used[move.x][move.y]) return 0;

    used[move.x][move.y] = true;
    shape.push_back(move);

    if ((int)shape.size() > value) return 0;

    return count_shape_completions_dfs(value, shape, used, value, limit);
}


/* ---------------------------------------------------------------------[<]-
 Function: move_completion_score
 Synopsis: scores a move based on how many shape completions it allows.
           lower score = better move. returns 1000000 if move is dead end.
 ---------------------------------------------------------------------[>]-*/
int Grid::move_completion_score(int areaIndex, const Cell& move) const
{
    int need = areas[areaIndex].value - (int)areas[areaIndex].cells.size();
    int completions = count_shape_completions_after_move(areaIndex, move, min(20, need + 1));

    if (completions == 0) return 1000000;

    int open = 0;
    for (const auto& nb : neighbors(move)) {
        if (grid[nb.x][nb.y] == 0) open++;
    }

    return completions * 10 + open;
}


/* ---------------------------------------------------------------------[<]-
 Function: is_invalid_state
 Synopsis: checks if current grid state is invalid - any area that
           cannot reach enough empty cells to complete.
 ---------------------------------------------------------------------[>]-*/
bool Grid::is_invalid_state() const {
    for (int i = 0; i < (int)areas.size(); i++) {
        if (!areas[i].alive) continue;
        if ((int)areas[i].cells.size() > areas[i].value) return true;

        int need = areas[i].value - (int)areas[i].cells.size();
        if (need <= 0) continue;

        int localFree = 0;
        for (const auto& c : areas[i].cells) {
            for (const auto& nb : neighbors(c)) {
                if (grid[nb.x][nb.y] == 0) localFree++;
            }
        }
        if (localFree == 0) return true; 

        int reachable = count_reachable_cells(i, grid);
        if (reachable < need) return true;
    }
    return false;
}


// checks if all cells are filled and all areas have correct size
bool Grid::is_solved() const {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (grid[i][j] == 0) return false;
        }
    }

    for (int i = 0; i < (int)areas.size(); i++) {
        if (!areas[i].alive) continue;
        if ((int)areas[i].cells.size() != areas[i].value) return false;
    }

    return true;
}


/* ---------------------------------------------------------------------[<]-
 Function: choose_branch_area
 Synopsis: selects the area with fewest allowed moves for branching.
           this minimizes the search tree size.
 ---------------------------------------------------------------------[>]-*/
int Grid::choose_branch_area() const
{
    int best = -1;
    int bestMoves = INT_MAX;

    for (int i = 0; i < (int)areas.size(); i++)
    {
        if (!areas[i].alive) continue;
        if (is_area_finished(i)) continue;

        auto moves = get_allowed_moves_for_area(i);

        if (moves.empty()) continue;

        int cnt = (int)moves.size();

        if (cnt < bestMoves)
        {
            bestMoves = cnt;
            best = i;
        }
    }

    return best;
}


// returns all allowed moves for a given area
vector<Cell> Grid::get_allowed_moves_for_area(int areaIndex) const {
    vector<Cell> allowed;
    if (!areas[areaIndex].alive) return allowed;
    if (is_area_finished(areaIndex)) return allowed;

    auto frontier = get_area_frontier(areaIndex);

    for (const auto& c : frontier) {
        if (move_is_allowed(areaIndex, c)) {
            allowed.push_back(c);
        }
    }

    return allowed;
}


// creates a new area from a list of cells with value = cell count
void Grid::create_new_area_from_cells(const vector<Cell>& cells) {
    if (cells.empty()) return;

    int value = (int)cells.size();

    Area a;
    a.id = (int)areas.size();
    a.value = value;
    a.alive = true;

    for (const auto& c : cells) {
        grid[c.x][c.y] = value;
        area_id[c.x][c.y] = a.id;
        a.cells.push_back(c);
    }

    areas.push_back(a);
}


// returns true if grid has any unfilled cells
bool Grid::has_zeros() const {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (grid[i][j] == 0) return true;
        }
    }
    return false;
}


/* ---------------------------------------------------------------------[<]-
 Function: fill_holes_by_size
 Synopsis: finds isolated groups of empty cells and creates areas from them.
           returns false if any group conflicts with an adjacent area.
 ---------------------------------------------------------------------[>]-*/
bool Grid::fill_holes_by_size() {
    if (!has_zeros()) return false;

    vector<vector<bool>> vis(n, vector<bool>(m, false));
    bool changed = false;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (grid[i][j] != 0 || vis[i][j]) continue;

            vector<Cell> comp;
            queue<Cell> q;
            q.push({i, j});
            vis[i][j] = true;

            while (!q.empty()) {
                Cell cur = q.front(); q.pop();
                comp.push_back(cur);
                for (const auto& nb : neighbors(cur)) {
                    if (!vis[nb.x][nb.y] && grid[nb.x][nb.y] == 0) {
                        vis[nb.x][nb.y] = true;
                        q.push(nb);
                    }
                }
            }

            int newValue = (int)comp.size();

            bool hasConflict = false;
            for (auto& hc : comp) {
                for (auto& nb : neighbors(hc)) {
                    if (grid[nb.x][nb.y] == newValue) {
                        hasConflict = true;
                        break;
                    }
                }
                if (hasConflict) break;
            }

            if (hasConflict) return false; 

            create_new_area_from_cells(comp);
            changed = true;
        }
    }

    return changed;
}


// serializes grid to string for state deduplication
string Grid::serialize() const {
    string s;
    s.reserve(n * m * 2);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            s += (char)grid[i][j];
    return s;
}