module;
#include <expected>
export module junopl.ifilereader;

export namespace JunoPL {
enum class FileReaderError {

    FileNotFound,
    FileNotReadable,
    FileNotOpen,
    FileEOF
};

class IFileReader {
  public:
    virtual ~IFileReader() = default;
    virtual std::expected<char, FileReaderError> read() = 0;
};
}
