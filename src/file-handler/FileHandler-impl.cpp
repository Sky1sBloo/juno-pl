module;
#include <expected>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>
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

std::expected<std::vector<std::string>, FileHandlerError>
FileHandler::readLines(const std::filesystem::path &path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return std::unexpected(FileHandlerError{
            FileHandlerErrorCode::FileNotFound, path, osErrorMessage()});
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back(); // CRLF files
        }
        lines.push_back(std::move(line));
    }
    if (in.bad()) {
        return std::unexpected(FileHandlerError{
            FileHandlerErrorCode::FileNotReadable, path, osErrorMessage()});
    }
    return lines;
}

bool FileHandler::exists(const std::filesystem::path &path) {
    std::error_code ec;
    return std::filesystem::exists(path, ec);
}
}
