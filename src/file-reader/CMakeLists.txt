add_library(juno-pl-file-reader)
target_sources(juno-pl-file-reader 
    PUBLIC
        FILE_SET CXX_MODULES
        FILES
            FileReader.cppm
            IFileReader.cppm
    PRIVATE
        FileReader-impl.cpp
        )
target_compile_features(juno-pl-file-reader PUBLIC cxx_std_23)