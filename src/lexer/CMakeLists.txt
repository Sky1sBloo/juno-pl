add_library(juno-pl-lexer STATIC Lexer.cpp)
target_include_directories(juno-pl-lexer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(juno-pl-lexer PRIVATE juno-pl-file-reader)
target_compile_features(juno-pl-lexer PUBLIC cxx_std_23)