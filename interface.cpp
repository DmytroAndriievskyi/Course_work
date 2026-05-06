#include "interface.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
using namespace std;

/* ---------------------------------------------------------------------[<]-
Function: print_title_screen
Synopsis: prints a welcome screen with Fillomino game instructions
          formatted inside a bordered box.
---------------------------------------------------------------------[>]-*/
void print_title_screen() {
    auto line = [](const string& text, int width) {
        int pad = width - 2 - (int)text.size();
        int lpad = pad / 2;
        int rpad = pad - lpad;
        cout << "|" << string(lpad, ' ') << text << string(rpad, ' ') << "|\n";
    };

    auto divider = [](int width) {
        cout << "+" << string(width - 2, '-') << "+\n";
    };

    auto rule = [&](const string& text, int width) {
        int pad = width - 3 - (int)text.size();
        cout << "| " << text << string(max(0, pad), ' ') << "|\n";
    };

    const int W = 50;

    cout << BOLD << CYAN;
    divider(W);
    line("", W);
    line("FILLOMINO PUZZLE SOLVER", W);
    line("", W);
    divider(W);
    line("", W);
    line("GAME RULES", W);
    line("", W);
    cout << RESET;

    divider(W);
    rule("  1. Fill every empty cell with a number.", W);
    rule("  2. Each number must belong to a connected", W);
    rule("     region whose size matches that number.", W);
    rule("  3. Two cells with the same number in one", W);
    rule("     region are not allowed.", W);
    rule("  4. Two regions sharing the same number", W);
    rule("     cannot touch each other side by side.", W);
    rule("  5. Regions connect by sides, not corners.", W);
    divider(W);
    rule("", W);
    rule("  GRID LEGEND:", W);
    rule("   '.'  - empty cell (will be filled)", W);
    rule("   'N'  - given clue (starting condition)", W);
    rule("", W);
    divider(W);

    cout << BOLD << CYAN;
    line("", W);
    line("Press Enter to start...", W);
    line("", W);
    divider(W);
    cout << RESET;

    cin.get();
}

/* ---------------------------------------------------------------------[<]-
 Function: print_header
 Synopsis: prints a centered title with a border.
 ---------------------------------------------------------------------[>]-*/
void print_header(const string& title) {
    cout << BOLD << CYAN;
    cout << "========================================\n";
    int pad = (40 - (int)title.size()) / 2;
    for (int i = 0; i < pad; i++) cout << " ";
    cout << title << "\n";
    cout << "========================================\n";
    cout << RESET << "\n";
}

/* ---------------------------------------------------------------------[<]-
 Function: print_grid
 Synopsis: prints the grid with borders and dots for empty cells.
 ---------------------------------------------------------------------[>]-*/
void print_grid(const vector<vector<int>>& grid) {
    int n = grid.size();
    int m = grid[0].size();

    cout << "  +";
    for (int j = 0; j < m; j++) cout << "---+";
    cout << "\n";

    for (int i = 0; i < n; i++) {
        cout << "  |";
        for (int j = 0; j < m; j++) {
            int v = grid[i][j];
            if (v == 0) cout << " . |";
            else        cout << " " << v << " |";
        }
        cout << "\n";

        cout << "  +";
        for (int j = 0; j < m; j++) cout << "---+";
        cout << "\n";
    }
    cout << "\n";
}

/* ---------------------------------------------------------------------[<]-
 Function: print_progress
 Synopsis: prints a progress bar with percentage.
           red < 33.4%, yellow < 66.7%, green otherwise.
 ---------------------------------------------------------------------[>]-*/
void print_progress(int filled, int total) {
    double pct = (total > 0) ? (100.0 * filled / total) : 0.0;

    int barWidth = 30;
    int filledBars = (int)(barWidth * pct / 100.0);

    const char* color;
    if (pct < 33.4)      color = RED;
    else if (pct < 66.7) color = YELLOW;
    else                  color = GREEN;

    cout << "\r\033[2K  Progress: [" << BOLD << color;
    for (int i = 0; i < barWidth; i++) {
        if (i < filledBars) cout << "|";
        else                cout << " ";
    }
    cout << RESET << "] ";
    cout << BOLD << color << fixed << setprecision(1) << pct << "%" << RESET;
    cout.flush();
} 