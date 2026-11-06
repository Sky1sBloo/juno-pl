add_library(juno-pl-diagnostics)
target_sources(juno-pl-diagnostics
    PUBLIC
        FILE_SET CXX_MODULES
        FILES
            Diagnostics.cppm
    PRIVATE
        Diagnostics-impl.cpp
        )
target_link_libraries(juno-pl-diagnostics PRIVATE juno-pl-common juno-pl-file-handler juno-pl-semantic-analyzer)
target_compile_features(juno-pl-diagnostics PUBLIC cxx_std_23)
