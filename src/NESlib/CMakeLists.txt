add_library(NESlib STATIC CPU.cpp Bus.cpp Cartridge.cpp mapper/MapperNROM.cpp)
target_include_directories(NESlib PUBLIC "${CURRENT_SOURCE_DIR}")

add_executable(testCPU tests/test_CPU2.cpp)
target_link_libraries(testCPU PRIVATE NESlib)