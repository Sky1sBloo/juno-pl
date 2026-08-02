module;
#include <cstddef>
#include <expected>
#include <string>
export module junopl.tests.filereader;
import junopl.ifilereader;

namespace JunoPL::Tests {
/**
For intializing with string rather than reading a file directly
 */
export class FileReaderTest : IFileReader {
  public:
    FileReaderTest() : mSourceCode() {}
    FileReaderTest(const std::string &sourceCode) : mSourceCode(sourceCode) {}

    void setSourceCode(const std::string &sourceCode) {
        mSourceCode = sourceCode;
    }

    std::expected<char, FileReaderError> read() override;

  private:
    std::string mSourceCode;
    std::size_t charPos;
};
}