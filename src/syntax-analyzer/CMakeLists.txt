add_library(juno-pl-syntax-analyzer)
target_sources(
  juno-pl-syntax-analyzer
  PUBLIC
  FILE_SET CXX_MODULES FILES SyntaxAnalyzer.cppm)

target_link_libraries(juno-pl-syntax-analyzer PRIVATE juno-pl-lexer)
target_compile_features(juno-pl-syntax-analyzer PUBLIC cxx_std_23)
