add_library(juno-pl-parse-nodes)
target_sources(
  juno-pl-parse-nodes
  PUBLIC
  FILE_SET CXX_MODULES FILES RootNodes.cppm Statements.cppm Expressions.cppm)
target_compile_features(juno-pl-parse-nodes PUBLIC cxx_std_23)
