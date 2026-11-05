module;
#include <expected>
#include <string>
export module junopl.ifilehandler;

export namespace JunoPL {
enum class FileHandlerError {

    FileNotFound,
    FileNotReadable,
    FileNotOpen,
    FileEOF,
    FileNotWritable
};

class IFileHandler {
  public:
    virtual ~IFileHandler() = default;
    virtual std::expected<char, FileHandlerError> read() = 0;
    virtual std::expected<void, FileHandlerError>
    write(const std::string &data) = 0;
};
}
