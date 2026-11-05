module;
#include <expected>
#include <string>
module junopl.tests.filehandler;

namespace JunoPL::Tests {
FileHandlerTest::FileHandlerTest()
    : mSourceCode(), charPos(0), mWrittenData() {}

FileHandlerTest::FileHandlerTest(const std::string &sourceCode)
    : mSourceCode(sourceCode), charPos(0), mWrittenData() {}

void FileHandlerTest::setSourceCode(const std::string &sourceCode) {
    mSourceCode = sourceCode;
    charPos = 0;
}

std::expected<char, FileHandlerError> FileHandlerTest::read() {
    if (charPos >= mSourceCode.size()) {
        return std::unexpected(FileHandlerError::FileEOF);
    }
    char c = mSourceCode.at(charPos);
    charPos++;
    return c;
}

std::expected<void, FileHandlerError> FileHandlerTest::write(
    const std::string &data) {
    mWrittenData += data;
    return {};
}
}
