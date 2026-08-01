module;
#include <expected>
export module junopl.ifilereader;

export enum class FileReaderError {
    FileNotFound,
    FileNotReadable,
    FileNotOpen,
    FileEOF
};

export namespace JunoPL {
class IFileReader {
  public:
    virtual ~IFileReader() = default;
    virtual std::expected<char, FileReaderError> read() = 0;
};
}
