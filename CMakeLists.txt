cmake_minimum_required(VERSION 4.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(juno-pl
    VERSION 1.0
    DESCRIPTION "Juno Programming Language"
    LANGUAGES CXX)

enable_testing()

# MSVC compiler settings:
#  /utf-8           keep UTF-8 sources and string literals intact (otherwise
#                   MSVC uses the system code page and warns C4819).
#  /Zc:__cplusplus  make __cplusplus report the selected standard; vendored
#                   headers such as pugixml test it directly.
# A static runtime lets the released juno-pl.exe run without the Visual C++
# redistributable installed.
if(MSVC)
    add_compile_options(/utf-8 /Zc:__cplusplus)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

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