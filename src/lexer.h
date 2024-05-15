#ifndef __AXE_LEXER_H__

#define __AXE_LEXER_H__

#include "token.h"
#include <string>

namespace axe {

class lexer {
  public:
    lexer(const std::string& input);
    token next_token();

  private:
    const std::string& input;
    size_t position;
    char ch;

    char peek_char();
    void read_char();
    void skip_whitespace();
    std::string read_ident();
    std::string read_integer();
    std::string read_string();
};
} // namespace axe

#endif // __AXE_LEXER_H__
