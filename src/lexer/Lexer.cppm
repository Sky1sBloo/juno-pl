module;
export module junopl.lexer;
import junopl.ifilereader;
namespace JunoPL {
class Lexer {
  public:
    Lexer(IFileReader &fileReader) : mFileReader(fileReader) {}

  private:
    IFileReader &mFileReader;
    enum class State { START, IDENT };
};
}
