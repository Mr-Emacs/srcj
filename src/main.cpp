#define LEXER_IMPLEMENTATION
#include "frontend/lexer.h"
#include "frontend/resolve.h"
#include "frontend/printer.h"
#include "middle/semantic.h"
#include "middle/diagnostic.h"
#include "backend/codegen.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>

static std::vector<std::unique_ptr<lang::Node>>
parse_program(lang::lex::Parser& p)
{
    std::vector<std::unique_ptr<lang::Node>> nodes;

    while (p.cur.kind != lang::lex::TokenKind::TEOF)
    {
        if (p.cur.kind == lang::lex::TokenKind::THASH)
        {
            auto n = lang::lex::parse_load_module(p);
            if (n) nodes.push_back(std::move(n));
            continue;
        }

        if (p.cur.kind == lang::lex::TokenKind::TIDENT && p.cur.name == "fn")
        {
            auto n = lang::lex::parse_function(p);
            if (n) nodes.push_back(std::move(n));
            continue;
        }

        if (p.cur.kind == lang::lex::TokenKind::TIDENT && p.cur.name == "struct")
        {
            auto n = lang::lex::parse_structure(p);
            if (n) nodes.push_back(std::move(n));
            continue;
        }

        if (p.cur.kind == lang::lex::TokenKind::TIDENT && p.cur.name == "extern")
        {
            auto n = lang::lex::parse_extern_lib(p);
            if (n) nodes.push_back(std::move(n));
            continue;
        }

        p.error_at(p.cur, std::format("Unexpected top-level token '{}'", p.cur.name));
        p.advance();
    }

    return nodes;
}

int main(int argc, char** argv)
{
    const char*   path = argc > 1 ? argv[1] : "src.jl";
    std::ifstream file(path);
    if (!file)
    {
        std::cerr << "Cannot open file: " << path << "\n";
        return 1;
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    std::string src = ss.str();

    lang::lex::Lexer  lexer{src, 0};
    auto              first_token = lang::lex::token_next(lexer);
    lang::lex::Parser parser{lexer, first_token};
    auto              nodes = parse_program(parser);

    if (nodes.empty())
    {
        std::cerr << "Nothing parsed.\n";
        return 1;
    }

    lang::Resolver resolver;
    for (auto& node : nodes)
        node = lang::resolve_node(resolver, std::move(node));

    lang::DiagnosticEngine diag;
    lang::Sema             sema{resolver, diag};
    for (const auto& node : nodes)
        if (node) sema.check(*node);

    diag.flush(src);
    if (diag.has_errors)
    {
        std::cerr << "Aborting due to semantic errors.\n";
        return 1;
    }

#ifdef DUMP_AST
    for (const auto& node : nodes)
        if (node) lang::print_node(*node);
#endif

    lang::Codegen cg("src");

    for (const auto& node : nodes)
        if (node) cg.emit(*node);

    // cg.dump();

    std::string obj_path = "/tmp/out.o";
    if (!cg.emit_object(obj_path))
    {
        std::cerr << "Code generation failed.\n";
        return 1;
    }

    std::string out_path = argc > 2 ? argv[2] : "a.out";
    std::string link_cmd = std::format("cc {} -o {} -lm", obj_path, out_path);
    int         rc       = std::system(link_cmd.c_str());
    if (rc != 0)
    {
        std::cerr << "Linking failed (cc returned " << rc << ")\n";
        return 1;
    }

    std::cout << "Written: " << out_path << "\n";
    return 0;
}
