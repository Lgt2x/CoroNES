cmake_minimum_required(VERSION 3.26)
project("NES emulator" LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 90)

add_subdirectory(src/NESlib)

add_executable(emu src/main.cpp)
target_link_libraries(emu PRIVATE NESlib)
