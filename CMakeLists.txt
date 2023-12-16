cmake_minimum_required(VERSION 3.23.1)
project(PTG)
add_executable(${PROJECT_NAME} src/main.cpp src/asthandles.cpp src/astprocessing.cpp src/codegen.cpp src/parser.cpp src/scanner.cpp)