add_library(juno-pl-parser)
target_sources(
  juno-pl-parser
  PUBLIC
  FILE_SET CXX_MODULES FILES Parser.cppm)

add_subdirectory(nodes)
target_link_libraries(juno-pl-parser PRIVATE juno-pl-lexer juno-pl-parse-nodes)
target_compile_features(juno-pl-parser PUBLIC cxx_std_23)
