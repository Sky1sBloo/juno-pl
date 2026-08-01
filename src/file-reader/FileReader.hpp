#pragma once

#include "IFileReader.hpp"

#include <expected>
#include <filesystem>
#include <fstream>

namespace JunoPL {
class FileReader : public IFileReader {
  public:
    static std::expected<FileReader, FileReaderError>
    open(const std::filesystem::path &path);

    std::expected<char, FileReaderError> read();

  private:
    FileReader(std::filesystem::path path);
    std::ifstream mFileStream;
};
}
