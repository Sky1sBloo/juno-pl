add_library(juno-pl-config)
target_sources(
  juno-pl-config
  PUBLIC
  FILE_SET CXX_MODULES FILES Config.cppm
  PRIVATE Config.cpp)

target_link_libraries(juno-pl-config PRIVATE nlohmann_json)

target_compile_features(juno-pl-config PUBLIC cxx_std_23)
