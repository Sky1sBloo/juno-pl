add_library(juno-pl-file-reader STATIC FileReader.cpp)
target_include_directories(juno-pl-file-reader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(juno-pl-file-reader PUBLIC cxx_std_23)