cmake_minimum_required(VERSION 4.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(juno-pl
    VERSION 1.0
    DESCRIPTION "Juno Programming Language"
    LANGUAGES CXX)

add_executable(juno-pl src/main.cpp)
add_subdirectory(src/file-reader)
add_subdirectory(src/lexer)
add_subdirectory(src/parser)
add_subdirectory(tests)
target_link_libraries(juno-pl PRIVATE juno-pl-lexer juno-pl-file-reader)