module;
#include <cstddef>
#include <expected>
#include <string>
export module junopl.tests.filehandler;
import junopl.ifilehandler;

namespace JunoPL::Tests {
/**
For intializing with string rather than reading a file directly
 */
export class FileHandlerTest : public IFileHandler {
  public:
    FileHandlerTest();
    FileHandlerTest(const std::string &sourceCode);

    void setSourceCode(const std::string &sourceCode);

    std::expected<char, FileHandlerError> read() override;
    std::expected<void, FileHandlerError>
    write(const std::string &data) override;

  private:
    std::string mSourceCode;
    std::size_t charPos;
    std::string mWrittenData;
};
}
