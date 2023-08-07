cmake_minimum_required(VERSION 3.26)
project(NES LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(src/NESlib)

add_executable(emu src/main.cpp)

target_link_libraries(emu PRIVATE NESlib)
