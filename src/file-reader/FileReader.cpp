#include "FileReader.hpp"

namespace JunoPL {
FileReader::FileReader(std::filesystem::path path) : mFileStream(path) {}

std::expected<char, FileReaderError> FileReader::read() {
    char c;
    if (mFileStream.get(c)) {
        return c;
    } else {
        return std::unexpected(FileReaderError::FileEOF);
    }
}
}