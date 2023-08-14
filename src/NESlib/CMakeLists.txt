add_library(NESlib STATIC CPU.cpp Bus.cpp Cartridge.cpp mappers/MapperNROM.cpp)
target_include_directories(NESlib PUBLIC "${CURRENT_SOURCE_DIR}")
target_include_directories(NESlib PUBLIC "${CMAKE_SOURCE_DIR}/src/ThirdParty/doctest")

add_executable(testCPU tests/TestCPU.cpp)
target_link_libraries(testCPU PRIVATE NESlib)

# ASM compiler
add_executable(asm6502 "${CMAKE_SOURCE_DIR}/src/ThirdParty/asm/asm6502.c")
