#pragma once

#include <cstdint>
#include <cctype>
#include <string_view>
#include <format>
#include "common.h"
#include "resolver.h"
#include "ast.h"

namespace lang::lex
{
    enum class TokenKind : uint8_t
    {
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
        TSTRING,
        TCOUNT,
    };

    struct Lexer
    {
        std::string src;
        std::size_t pos;
        char        peek();
        char        peek(std::size_t n);
        char        advance();
        void        skip_whitespace();
    };

    struct Token
    {
        TokenKind   kind;
        std::string name;
        std::size_t pos;
        void        print_token();
    };

    Token                 token_next(Lexer& l);
    constexpr std::string token_kind_name(TokenKind kind);

    struct Parser
    {
        Lexer lex;
        Token cur;
        void  error_at(Token t, const std::string& msg);
        void  advance();
        bool  expect(TokenKind kind);
    };

    std::unique_ptr<Node> parse_function(Parser& p);
    std::unique_ptr<Node> parse_block(Parser& p);
    std::unique_ptr<Node> parse_statement(Parser& p);
    std::unique_ptr<Node> parse_value(Parser& p);

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
        while (pos < src.length() && isspace(static_cast<unsigned char>(peek())))
            advance();
    }

    Token token_next(Lexer& l)
    {
        l.skip_whitespace();
        if (l.pos >= l.src.length())
            return Token{.kind = TokenKind::TEOF, .pos = l.pos};

        char c = l.peek();
        if (c == '=')
        {
            std::size_t p = l.pos;
            l.advance();
            return {.kind = TokenKind::TEQUAL, .pos = p};
        }
        if (c == '.')
        {
            std::size_t p = l.pos;
            l.advance();
            return {.kind = TokenKind::TDOT, .pos = p};
        }
        if (c == '(')
        {
            std::size_t p = l.pos;
            l.advance();
            return {.kind = TokenKind::TLPARENT, .pos = p};
        }
        if (c == ')')
        {
            std::size_t p = l.pos;
            l.advance();
            return {.kind = TokenKind::TRPARENT, .pos = p};
        }
        if (c == '!')
        {
            std::size_t p = l.pos;
            l.advance();
            return {.kind = TokenKind::TNOT, .pos = p};
        }
        if (c == '&')
        {
            std::size_t p = l.pos;
            l.advance();
            return {.kind = TokenKind::TAMPRESAND, .pos = p};
        }
        if (c == '|')
        {
            std::size_t p = l.pos;
            l.advance();
            return {.kind = TokenKind::TBAR, .pos = p};
        }
        if (c == '>')
        {
            std::size_t p = l.pos;
            l.advance();
            return {.kind = TokenKind::TGREAT, .pos = p};
        }
        if (c == '<')
        {
            std::size_t p = l.pos;
            l.advance();
            return {.kind = TokenKind::TLESS, .pos = p};
        }
        if (c == '{')
        {
            std::size_t p = l.pos;
            l.advance();
            return {.kind = TokenKind::TLBRACE, .pos = p};
        }
        if (c == '}')
        {
            std::size_t p = l.pos;
            l.advance();
            return {.kind = TokenKind::TRBRACE, .pos = p};
        }
        if (c == '[')
        {
            std::size_t p = l.pos;
            l.advance();
            return {.kind = TokenKind::TLSBRACE, .pos = p};
        }
        if (c == ']')
        {
            std::size_t p = l.pos;
            l.advance();
            return {.kind = TokenKind::TRSBRACE, .pos = p};
        }
        if (c == ';')
        {
            std::size_t p = l.pos;
            l.advance();
            return {.kind = TokenKind::TSEMICOLON, .pos = p};
        }
        if (c == '\'')
        {
            std::size_t p = l.pos;
            l.advance();
            return {.kind = TokenKind::TSQUOTE, .pos = p};
        }
        if (c == '"')
        {
            std::size_t start = l.pos;
            l.advance();

            // TODO: Add escape sequence handling here if needed
            while (l.pos < l.src.size() && l.peek() != '"')
            {
                l.advance();
            }

            // NOTE: Unterminated string
            if (l.pos >= l.src.size())
            {
                std::cerr << std::format("Unterminated string literal starting at position {}\n", start);
                return Token{.kind = TokenKind::TUNKNOWN, .pos = start};
            }

            l.advance();

            std::string value = l.src.substr(start + 1, l.pos - start - 2);
            return Token{.kind = TokenKind::TSTRING, .name = value, .pos = start};
        }

        if (c == ':')
        {
            std::size_t p = l.pos;
            l.advance();
            if (l.peek() == '=')
            {
                l.advance();
                return {.kind = TokenKind::TCOLONEQUAL, .pos = p};
            }
            return {.kind = TokenKind::TCOLON, .pos = p};
        }
        if (isdigit(static_cast<unsigned char>(c)))
        {
            std::size_t start = l.pos;
            while (isdigit(static_cast<unsigned char>(l.peek()))) l.advance();
            return {.kind = TokenKind::TNUMBER, .name = l.src.substr(start, l.pos - start), .pos = start};
        }
        if (isalpha(static_cast<unsigned char>(c)))
        {
            std::size_t start = l.pos;
            while (isalnum(static_cast<unsigned char>(l.peek())) || l.peek() == '_') l.advance();
            return {.kind = TokenKind::TIDENT, .name = l.src.substr(start, l.pos - start), .pos = start};
        }

        std::cerr << std::format("Unexpected char '{}'\n", c);
        l.advance();
        return {.kind = TokenKind::TUNKNOWN, .pos = l.pos};
    }

    constexpr std::string token_kind_name(TokenKind kind)
    {
        constexpr std::string_view names[] = {
            "LPARENT", "RPARENT", "EQUAL", "NUMBER", "IDENT", "DOT", "NOT", "AMPRESAND", "BAR", "GREATER", "LESSER", "EOF", "UNKNOWN", "COLON", "RBRACE", "LBRACE", "RSBRACE", "LSBRACE", "SEMICOLON", "DQUOTE", "SQUOTE", "COLONEQUAL", "STRING"};
        static_assert(std::size(names) == static_cast<std::size_t>(TokenKind::TCOUNT),
                      "token_kind_name is missing an entry — add it");
        auto i = static_cast<std::size_t>(kind);
        return i < static_cast<std::size_t>(TokenKind::TCOUNT) ? std::string(names[i]) : "INVALID";
    }

    void Token::print_token()
    {
        if (!name.empty())
            std::cout << std::format("Token Kind: {} -> Token Name: {}\n", token_kind_name(kind), name);
        else
            std::cout << std::format("Token Kind: {}\n", token_kind_name(kind));
    }

    void Parser::advance()
    {
        cur = token_next(lex);
    }

    void Parser::error_at(Token token, const std::string& msg)
    {
        std::size_t line_start = 0;
        int         line_num   = 1;

        for (std::size_t i = 0; i < token.pos && i < lex.src.size(); i++)
        {
            if (lex.src[i] == '\n')
            {
                line_start = i + 1;
                line_num++;
            }
        }

        std::size_t line_end = lex.src.find('\n', token.pos);
        if (line_end == std::string::npos) line_end = lex.src.size();

        std::string line = lex.src.substr(line_start, line_end - line_start);

        std::cerr << std::format("Error at line {}: {}\n", line_num, msg);
        std::cerr << std::format("    {}\n", line);
        std::cerr << std::format("    {}^\n", std::string(token.pos - line_start, ' '));
    }

    bool Parser::expect(TokenKind kind)
    {
        if (cur.kind != kind)
        {
            error_at(cur, std::format("Expected {} but got {}", token_kind_name(kind), token_kind_name(cur.kind)));
            return false;
        }
        return true;
    }

    std::unique_ptr<Node> parse_value(Parser& p)
    {
        if (p.cur.kind == TokenKind::TNUMBER)
        {
            std::size_t value = std::stoull(p.cur.name);
            p.advance();
            return std::make_unique<Node>(NodeKind::NUMBER, NumberLiteral{value});
        }
        if (p.cur.kind == TokenKind::TIDENT &&
            (p.cur.name == "true" || p.cur.name == "false"))
        {
            bool value = p.cur.name == "true";
            p.advance();
            return std::make_unique<Node>(NodeKind::BOOLEAN, BooleanLiteral{value});
        }
        if (p.cur.kind == TokenKind::TIDENT)
        {
            std::string name = p.cur.name;
            p.advance();
            return std::make_unique<Node>(NodeKind::IDENT, Identifier{name});
        }
        if (p.cur.kind == TokenKind::TSTRING)
        {
            std::string value = p.cur.name;
            p.advance();
            return std::make_unique<Node>(NodeKind::STRING, StringLiteral{value});
        }

        p.error_at(p.cur, std::format("Unexpected value token '{}'", token_kind_name(p.cur.kind)));
        p.advance();
        return nullptr;
    }

    std::unique_ptr<Node> parse_statement(Parser& p)
    {
        Token start_token = p.cur;

        if (!p.expect(TokenKind::TIDENT))
        {
            p.advance();
            return nullptr;
        }

        std::string var_name = p.cur.name;
        p.advance();

        if (p.cur.kind == TokenKind::TLPARENT)
        {
            p.advance();
            std::vector<std::unique_ptr<Node>> args;

            while (p.cur.kind != TokenKind::TRPARENT && p.cur.kind != TokenKind::TEOF)
            {
                auto arg = parse_value(p);
                if (arg) args.push_back(std::move(arg));
            }

            Token rparen_token = p.cur;

            if (!p.expect(TokenKind::TRPARENT)) return nullptr;
            p.advance();

            if (p.cur.kind != TokenKind::TSEMICOLON)
            {
                Token expected_semicolon;
                expected_semicolon.pos  = rparen_token.pos + 1;
                expected_semicolon.kind = TokenKind::TSEMICOLON;

                p.error_at(expected_semicolon, std::format("Missing ';' after function call to '{}'", var_name));
                return nullptr;
            }
            p.advance();
            return std::make_unique<Node>(NodeKind::FUNCTIONCALL,
                                          FunctionCallNode{var_name, std::move(args)});
        }

        if (!p.expect(TokenKind::TCOLONEQUAL))
        {
            p.advance();
            return nullptr;
        }
        p.advance();

        Token value_start = p.cur;
        auto  value       = parse_value(p);

        if (!value) return nullptr;

        if (p.cur.kind != TokenKind::TSEMICOLON)
        {
            Token expected_semicolon;
            if (value_start.kind == TokenKind::TNUMBER ||
                value_start.kind == TokenKind::TSTRING ||
                value_start.kind == TokenKind::TIDENT)
            {
                expected_semicolon.pos = value_start.pos + value_start.name.length();
            }
            else
            {
                expected_semicolon.pos = p.cur.pos;
            }
            expected_semicolon.kind = TokenKind::TSEMICOLON;

            p.error_at(expected_semicolon, std::format("Missing ';' after assignment to '{}'", var_name));
            return nullptr;
        }
        p.advance();

        return std::make_unique<Node>(NodeKind::ASSIGN,
                                      AssignNode{var_name, std::move(value)});
    }

    std::unique_ptr<Node> parse_block(Parser& p)
    {
        if (!p.expect(TokenKind::TLBRACE)) return nullptr;
        p.advance();

        BlockNode block;
        while (p.cur.kind != TokenKind::TRBRACE && p.cur.kind != TokenKind::TEOF)
        {
            auto stmt = parse_statement(p);
            if (stmt) block.statements.push_back(std::move(stmt));
        }

        if (!p.expect(TokenKind::TRBRACE)) return nullptr;
        p.advance();

        return std::make_unique<Node>(NodeKind::BLOCK, std::move(block));
    }

    std::unique_ptr<Node> parse_function(Parser& p)
    {
        if (p.cur.kind != TokenKind::TIDENT || p.cur.name != "fn")
        {
            p.error_at(p.cur, "Expected keyword 'fn'.");
            return nullptr;
        }
        p.advance();

        if (!p.expect(TokenKind::TIDENT)) return nullptr;
        std::string name = p.cur.name;
        p.advance();

        if (!p.expect(TokenKind::TLPARENT)) return nullptr;

        Token lparen_token = p.cur;
        p.advance();

        if (p.cur.kind != TokenKind::TRPARENT)
        {
            Token expected_rparen;
            expected_rparen.pos  = lparen_token.pos + 1;
            expected_rparen.kind = TokenKind::TRPARENT;

            p.error_at(expected_rparen, std::format("Expected ')' after function '{}' parameters", name));

            while (p.cur.kind != TokenKind::TLBRACE && p.cur.kind != TokenKind::TEOF)
            {
                p.advance();
            }

            if (p.cur.kind == TokenKind::TLBRACE)
            {
                auto body = parse_block(p);
                if (!body) return nullptr;
                return std::make_unique<Node>(NodeKind::FUNCTIONDECL,
                                              FunctionDeclNode{name, std::move(body)});
            }
            return nullptr;
        }
        p.advance();

        auto body = parse_block(p);
        if (!body) return nullptr;

        return std::make_unique<Node>(NodeKind::FUNCTIONDECL,
                                      FunctionDeclNode{name, std::move(body)});
    }

#endif
};
