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
    charPos = 0;
}

std::expected<char, FileReaderError> FileReaderTest::read() {
    if (charPos > mSourceCode.size()) {
        return std::unexpected(FileReaderError::FileEOF);
    }
    char c = mSourceCode.at(charPos);
    charPos++;
    return c;
}
}
