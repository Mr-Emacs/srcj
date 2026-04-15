#pragma once

#include <cstdint>
#include <cctype>
#include <string_view>
#include <format>

enum class TokenKind : uint8_t {
    TLPARENT,
    TRPARENT,
    TEQUAL,
    TNUMBER,
    TIDENT,
    TDOT,
    TNOT,
    TAMPRESAND,
    TBAR,
    TGREAT,
    TLESS,
    TEOF,
    TUNKNOWN,
    TCOLON,
    TRBRACE,
    TLBRACE,
    TRSBRACE,
    TLSBRACE,
    TSEMICOLON,
    TDQUOTE,
    TSQUOTE,
    TCOLONEQUAL,
};

// TODO: Add colomun so we can have better diagnostics
struct Lexer {
    std::string src;
    std::size_t pos;
    char peek();
    char peek(std::size_t n);
    char advance();
    void skip_whitespace();
};

struct Token {
    TokenKind kind;
    std::string name;
    std::size_t pos;
    void print_token();
};

Token token_next(Lexer &l);
constexpr std::string token_kind_name(TokenKind as);

struct Parser {
    Lexer lex;
    Token cur;
    void error_at(Token t, const std::string &msg);
    void advance();
    bool expect(TokenKind kind);
};

void parse_function(Parser &p);
void parse_block(Parser &p);
void parse_statement(Parser &p);
void parse_value(Parser &p, const std::string& var_name);

#if defined(LEXER_IMPLEMENTATION)
char Lexer::peek()
{
    return (pos < src.length()) ? src[pos] : '\0';
}

char Lexer::peek(std::size_t n)
{
    return (pos + n < src.length()) ? src[pos + n] : '\0';
}

char Lexer::advance()
{
    return (pos < src.length()) ? src[pos++] : '\0';
}

void Lexer::skip_whitespace()
{
    while(pos < src.length() && isspace((unsigned char)peek())) {
        advance();

    }
}

Token token_next(Lexer &l)
{
    l.skip_whitespace();
    if (l.pos >= l.src.length()) {
        return Token{ .kind = TokenKind::TEOF, .pos = l.pos };
    }

    char c = l.peek();
    if (c == '=') {
        std::size_t p = l.pos;
        l.advance();
        return (Token) { .kind = TokenKind::TEQUAL    , .pos = p };
    }
    if (c == '.') {
        std::size_t p = l.pos;
        l.advance();
        return (Token) { .kind = TokenKind::TDOT      , .pos = p };
    }
    if (c == '(') {
        std::size_t p = l.pos;
        l.advance();
        return (Token) { .kind = TokenKind::TLPARENT  , .pos = p  };
    }
    if (c == ')') {
        std::size_t p = l.pos;
        l.advance();
        return (Token) { .kind = TokenKind::TRPARENT  , .pos = p };
    }
    if (c == '!') {
        std::size_t p = l.pos;
        l.advance();
        return (Token) { .kind = TokenKind::TNOT      , .pos = p };
    }
    if (c == '&') {
        std::size_t p = l.pos;
        l.advance();
        return (Token) { .kind = TokenKind::TAMPRESAND, .pos = p };
    }
    if (c == '|') {
        std::size_t p = l.pos;
        l.advance();
        return (Token) { .kind = TokenKind::TBAR      , .pos = p };
    }
    if (c == '>') {
        std::size_t p = l.pos;
        l.advance();
        return (Token) { .kind = TokenKind::TGREAT    , .pos = p };
    }
    if (c == '<') {
        std::size_t p = l.pos;
        l.advance();
        return (Token) { .kind = TokenKind::TLESS     , .pos = p };
    }
    if (c == ':') {
        std::size_t p = l.pos;
        l.advance();
        if (l.peek() == '=') {
            l.advance();
            return (Token) { .kind = TokenKind::TCOLONEQUAL , .pos = p };
        }
        return (Token) { .kind = TokenKind::TCOLON     , .pos = p };
    }
    if (c == '{') {
        std::size_t p = l.pos;
        l.advance();
        return (Token) { .kind = TokenKind::TLBRACE     , .pos = p };
    }
    if (c == '}') {
        std::size_t p = l.pos;
        l.advance();
        return (Token) { .kind = TokenKind::TRBRACE     , .pos = p };
    }
    if (c == '[') {
        std::size_t p = l.pos;
        l.advance();
        return (Token) { .kind = TokenKind::TLSBRACE     , .pos = p };
    }
    if (c == ']') {
        std::size_t p = l.pos;
        l.advance();
        return (Token) { .kind = TokenKind::TRSBRACE     , .pos = p };
    }

    if (c == ';') {
        std::size_t p = l.pos;
        l.advance();
        return (Token) { .kind = TokenKind::TSEMICOLON   , .pos = p };
    }
    if (c == '"') {
        std::size_t p = l.pos;
        l.advance();
        return (Token) { .kind = TokenKind::TDQUOTE   , .pos = p };
    }

    if (c == '\'') {
        std::size_t p = l.pos;
        l.advance();
        return (Token) { .kind = TokenKind::TSQUOTE   , .pos = p };
    }

    if (isdigit((unsigned char)c)) {
        std::size_t start = l.pos;
        while (isdigit((unsigned char)l.peek())) l.advance();
        return (Token) {
            .kind = TokenKind::TNUMBER,
            .name = l.src.substr(start, l.pos - start),
            .pos = start
        };
    }

    if (isalpha((unsigned char)c)) {
        std::size_t start = l.pos;
        while (isalnum((unsigned char)l.peek()) ||
               l.peek() == '_') l.advance();
        return (Token) {
            .kind = TokenKind::TIDENT,
            .name = l.src.substr(start, l.pos - start),
            .pos = start
        };
    }

    std::cerr << std::format("Unexpected char '{}'", c) << std::endl;
    l.advance();
    return (Token) { .kind = TokenKind::TUNKNOWN , .pos = l.pos };
}

constexpr std::string token_kind_name(TokenKind kind)
{
    switch(kind) {
        case TokenKind::TLPARENT:    return "LPARENT";
        case TokenKind::TRPARENT:    return "RPARENT";
        case TokenKind::TEQUAL:      return "EQUAL";
        case TokenKind::TIDENT:      return "IDENT";
        case TokenKind::TNUMBER:     return "NUMBER";
        case TokenKind::TDOT:        return "DOT";
        case TokenKind::TNOT:        return "NOT";
        case TokenKind::TAMPRESAND:  return "AMPRESAND";
        case TokenKind::TBAR:        return "BAR";
        case TokenKind::TEOF:        return "EOF";
        case TokenKind::TGREAT:      return "GREATER";
        case TokenKind::TLESS:       return "LESSER";
        case TokenKind::TUNKNOWN:    return "UNKNOWN";
        case TokenKind::TCOLON:      return "COLON";
        case TokenKind::TRBRACE:     return "RBRACE";
        case TokenKind::TLBRACE:     return "LBRACE";
        case TokenKind::TRSBRACE:    return "RSBRACE";
        case TokenKind::TLSBRACE:    return "LSBRACE";
        case TokenKind::TSEMICOLON:  return "SEMICOLON";
        case TokenKind::TSQUOTE:     return "SQUOTE";
        case TokenKind::TDQUOTE:     return "DQUOTE";
        case TokenKind::TCOLONEQUAL: return "COLONEQUAL";
        default: return "INVALID";
    }
}

void Token::print_token()
{
    if (name.length() > 0) {
        std::cout <<
        std::format("Token Kind: {} -> Token Name: {}", token_kind_name(kind), name)
        << std::endl;
    } else {
        std::cout <<
        std::format("Token Kind: {}", token_kind_name(kind), name)
        << std::endl;
    }
}

void Parser::advance()
{
    cur = token_next(lex);
}

void Parser::error_at(Token token, const std::string &msg)
{
    std::cerr << msg << std::endl;

    std::size_t line_start = lex.src.rfind('\n', token.pos);
    if (line_start == std::string::npos) line_start = 0;
    else line_start += 1;

    std::size_t line_end = lex.src.find('\n', token.pos);
    if (line_end == std::string::npos) line_end = lex.src.size();

    std::string line = lex.src.substr(line_start, line_end - line_start);

    std::cerr << std::format("    {}\n", line);

    std::size_t col = token.pos - line_start;
    std::string pad(col, ' ');

    std::cerr << std::format("    {}^\n", pad);
}

bool Parser::expect(TokenKind kind)
{
    if (cur.kind != kind) {
        std::string msg = std::format("Expected {} but got {}\n",
                          token_kind_name(kind), token_kind_name(cur.kind));

        error_at(cur, msg);
        return false;
    }
    return true;
}

void parse_function(Parser &p)
{
    if (p.cur.kind != TokenKind::TIDENT || p.cur.name != "fn") {
        p.error_at(p.cur, "Expected keyword 'fn'.");
        return;
    }

    std::cout << "Parsed fn keyword\n";
    p.advance();
    if (p.cur.kind != TokenKind::TIDENT) {
        p.error_at(p.cur, "Expected function name can not be empty.");
        return;
    }
    std::cout << std::format("Parsed function name {}\n", p.cur.name);
    p.advance();
    if (!p.expect(TokenKind::TLPARENT)) return;

    std::cout << "Parsed '('\n";
    p.advance();

    if (!p.expect(TokenKind::TRPARENT)) return;
    std::cout << "Parsed ')'\n";
    p.advance();

    parse_block(p);
}

void parse_block(Parser &p)
{
    if (!p.expect(TokenKind::TLBRACE)) return;
    std::cout << "Parsed '{'\n";
    p.advance();

    while(p.cur.kind != TokenKind::TRBRACE && p.cur.kind != TokenKind::TEOF) {
        parse_statement(p);
    }

    if (!p.expect(TokenKind::TRBRACE)) return;
    std::cout << "Parsed '}'\n";
    p.advance();
}

void parse_statement(Parser &p)
{
    if (!p.expect(TokenKind::TIDENT)) {
        p.advance();
        return;
    }
    std::string var_name = p.cur.name;
    p.advance();

    if (!p.expect(TokenKind::TCOLONEQUAL)) {
        p.advance();
        return;
    }
    p.advance();
    parse_value(p, var_name);

    if (!p.expect(TokenKind::TSEMICOLON)) {
        p.advance();
        return;
    }
    std::cout << "Parsed ';'\n";
    p.advance();
}

void parse_value(Parser &p, const std::string& var_name)
{
    if (p.cur.kind == TokenKind::TNUMBER) {
        std::cout << std::format("Parsed: var {} := number {}\n", var_name, p.cur.name);
        p.advance();
    }
    else if (p.cur.kind == TokenKind::TIDENT
    && (p.cur.name == "true" || p.cur.name == "false")) {
        std::cout << std::format("Parsed: var {} := bool {}\n", var_name, p.cur.name);
        p.advance();
    }
    else if (p.cur.kind == TokenKind::TDQUOTE) {
        std::size_t start = p.cur.pos + 1;

        while(p.lex.pos < p.lex.src.size() && p.lex.peek() != '"') p.lex.advance();

        std::string str_value = p.lex.src.substr(start, p.lex.pos - start);

        p.lex.advance();
        p.advance();
        std::cout << std::format("Parsed: var {} := String {}\n", var_name, str_value);
    } else {
        p.error_at(p.cur, std::format("Unexpected value token '{}'", token_kind_name(p.cur.kind)));
        p.advance();
    }

}
#endif
