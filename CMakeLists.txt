CMAKE_MINIMUM_REQUIRED(VERSION 3.22)
project(4lCSVParserab)

set(CMAKE_CXX_STANDARD 17)
add_executable(CSVParser main.cpp print_tuple.hpp CSVParser.hpp)