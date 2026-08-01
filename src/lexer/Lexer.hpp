#pragma once
#include "IFileReader.hpp"

namespace JunoPL {
class Lexer {
  public:
    Lexer(IFileReader &fileReader);

  private:
    IFileReader &mFileReader;

    enum class State { START, IDENT };
};
}
