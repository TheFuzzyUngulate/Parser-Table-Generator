cmake_minimum_required(3.23.1)
project(ParserTableGenerator)
add_executable(ptg src/main.cpp src/asthandles.cpp src/astprocessing.cpp src/codegen.cpp src/parser.cpp src/scanner.cpp)