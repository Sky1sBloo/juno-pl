add_library(juno-pl-common)
target_sources(
  juno-pl-common
  PUBLIC
  FILE_SET CXX_MODULES FILES Location.cppm)
target_compile_features(juno-pl-common PUBLIC cxx_std_23)
