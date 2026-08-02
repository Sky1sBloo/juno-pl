add_subdirectory(doctest)

add_executable(juno-pl-tests)
target_sources(juno-pl-tests PRIVATE doctest_main.cpp)
target_compile_features(juno-pl-tests PRIVATE cxx_std_23)
target_link_libraries(juno-pl-tests doctest juno-pl-file-reader juno-pl-lexer)

enable_testing()
add_test(NAME lexer_test COMMAND lexer_test)