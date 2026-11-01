add_library(juno-pl-semantic-analyzer)
target_sources(
  juno-pl-semantic-analyzer
  PUBLIC
  FILE_SET CXX_MODULES FILES SemanticAnalyzer.cppm Symbols.cppm)

target_link_libraries(
  juno-pl-semantic-analyzer
  PRIVATE juno-pl-parser juno-pl-parse-nodes)
target_compile_features(juno-pl-semantic-analyzer PUBLIC cxx_std_23)
