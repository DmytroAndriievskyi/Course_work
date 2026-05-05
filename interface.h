#pragma once
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
using namespace std;

inline constexpr const char* RESET  = "\033[0m";
inline constexpr const char* BOLD   = "\033[1m";
inline constexpr const char* RED    = "\033[31m";
inline constexpr const char* YELLOW = "\033[33m";
inline constexpr const char* GREEN  = "\033[32m";
inline constexpr const char* CYAN   = "\033[36m";

void print_title_screen();
void print_header(const string& title);
void print_grid(const vector<vector<int>>& grid);
void print_progress(int filled, int total);
 

