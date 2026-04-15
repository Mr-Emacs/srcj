#include <iostream>
#include <fstream>
#include <sstream>
#define LEXER_IMPLEMENTATION
#include "lexer.h"

std::string read_file(const std::string &path)
{
    std::fstream fs(path);
    if (!fs.is_open()) return "";

    std::stringstream content_buf;
    content_buf << fs.rdbuf();

    fs.close();
    return content_buf.str();
}

int main()
{
    Lexer l {
        .src = read_file("src.j"),
        .pos = 0,
    };

    Parser p;
    p.lex = l;
    p.advance();

    while (p.cur.kind != TokenKind::TEOF) {
        if (p.cur.kind == TokenKind::TIDENT && p.cur.name == "fn") {
            parse_function(p);
        } else {
            p.error_at(p.cur, std::format("Unexpected top-level token '{}'",
                       token_kind_name(p.cur.kind)));
            p.advance();
        }
    }

    return 0;
}
