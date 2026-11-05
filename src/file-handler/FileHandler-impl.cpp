module;
#include <cerrno>
#include <cstring>
#include <expected>
#include <fstream>
#include <string>
module junopl.filehandler;
import junopl.ifilehandler;

namespace JunoPL {
std::expected<char, FileHandlerError> FileHandler::read() {
    if (!mFileStream.is_open()) {
        return std::unexpected(FileHandlerError{
            FileHandlerErrorCode::FileNotOpen, mPath, osErrorMessage()});
    }

    char c;
    if (!mFileStream.get(c)) {
        if (mFileStream.eof()) {
            return std::unexpected(FileHandlerError{
                FileHandlerErrorCode::FileEOF, mPath, osErrorMessage()});
        } else {
            return std::unexpected(FileHandlerError{
                FileHandlerErrorCode::FileNotReadable, mPath,
                osErrorMessage()});
        }
    }

    return c;
}

std::expected<void, FileHandlerError> FileHandler::write(
    const std::string &data) {
    std::ofstream out(mPath, std::ios::app);
    if (!out.is_open()) {
        return std::unexpected(FileHandlerError{
            FileHandlerErrorCode::FileNotOpen, mPath, osErrorMessage()});
    }
    out << data;
    if (!out.good()) {
        return std::unexpected(FileHandlerError{
            FileHandlerErrorCode::FileNotWritable, mPath, osErrorMessage()});
    }
    return {};
}
}
