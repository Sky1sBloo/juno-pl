module;
#include <expected>
#include <fstream>
#include <string>
module junopl.filehandler;
import junopl.ifilehandler;

namespace JunoPL {
std::expected<char, FileHandlerError> FileHandler::read() {
    if (!mFileStream.is_open()) {
        return std::unexpected(FileHandlerError::FileNotOpen);
    }

    char c;
    if (!mFileStream.get(c)) {
        if (mFileStream.eof()) {
            return std::unexpected(FileHandlerError::FileEOF);
        } else {
            return std::unexpected(FileHandlerError::FileNotReadable);
        }
    }

    return c;
}

std::expected<void, FileHandlerError> FileHandler::write(
    const std::string &data) {
    std::ofstream out(mPath, std::ios::app);
    if (!out.is_open()) {
        return std::unexpected(FileHandlerError::FileNotOpen);
    }
    out << data;
    if (!out.good()) {
        return std::unexpected(FileHandlerError::FileNotWritable);
    }
    return {};
}
}
