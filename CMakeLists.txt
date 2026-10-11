cmake_minimum_required(VERSION 3.28)
project(CHIP_8_EMU)

set(CMAKE_CXX_STANDARD 17)

add_executable(CHIP_8_EMU main.cpp)
target_link_libraries(CHIP_8_EMU PRIVATE sfml-graphics sfml-window sfml-system)
