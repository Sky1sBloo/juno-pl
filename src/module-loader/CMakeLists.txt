add_library(juno-pl-module-loader)
target_sources(juno-pl-module-loader
    PUBLIC
        FILE_SET CXX_MODULES
        FILES
            ModuleLoader.cppm
    PRIVATE
        ModuleLoader-impl.cpp
        )
target_link_libraries(juno-pl-module-loader PRIVATE juno-pl-common juno-pl-diagnostics juno-pl-file-handler juno-pl-lexer juno-pl-parser juno-pl-parse-nodes juno-pl-semantic-analyzer)
target_compile_features(juno-pl-module-loader PUBLIC cxx_std_23)
