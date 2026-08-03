add_library(juno-pl-tests-impl)
target_sources(juno-pl-tests-impl
PUBLIC
    FILE_SET CXX_MODULES
    FILES
        FileReaderTest.cppm
PRIVATE
    FileReaderTest-impl.cpp)
target_link_libraries(juno-pl-tests-impl PUBLIC juno-pl-file-reader)
target_compile_features(juno-pl-tests-impl PUBLIC cxx_std_23)