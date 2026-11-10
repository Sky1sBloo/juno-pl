add_subdirectory(doctest)
add_subdirectory(tests-impl)

add_executable(juno-pl-tests)
target_sources(juno-pl-tests PRIVATE 
doctest_main.cpp
FileHandler-tests.cpp
Lexer-tests.cpp
StandardLibrary-tests.cpp
SemanticAnalyzer-tests.cpp
Config-tests.cpp
Compiler-tests.cpp
CompilerXML-tests.cpp)
target_compile_features(juno-pl-tests PRIVATE cxx_std_23)
target_link_libraries(juno-pl-tests doctest juno-pl-lexer juno-pl-parser juno-pl-tests-impl juno-pl-stl juno-pl-semantic-analyzer juno-pl-config juno-pl-compiler)
add_subdirectory(parser)

add_test(NAME juno-pl-tests COMMAND juno-pl-tests)