#include "grid.h"
#include <iostream>
#include <queue>
using namespace std;


// constructor - initializes grid and finds all existing areas
Grid::Grid(const vector<vector<int>>& start) : grid(start) {
    n = grid.size();
    m = grid[0].size();

    area_id.assign(n, vector<int>(m, -1));

    vis_token.assign(n, vector<int>(m, 0));
    current_token = 1;

    last_progress_time = chrono::steady_clock::now();
    init_areas();
}


/* ---------------------------------------------------------------------[<]-
 Function: init_areas
 Synopsis: finds all existing number regions using BFS
           and registers them as areas.
 ---------------------------------------------------------------------[>]-*/
void Grid::init_areas() {
    areas.clear();
    int id = 0;

    vector<vector<bool>> vis(n, vector<bool>(m, false));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {

            if (grid[i][j] <= 0 || vis[i][j]) continue;

            int value = grid[i][j];

            Area a;
            a.id = id;
            a.value = value;
            a.alive = true;

            queue<Cell> q;
            q.push({i, j});
            vis[i][j] = true;

            while (!q.empty()) {
                Cell cur = q.front(); q.pop();

                a.cells.push_back(cur);
                area_id[cur.x][cur.y] = id;

                for (auto& nb : neighbors(cur)) {
                    if (!vis[nb.x][nb.y] && grid[nb.x][nb.y] == value) {
                        vis[nb.x][nb.y] = true;
                        q.push(nb);
                    }
                }
            }

            areas.push_back(a);
            id++;
        }
    }
}



bool Grid::in_bounds(int x, int y) const {
    return x >= 0 && y >= 0 && x < n && y < m;
}


//return neighbors areas from all sides
vector<Cell> Grid::neighbors(const Cell& c) const {
    vector<Cell> res;
    if (in_bounds(c.x + 1, c.y)) res.push_back({c.x + 1, c.y});
    if (in_bounds(c.x - 1, c.y)) res.push_back({c.x - 1, c.y});
    if (in_bounds(c.x, c.y + 1)) res.push_back({c.x, c.y + 1});
    if (in_bounds(c.x, c.y - 1)) res.push_back({c.x, c.y - 1});
    return res;
}



bool Grid::is_area_finished(int areaIndex) const {
    if (!areas[areaIndex].alive) return true;
    return (int)areas[areaIndex].cells.size() == areas[areaIndex].value;
}


/* ---------------------------------------------------------------------[<]-
 Function: get_area_frontier
 Synopsis: returns list of empty cells adjacent to the given area.
 ---------------------------------------------------------------------[>]-*/
vector<Cell> Grid::get_area_frontier(int areaIndex) const {
    vector<Cell> frontier;
    if (!areas[areaIndex].alive) return frontier;
    if (is_area_finished(areaIndex)) return frontier;

    int token = current_token++;

    for (const auto& c : areas[areaIndex].cells) {
        for (const auto& nb : neighbors(c)) {
            if (grid[nb.x][nb.y] == 0 && vis_token[nb.x][nb.y] != token) {
                vis_token[nb.x][nb.y] = token;
                frontier.push_back(nb);
            }
        }
    }

    return frontier;
}


/* ---------------------------------------------------------------------[<]-
 Function: merge_areas
 Synopsis: merges source area into target, marks source as dead.
 ---------------------------------------------------------------------[>]-*/
void Grid::merge_areas(int target, int source) {
    if (!areas[source].alive) return;

    for (const auto& c : areas[source].cells) {
        areas[target].cells.push_back(c);
        area_id[c.x][c.y] = target;
    }

    areas[source].cells.clear();
    areas[source].alive = false;
}


/* ---------------------------------------------------------------------[<]-
 Function: apply_fill
 Synopsis: fills a cell with the area value and merges adjacent areas.
 ---------------------------------------------------------------------[>]-*/
void Grid::apply_fill(int areaIndex, const Cell& c) {
    if (grid[c.x][c.y] != 0) return;

    int value = areas[areaIndex].value;

    grid[c.x][c.y] = value;

    auto adj = get_adjacent_same_value_areas(c, value);

    areas[areaIndex].cells.push_back(c);
    area_id[c.x][c.y] = areaIndex;

    for (int id : adj) {
        if (id != areaIndex) {
            merge_areas(areaIndex, id);
        }
    }
    filled_count++;
    if (on_progress) {
        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::milliseconds>(now - last_progress_time).count() >= 200) {
            last_progress_time = now;
            on_progress(filled_count, n * m);
        }
    }
}


/* ---------------------------------------------------------------------[<]-
 Function: count_reachable_cells
 Synopsis: counts empty cells reachable from given area using BFS.
 ---------------------------------------------------------------------[>]-*/
int Grid::count_reachable_cells(int areaIndex,
    const vector<vector<int>>& tmpGrid) const
{
    int value = areas[areaIndex].value;

    int token = current_token++;
    queue<Cell> q;

    for (const auto& c : areas[areaIndex].cells) {
        vis_token[c.x][c.y] = token;
        q.push(c);
    }

    int reachable = 0;

    while (!q.empty()) {
        Cell cur = q.front(); q.pop();

        for (auto& nb : neighbors(cur)) {
            if (vis_token[nb.x][nb.y] == token) continue;

            if (tmpGrid[nb.x][nb.y] == 0 ||
                tmpGrid[nb.x][nb.y] == value)
            {
                vis_token[nb.x][nb.y] = token;
                q.push(nb);

                if (tmpGrid[nb.x][nb.y] == 0)
                    reachable++;
            }
        }
    }

    return reachable;
}


/* ---------------------------------------------------------------------[<]-
 Function: get_reachable_cells
 Synopsis: returns list of empty cells reachable from given area using BFS.
 ---------------------------------------------------------------------[>]-*/
vector<Cell> Grid::get_reachable_cells(
    int areaIndex,
    const vector<vector<int>>& tmpGrid) const
{
    int value = areas[areaIndex].value;

    int token = current_token++;
    queue<Cell> q;

    for (const auto& c : areas[areaIndex].cells) {
        vis_token[c.x][c.y] = token;
        q.push(c);
    }

    vector<Cell> reachable;

    while (!q.empty()) {
        Cell cur = q.front(); q.pop();

        for (auto& nb : neighbors(cur)) {
            if (vis_token[nb.x][nb.y] == token) continue;

            if (tmpGrid[nb.x][nb.y] == 0 ||
                tmpGrid[nb.x][nb.y] == value)
            {
                vis_token[nb.x][nb.y] = token;
                q.push(nb);

                if (tmpGrid[nb.x][nb.y] == 0)
                    reachable.push_back(nb);
            }
        }
    }

    return reachable;
}






























