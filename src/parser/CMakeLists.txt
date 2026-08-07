add_library(juno-pl-parser)
target_sources(
  juno-pl-parser
  PUBLIC
  FILE_SET
    CXX_MODULES FILES Parser.cppm ParserError.cppm handlers/Handler.cppm
    handlers/FunctionHandler.cppm
  PRIVATE Parser-impl.cpp handlers/FunctionHandler-impl.cpp)

add_subdirectory(nodes)
target_link_libraries(juno-pl-parser PRIVATE juno-pl-lexer juno-pl-parse-nodes)
target_compile_features(juno-pl-parser PUBLIC cxx_std_23)
