/* ----------------------------------------------------------------<Header>-
 Name: task5.cpp
 Title: Filomino puzzle solver.
 Group: TV-51
 Student: Andriievskyi D.Y.
 Written: 2025-05-03
 Revised: 2025-05-03
 Description: Main file for the Filomino puzzle solver.
              Runs three test puzzles and prints the solutions
              with a progress bar and time taken.
 ------------------------------------------------------------------</Header>-*/

#include "grid.h"
#include "interface.h"
#include <iostream>
#include <unordered_set>
#include <chrono>
using namespace std;

//counts non-zero cells in the grid
int count_filled(const vector<vector<int>>& grid) {
    int count = 0;
    for (auto& row : grid)
        for (int v : row)
            if (v != 0) count++;
    return count;
}

/* ---------------------------------------------------------------------[<]-
 Function: run_test
 Synopsis: runs solver on a given puzzle, prints initial state,
           progress bar, solution and time taken.
 ---------------------------------------------------------------------[>]-*/
void run_test(const string& name, const vector<vector<int>>& puzzle) {
    int total = puzzle.size() * puzzle[0].size();

    print_header(name);
    cout << "  Initial state:\n\n";
    print_grid(puzzle);

    cout << "  Solving...\n\n";

    int initial_filled = 0;
    for (auto& row : puzzle)
        for (int v : row)
            if (v != 0) initial_filled++;

    print_progress(initial_filled, total);

    auto t1 = chrono::high_resolution_clock::now();

    Grid g(puzzle);
    g.on_progress = [&](int filled, int tot) {
        print_progress(filled, tot);
    };

    unordered_set<string> seen;
    bool solved = g.solve(seen);

    auto t2 = chrono::high_resolution_clock::now();
    double elapsed = chrono::duration<double>(t2 - t1).count();

    print_progress(total, total);
    cout << "\n\n";

    if (solved) {
        cout << "\n\n  Solved!\n\n";
        print_grid(g.data());
    } else {
        cout << "  No solution found.\n\n";
    }

    cout << "  Time: " << fixed << setprecision(4) << elapsed << " sec\n\n";
}

int main() {
    print_title_screen();

    vector<vector<int>> test1 = {
        {0,3,5,0,5,4,0,4,4,0},
        {1,0,0,0,0,0,0,0,0,6},
        {3,0,0,0,0,0,0,0,0,3},
        {0,0,0,2,6,2,6,0,0,0},
        {6,0,0,6,0,0,2,0,0,1},
        {1,0,0,2,0,0,6,0,0,2},
        {0,0,0,6,2,6,2,0,0,0},
        {3,0,0,0,0,0,0,0,0,5},
        {2,0,0,0,0,0,0,0,0,6},
        {0,3,4,0,4,6,0,2,1,0}
    };

    vector<vector<int>> test2 = {
        {1,2,3,4,5,6,7,8,0,8},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,8},
        {2,0,0,0,0,0,0,0,8,0},
        {0,8,0,8,0,0,0,8,0,8},
        {1,0,7,0,0,0,7,0,2,0},
        {0,8,0,0,0,0,0,0,0,3},
        {8,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {1,0,8,7,6,5,4,3,2,1}
    };

    vector<vector<int>> test3 = {
        {0,0,0,0,0,0,0,0,0,0},
        {0,1,5,6,2,2,6,3,1,0},
        {0,9,0,0,0,0,0,0,4,0},
        {0,5,5,3,4,4,2,2,7,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0},
        {0,7,2,7,5,4,3,6,3,0},
        {0,9,0,0,0,0,0,0,7,0},
        {0,9,5,4,8,6,6,2,2,0},
        {0,0,0,0,0,0,0,0,0,0}
    };

    run_test("Test 1", test1);
    run_test("Test 2", test2);
    run_test("Test 3", test3);

    return 0;
}






