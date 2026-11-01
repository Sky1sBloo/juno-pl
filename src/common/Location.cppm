module;
export module junopl.common.location;

namespace JunoPL {
/**
Position in the source file, mirroring the fields the lexer attaches to
tokens. A default constructed location (line == -1) means "unknown".
 */
export struct SourceLocation {
    int line = -1;
    int colStart = -1;
    int colEnd = -1;

    bool isValid() const { return line >= 0; }
};
} // namespace JunoPL
