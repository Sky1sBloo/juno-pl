module;

#include <expected>
#include <filesystem>
#include <fstream>

export module junopl.filereader;
import junopl.ifilereader;

export namespace JunoPL {
class FileReader : public IFileReader {
  public:
    static std::expected<FileReader, FileReaderError>
    open(const std::filesystem::path &path) {
        return FileReader(path);
    }

    std::expected<char, FileReaderError> read();

  private:
    FileReader(std::filesystem::path path) : mFileStream(path) {}
    std::ifstream mFileStream;
};

}
