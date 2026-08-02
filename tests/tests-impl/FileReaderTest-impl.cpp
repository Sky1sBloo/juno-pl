module;
#include <expected>
#include <string>
module junopl.tests.filereader;

namespace JunoPL::Tests {
FileReaderTest::FileReaderTest() : mSourceCode(), charPos(0) {}

FileReaderTest::FileReaderTest(const std::string &sourceCode)
    : mSourceCode(sourceCode), charPos(0) {}

void FileReaderTest::setSourceCode(const std::string &sourceCode) {
    mSourceCode = sourceCode;
}

std::expected<char, FileReaderError> FileReaderTest::read() {
    char c = mSourceCode.at(c);
    charPos++;
    if (charPos > mSourceCode.size()) {
        return std::unexpected(FileReaderError::FileEOF);
    }
    return c;
}
}
