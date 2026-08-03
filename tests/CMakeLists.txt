add_subdirectory(doctest)
add_subdirectory(tests-impl)

add_executable(juno-pl-tests)
target_sources(juno-pl-tests PRIVATE 
doctest_main.cpp
FileReader-tests.cpp
Lexer-tests.cpp)
target_compile_features(juno-pl-tests PRIVATE cxx_std_23)
target_link_libraries(juno-pl-tests doctest juno-pl-lexer juno-pl-tests-impl)

enable_testing()
add_test(NAME lexer_test COMMAND lexer_test)