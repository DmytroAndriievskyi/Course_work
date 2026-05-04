#include "grid.h"
#include <queue>
#include <set>
using namespace std;


/* ---------------------------------------------------------------------[<]-
 Function: move_blocks_other_area
 Synopsis: checks if placing a cell would block another area
           from reaching enough empty cells to complete.
 ---------------------------------------------------------------------[>]-*/
bool Grid::move_blocks_other_area(int areaIndex, const Cell& c) const {
    if (grid[c.x][c.y] != 0) return true;

    int value = areas[areaIndex].value;

    const_cast<vector<vector<int>>&>(grid)[c.x][c.y] = value;

    set<int> toCheck;
    for (auto& nb : neighbors(c)) {
        int aid = area_id[nb.x][nb.y];
        if (aid != -1 && aid != areaIndex && areas[aid].alive)
            toCheck.insert(aid);
    }
    for (auto& nb : neighbors(c)) {
        if (grid[nb.x][nb.y] != 0) continue;
        for (auto& nb2 : neighbors(nb)) {
            int aid = area_id[nb2.x][nb2.y];
            if (aid != -1 && aid != areaIndex && areas[aid].alive)
                toCheck.insert(aid);
        }
    }

    bool blocked = false;
    for (int id : toCheck) {
        int need = areas[id].value - (int)areas[id].cells.size();
        if (need <= 0) continue;
        int reachable = count_reachable_cells(id, grid);
        if (reachable < need) {
            blocked = true;
            break;
        }
    }

    const_cast<vector<vector<int>>&>(grid)[c.x][c.y] = 0;

    return blocked;
}


/* ---------------------------------------------------------------------[<]-
 Function: move_causes_invalid_merge
 Synopsis: checks if placing a cell would cause a merge that
           exceeds the area's value limit.
 ---------------------------------------------------------------------[>]-*/
bool Grid::move_causes_invalid_merge(int areaIndex, const Cell& c) const {
    int value = areas[areaIndex].value;

    auto adj = get_adjacent_same_value_areas(c, value);

    int total = areas[areaIndex].cells.size() + 1;

    for (int id : adj) {
        if (id != areaIndex && areas[id].alive) {
            total += (int)areas[id].cells.size();
        }
    }

    return total > value;
}


/* ---------------------------------------------------------------------[<]-
 Function: get_adjacent_same_value_areas
 Synopsis: returns list of area ids adjacent to cell c with given value.
 ---------------------------------------------------------------------[>]-*/
vector<int> Grid::get_adjacent_same_value_areas(const Cell& c, int value) const {
    set<int> ids;

    for (auto& nb : neighbors(c)) {
        if (grid[nb.x][nb.y] == value) {
            ids.insert(area_id[nb.x][nb.y]);
        }
    }

    return vector<int>(ids.begin(), ids.end());
}


// checks both merge and blocking conditions
bool Grid::move_is_allowed(int areaIndex, const Cell& c) const { 
    if (move_causes_invalid_merge(areaIndex, c)) return false;
    if (move_blocks_other_area(areaIndex, c)) return false;
    return true;
}


/* ---------------------------------------------------------------------[<]-
 Function: try_forced_shape
 Synopsis: if an area has exactly as many reachable cells as it needs,
           fills them all immediately.
 ---------------------------------------------------------------------[>]-*/
bool Grid::try_forced_shape() {
    for (int i = 0; i < (int)areas.size(); i++) {
        if (!areas[i].alive) continue;
        if (is_area_finished(i)) continue;

        int need = areas[i].value - (int)areas[i].cells.size();
        if (need <= 0) continue;

        auto reachable = get_reachable_cells(i, grid);

        if ((int)reachable.size() == need) {
            for (auto& c : reachable) {
                apply_fill(i, c);
            }
            return true;
        }
    }

    return false;
}


/* ---------------------------------------------------------------------[<]-
 Function: try_no_new_ones
 Synopsis: if an empty cell is near a completed area of value 1,
           and only one other area can claim it, force that claim.
 ---------------------------------------------------------------------[>]-*/
bool Grid::try_no_new_ones() {
    for (int x = 0; x < n; x++) {
        for (int y = 0; y < m; y++) {

            if (grid[x][y] != 0) continue;

            bool nearOne = false;
            for (auto& nb : neighbors({x, y})) {
                if (grid[nb.x][nb.y] == 1) {
                    nearOne = true;
                    break;
                }
            }

            if (!nearOne) continue;

            set<int> possibleAreas;  

            for (int i = 0; i < (int)areas.size(); i++) {
                if (!areas[i].alive) continue;
                if (is_area_finished(i)) continue;  
                if (areas[i].value == 1) continue;

                for (auto& nb : neighbors({x, y})) {
                    if (area_id[nb.x][nb.y] == i) {
                        if (move_is_allowed(i, {x, y})) {
                            possibleAreas.insert(i);
                        }
                        break;
                    }
                }
            }

            if (possibleAreas.size() == 1) {
                apply_fill(*possibleAreas.begin(), {x, y});
                return true;
            }
        }
    }

    return false;
}


/* ---------------------------------------------------------------------[<]-
 Function: try_forced_merge
 Synopsis: checks if two large areas must merge through a unique path
           and forces that merge if confirmed safe.
 ---------------------------------------------------------------------[>]-*/
bool Grid::try_forced_merge() {
    for (int minVal = 9; minVal >= 7; minVal--) {

        vector<int> candidates;
        for (int i = 0; i < (int)areas.size(); i++) {
            if (!areas[i].alive) continue;
            if (is_area_finished(i)) continue;
            if (areas[i].value >= minVal)
                candidates.push_back(i);
        }

        for (int ai = 0; ai < (int)candidates.size(); ai++) {
            for (int bi = ai + 1; bi < (int)candidates.size(); bi++) {
                int idA = candidates[ai];
                int idB = candidates[bi];

                if (areas[idA].value != areas[idB].value) continue;

                int value = areas[idA].value;
                int sizeA = (int)areas[idA].cells.size();
                int sizeB = (int)areas[idB].cells.size();

                if (sizeA + sizeB > value) continue;

                vector<vector<bool>> inA(n, vector<bool>(m, false));
                vector<vector<bool>> inB(n, vector<bool>(m, false));
                for (const auto& c : areas[idA].cells) inA[c.x][c.y] = true;
                for (const auto& c : areas[idB].cells) inB[c.x][c.y] = true;

                vector<vector<int>> dist(n, vector<int>(m, -1));
                vector<vector<long long>> pathCount(n, vector<long long>(m, 0));
                queue<Cell> q;

                for (const auto& c : areas[idA].cells) {
                    dist[c.x][c.y] = 0;
                    pathCount[c.x][c.y] = 1;
                    q.push(c);
                }

                while (!q.empty()) {
                    Cell cur = q.front(); q.pop();

                    for (const auto& nb : neighbors(cur)) {
                        bool isZero = (grid[nb.x][nb.y] == 0);
                        bool isB    = inB[nb.x][nb.y];
                        bool isA    = inA[nb.x][nb.y];

                        if (!isZero && !isB && !isA) continue;
                        if (isA && dist[nb.x][nb.y] != -1) continue;

                        if (dist[nb.x][nb.y] == -1) {
                            dist[nb.x][nb.y] = dist[cur.x][cur.y] + 1;
                            pathCount[nb.x][nb.y] = pathCount[cur.x][cur.y];
                            if (!isB)
                                q.push(nb);
                        } else if (dist[nb.x][nb.y] == dist[cur.x][cur.y] + 1) {
                            pathCount[nb.x][nb.y] += pathCount[cur.x][cur.y];
                        }
                    }
                }

                int minDistToB = INT_MAX;
                for (const auto& c : areas[idB].cells) {
                    if (dist[c.x][c.y] != -1)
                        minDistToB = min(minDistToB, dist[c.x][c.y]);
                }

                if (minDistToB == INT_MAX) continue;

                long long totalPaths = 0;
                for (const auto& c : areas[idB].cells) {
                    if (dist[c.x][c.y] == minDistToB)
                        totalPaths += pathCount[c.x][c.y];
                }

                if (totalPaths != 1) continue;

                int pathLen = minDistToB - 1;

                if (sizeA + pathLen + sizeB != value) continue;

                // reconstruct the unique path
                vector<vector<Cell>> prev(n, vector<Cell>(m, {-1, -1}));
                vector<vector<int>> dist2(n, vector<int>(m, -1));
                queue<Cell> q2;

                for (const auto& c : areas[idA].cells) {
                    dist2[c.x][c.y] = 0;
                    q2.push(c);
                }

                Cell endCell = {-1, -1};
                bool found = false;

                while (!q2.empty() && !found) {
                    Cell cur = q2.front(); q2.pop();

                    for (const auto& nb : neighbors(cur)) {
                        if (dist2[nb.x][nb.y] != -1) continue;

                        bool isZero = (grid[nb.x][nb.y] == 0);
                        bool isB2   = inB[nb.x][nb.y];
                        if (!isZero && !isB2) continue;

                        dist2[nb.x][nb.y] = dist2[cur.x][cur.y] + 1;
                        prev[nb.x][nb.y] = cur;
                        q2.push(nb);

                        if (isB2) {
                            found = true;
                            endCell = nb;
                            break;
                        }
                    }
                }

                if (!found) continue;

                vector<Cell> pathCells;
                Cell cur = endCell;
                while (true) {
                    Cell p = prev[cur.x][cur.y];
                    if (p.x == -1) break;
                    if (grid[cur.x][cur.y] == 0)
                        pathCells.push_back(cur);
                    if (inA[p.x][p.y]) break;
                    cur = p;
                }

                // check if merge is truly forced
                vector<vector<bool>> onPath(n, vector<bool>(m, false));
                for (const auto& pc : pathCells)
                    onPath[pc.x][pc.y] = true;
                for (const auto& c : areas[idB].cells)
                    onPath[c.x][c.y] = true;

                bool aHasAlternative = false;
                for (const auto& c : areas[idA].cells) {
                    for (const auto& nb : neighbors(c)) {
                        if (grid[nb.x][nb.y] == 0 && !onPath[nb.x][nb.y]) {
                            aHasAlternative = true;
                            break;
                        }
                    }
                    if (aHasAlternative) break;
                }

                vector<vector<bool>> onPathFromB(n, vector<bool>(m, false));
                for (const auto& pc : pathCells)
                    onPathFromB[pc.x][pc.y] = true;
                for (const auto& c : areas[idA].cells)
                    onPathFromB[c.x][c.y] = true;

                bool bHasAlternative = false;
                for (const auto& c : areas[idB].cells) {
                    for (const auto& nb : neighbors(c)) {
                        if (grid[nb.x][nb.y] == 0 && !onPathFromB[nb.x][nb.y]) {
                            bHasAlternative = true;
                            break;
                        }
                    }
                    if (bHasAlternative) break;
                }

                if (aHasAlternative && bHasAlternative) continue;

                // check that merge does not block other areas
                vector<vector<int>> tmpGrid = grid;
                for (const auto& pc : pathCells)
                    tmpGrid[pc.x][pc.y] = value;
                for (const auto& c : areas[idB].cells)
                    tmpGrid[c.x][c.y] = value;

                bool anyBlocked = false;
                for (int i = 0; i < (int)areas.size(); i++) {
                    if (i == idA || i == idB) continue;
                    if (!areas[i].alive) continue;
                    if (is_area_finished(i)) continue;

                    int need = areas[i].value - (int)areas[i].cells.size();
                    if (need <= 0) continue;

                    int reachable = count_reachable_cells(i, tmpGrid);
                    if (reachable < need) {
                        anyBlocked = true;
                        break;
                    }
                }

                if (anyBlocked) continue;

                for (const auto& pc : pathCells)
                    apply_fill(idA, pc);
                merge_areas(idA, idB);

                return true;
            }
        }
    }

    return false;
}


/* ---------------------------------------------------------------------[<]-
 Function: deduction_pass
 Synopsis: runs one full pass of all deduction strategies.
           returns true if any change was made.
 ---------------------------------------------------------------------[>]-*/
bool Grid::deduction_pass() {
    bool changedAny = false;

    while (true) {
        bool changed = false;

        if (try_no_new_ones()) {
            changedAny = true;
            continue;
        }

        if (try_forced_merge()) {
            changedAny = true;
            continue;
        }


        if (try_forced_shape()) {
            changedAny = true;
            continue;
        }

        for (int i = 0; i < (int)areas.size(); i++) {
            if (!areas[i].alive) continue;
            if (is_area_finished(i)) continue;

            auto frontier = get_area_frontier(i);

            vector<Cell> allowed;

            for (auto& c : frontier) {
                if (move_is_allowed(i, c))
                    allowed.push_back(c);
            }

            if (allowed.size() == 1) {
                Grid backup = *this;

                apply_fill(i, allowed[0]);

                if (is_invalid_state()) {
                    *this = backup;
                    continue;
                }

                changed = true;
                changedAny = true;
                break;
            }
        }

        if (!changed) break;
    }

    return changedAny;
}


// runs deduction until no more changes can be made
void Grid::deduction_until_stable() {
    while (deduction_pass()) {}
}