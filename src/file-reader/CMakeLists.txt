add_library(juno-pl-file-reader)
target_sources(juno-pl-file-reader PUBLIC
    FILE_SET CXX_MODULES
    FILES
    FileReader.cppm
    IFileReader.cppm)
target_include_directories(juno-pl-file-reader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(juno-pl-file-reader PUBLIC cxx_std_23)