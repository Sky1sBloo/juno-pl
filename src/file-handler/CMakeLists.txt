add_library(juno-pl-file-handler)
target_sources(juno-pl-file-handler 
    PUBLIC
        FILE_SET CXX_MODULES
        FILES
            FileHandler.cppm
            IFileHandler.cppm
    PRIVATE
        FileHandler-impl.cpp
        )
target_compile_features(juno-pl-file-handler PUBLIC cxx_std_23)
