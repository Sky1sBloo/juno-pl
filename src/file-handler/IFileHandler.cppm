module;
#include <expected>
#include <filesystem>
#include <string>
export module junopl.ifilehandler;

export namespace JunoPL {
enum class FileHandlerErrorCode {
    FileNotFound,
    FileNotReadable,
    FileNotOpen,
    FileEOF,
    FileNotWritable
};

struct FileHandlerError {
    FileHandlerErrorCode code;
    std::filesystem::path path;
    std::string osMessage; // human-readable OS error (from errno), may be empty

    std::string message() const {
        switch (code) {
        case FileHandlerErrorCode::FileNotFound:
            return std::string("no such file or directory: '") + path.string() + "'";
        case FileHandlerErrorCode::FileNotReadable:
            return std::string("cannot read file: '") + path.string() + "'";
        case FileHandlerErrorCode::FileNotOpen:
            return std::string("file is not open: '") + path.string() + "'";
        case FileHandlerErrorCode::FileEOF:
            return std::string("unexpected end of file: '") + path.string() + "'";
        case FileHandlerErrorCode::FileNotWritable:
            return std::string("cannot write to file: '") + path.string() + "'";
        }
        return std::string("file error: '") + path.string() + "'";
    }
};

class IFileHandler {
  public:
    virtual ~IFileHandler() = default;
    virtual std::expected<char, FileHandlerError> read() = 0;
    virtual std::expected<void, FileHandlerError>
    write(const std::string &data) = 0;
};
}
