module;
#include <expected>
#include <filesystem>
#include <fstream>
#include <string>

export module junopl.filehandler;
export import junopl.ifilehandler;

export namespace JunoPL {
class FileHandler : public IFileHandler {
  public:
    static std::expected<FileHandler, FileHandlerError>
    open(const std::filesystem::path &path) {
        FileHandler reader(path);
        if (!reader.mFileStream.is_open()) {
            return std::unexpected(FileHandlerError::FileNotFound);
        }
        return reader;
    }

    static std::expected<void, FileHandlerError>
    writeToFile(const std::filesystem::path &path,
                const std::string &data) {
        std::ofstream out(path);
        if (!out.is_open()) {
            return std::unexpected(FileHandlerError::FileNotOpen);
        }
        out << data;
        if (!out.good()) {
            return std::unexpected(FileHandlerError::FileNotWritable);
        }
        return {};
    }

    std::expected<char, FileHandlerError> read() override;
    std::expected<void, FileHandlerError>
    write(const std::string &data) override;

  private:
    FileHandler(std::filesystem::path path)
        : mFileStream(path), mPath(path) {}
    std::ifstream mFileStream;
    std::filesystem::path mPath;
};
}
