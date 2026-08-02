module;
#include <expected>
module junopl.filereader;
import junopl.ifilereader;

namespace JunoPL {
std::expected<char, FileReaderError> FileReader::read() {
    if (!mFileStream.is_open()) {
        return std::unexpected(FileReaderError::FileNotOpen);
    }

    char c;
    if (!mFileStream.get(c)) {
        if (mFileStream.eof()) {
            return std::unexpected(FileReaderError::FileEOF);
        } else {
            return std::unexpected(FileReaderError::FileNotReadable);
        }
    }

    return c;
}
}