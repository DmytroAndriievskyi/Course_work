CXX = g++
CXXFLAGS =  -Wall -Wextra -Wpedantic -std=c++17

course_work: task5.cpp grid_core.cpp grid_deduction.cpp grid_solver.cpp interface.cpp

	$(CXX) $(CXXFLAGS) -o course_work task5.cpp grid_core.cpp grid_deduction.cpp grid_solver.cpp interface.cpp