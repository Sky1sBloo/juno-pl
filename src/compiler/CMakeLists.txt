add_library(juno-pl-compiler)
target_sources(
  juno-pl-compiler
  PUBLIC
  FILE_SET CXX_MODULES FILES Compiler.cppm
  PRIVATE Compiler.cpp)

target_link_libraries(juno-pl-compiler PRIVATE juno-pl-parse-nodes juno-pl-stl pugixml)

target_compile_features(juno-pl-compiler PUBLIC cxx_std_23)
