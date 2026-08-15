add_library(juno-pl-lexer)
target_sources(juno-pl-lexer 
PUBLIC 
FILE_SET CXX_MODULES
FILES
    Lexer.cppm
    Tokens.cppm
    LexerError.cppm
    LexerStateHandler.cppm
    TokenIdentifier.cppm
PRIVATE
    Lexer-impl.cpp
    LexerStateHandler-impl.cpp
    LexerStateHandlerResult-impl.cpp
    TokenIdentifier-impl.cpp
)

target_link_libraries(juno-pl-lexer PRIVATE juno-pl-file-reader)
target_compile_features(juno-pl-lexer PUBLIC cxx_std_23)