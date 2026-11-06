cmake_minimum_required(VERSION 4.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(juno-pl
    VERSION 1.0
    DESCRIPTION "Juno Programming Language"
    LANGUAGES CXX)

add_executable(juno-pl src/main.cpp)
add_subdirectory(src/common)
add_subdirectory(src/juno-stl)
add_subdirectory(src/file-handler)
add_subdirectory(src/lexer)
add_subdirectory(src/parser)
add_subdirectory(src/semantic-analyzer)
add_subdirectory(src/config)
add_subdirectory(src/diagnostics)
add_subdirectory(src/module-loader)
add_subdirectory(lib)
add_subdirectory(src/compiler)
add_subdirectory(tests)
target_link_libraries(juno-pl PRIVATE juno-pl-lexer juno-pl-file-handler juno-pl-parser juno-pl-parse-nodes juno-pl-stl juno-pl-semantic-analyzer juno-pl-config juno-pl-compiler juno-pl-diagnostics juno-pl-module-loader pugixml)