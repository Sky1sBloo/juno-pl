add_library(juno-pl-lexer)
target_sources(juno-pl-lexer PUBLIC 
FILE_SET CXX_MODULES
FILES
Lexer.cppm
)

target_include_directories(juno-pl-lexer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(juno-pl-lexer PUBLIC juno-pl-file-reader)
target_compile_features(juno-pl-lexer PUBLIC cxx_std_23)