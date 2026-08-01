#pragma once
#include <expected>

enum FileReaderError { FileNotFound, FileNotReadable, FileNotOpen, FileEOF };

class IFileReader {
  public:
    virtual ~IFileReader() = default;
    virtual std::expected<char, FileReaderError> read() = 0;
};