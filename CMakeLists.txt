cmake_minimum_required(VERSION 3.22)
project(NES)

set(CMAKE_CXX_STANDARD 14)

add_executable(NES main.cpp CPU.cpp CPU.h Parser.cpp Parser.h RAM.cpp RAM.h)
