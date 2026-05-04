#pragma once
#include <vector>
#include <string>
#include <unordered_set>
#include <functional>
#include <chrono>
using namespace std;

struct Cell {
    int x, y;
};

struct Area {
    int id;
    int value;
    vector<Cell> cells;
    bool alive = true;
};

/* ----------------------------------------------------------------------[<]-
    Class: Grid
    Synopsis: main class for the Filomino puzzle solver.
              Contains grid data, area tracking, deduction strategies
              and backtracking solver.
    ---------------------------------------------------------------------[>]-*/
class Grid {
public:
    Grid(const vector<vector<int>>& start);
    bool solve(unordered_set<string>& seen);

    const vector<vector<int>>& data() const { return grid; }

    function<void(int, int)> on_progress;
    int filled_count = 0;

private:
    int n, m;
    vector<vector<int>> grid;
    vector<vector<int>> area_id;
    vector<Area> areas;
    mutable vector<vector<int>> vis_token;
    mutable int current_token;
    mutable chrono::steady_clock::time_point last_progress_time;

    
    void init_areas();
    bool in_bounds(int x, int y) const;
    vector<Cell> neighbors(const Cell& c) const;
    bool is_area_finished(int areaIndex) const;
    vector<Cell> get_area_frontier(int areaIndex) const;
    void merge_areas(int target, int source);
    void apply_fill(int areaIndex, const Cell& c);
    int count_reachable_cells(int areaIndex,
        const vector<vector<int>>& tmpGrid) const;
    vector<Cell> get_reachable_cells(
        int areaIndex,
        const vector<vector<int>>& tmpGrid) const;


    bool move_blocks_other_area(int areaIndex, const Cell& c) const;
    bool move_causes_invalid_merge(int areaIndex, const Cell& c) const;    
    vector<int> get_adjacent_same_value_areas(const Cell& c, int value) const;
    bool move_is_allowed(int areaIndex, const Cell& c) const;
    bool try_forced_shape();
    bool try_no_new_ones();
    bool try_forced_merge();
    bool deduction_pass();
    void deduction_until_stable();


    int count_shape_completions_dfs(int value, vector<Cell>& shape, 
        vector<vector<bool>>& used, int targetSize, int limit) const;
    int count_shape_completions_after_move(int areaIndex, const Cell& move, int limit) const;
    int move_completion_score(int areaIndex, const Cell& move) const;    
    bool is_invalid_state() const;
    bool is_solved() const;
    int choose_branch_area() const;
    vector<Cell> get_allowed_moves_for_area(int areaIndex) const;
    void create_new_area_from_cells(const vector<Cell>& cells);    
    bool has_zeros() const;
    bool fill_holes_by_size();
    string serialize() const;
};