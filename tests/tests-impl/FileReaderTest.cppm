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
export class FileReaderTest : public IFileReader {
  public:
    FileReaderTest();
    FileReaderTest(const std::string &sourceCode);

    void setSourceCode(const std::string &sourceCode); 

    std::expected<char, FileReaderError> read() override;

  private:
    std::string mSourceCode;
    std::size_t charPos;
};
}