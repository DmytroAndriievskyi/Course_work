#pragma once
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
using namespace std;

#define RESET  "\033[0m"
#define BOLD   "\033[1m"
#define RED    "\033[31m"
#define YELLOW "\033[33m"
#define GREEN  "\033[32m"
#define CYAN   "\033[36m"

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

