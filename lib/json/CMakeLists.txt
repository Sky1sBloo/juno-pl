# nlohmann/json 3.12 ships as a single header, so this is an interface
# target that only publishes the include directory.
add_library(nlohmann_json INTERFACE)

target_sources(nlohmann_json INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/json.hpp
)

target_include_directories(nlohmann_json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(nlohmann_json INTERFACE cxx_std_23)
