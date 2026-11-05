add_library(pugixml STATIC
    pugixml.cpp
)

target_sources(pugixml PUBLIC
    pugixml.hpp
    pugiconfig.hpp
)

target_include_directories(pugixml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(pugixml PUBLIC cxx_std_23)
