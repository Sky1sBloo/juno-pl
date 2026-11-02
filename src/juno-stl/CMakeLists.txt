add_library(juno-pl-stl)
target_sources(
  juno-pl-stl
  PUBLIC
  FILE_SET CXX_MODULES FILES StandardLibrary.cppm)
target_compile_features(juno-pl-stl PUBLIC cxx_std_23)

