#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#define LEXER_IMPLEMENTATION
#include "lexer.h"
#include "printer.h"
#include "resolve.h"

std::string read_file(const std::string& path)
{
    std::fstream fs(path);
    if (!fs.is_open()) return "";
    std::stringstream buf;
    buf << fs.rdbuf();
    return buf.str();
}

int main()
{
    lang::lex::Lexer  l{.src = read_file("src.jl"), .pos = 0};
    lang::lex::Parser p;
    p.lex = l;
    p.advance();

    std::vector<std::unique_ptr<lang::Node>> tree;
    while (p.cur.kind != lang::lex::TokenKind::TEOF)
    {
        if (p.cur.kind == lang::lex::TokenKind::TIDENT && p.cur.name == "fn")
        {
            auto node = parse_function(p);
            if (node) tree.push_back(std::move(node));
        }
        else
        {
            p.error_at(p.cur, std::format("Unexpected top-level token '{}'", token_kind_name(p.cur.kind)));
            p.advance();
        }
    }

    lang::Resolver resolver;
    std::cout << "\n----Resolved AST PRINT ----\n";
    for (auto& node : tree)
    {
        node = resolve_node(resolver, std::move(node));
        if (node) print_node(*node);
    }

    return 0;
}
