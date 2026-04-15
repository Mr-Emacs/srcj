#pragma once
#include <iostream>
#include <string>
#include "ast.h"

inline void print_node(const Node &node, int depth = 0)
{
    std::string indent(depth * 2, ' ');

    std::visit([&](const auto &data) {
        using T = std::decay_t<decltype(data)>;

        if constexpr (std::is_same_v<T, NumberLiteral>) {
            std::cout << indent << "Number(" << data.value << ")\n";
        }
        else if constexpr (std::is_same_v<T, BooleanLiteral>) {
            std::cout << indent << "Bool(" << (data.value ? "true" : "false") << ")\n";
        }
        else if constexpr (std::is_same_v<T, StringLiteral>) {
            std::cout << indent << "String(\"" << data.value << "\")\n";
        }
        else if constexpr (std::is_same_v<T, Identifier>) {
            std::cout << indent << "Ident(" << data.value << ")\n";
        }
        else if constexpr (std::is_same_v<T, AssignNode>) {
            std::cout << indent << "Assign(" << data.name << ")\n";
            if (data.value) print_node(*data.value, depth + 1);
        }
        else if constexpr (std::is_same_v<T, BinOpNode>) {
            constexpr std::string_view ops[] = { "ADD", "SUB", "MULT", "DIV" };
            std::cout << indent << "BinOp(" << ops[static_cast<int>(data.kind)] << ")\n";
            if (data.lhs) print_node(*data.lhs, depth + 1);
            if (data.rhs) print_node(*data.rhs, depth + 1);
        }
        else if constexpr (std::is_same_v<T, BlockNode>) {
            std::cout << indent << "Block\n";
            for (const auto &stmt : data.statements)
                if (stmt) print_node(*stmt, depth + 1);
        }
        else if constexpr (std::is_same_v<T, FunctionDeclNode>) {
            std::cout << indent << "FunctionDecl(" << data.name << ")\n";
            if (data.body) print_node(*data.body, depth + 1);
        }
        else if constexpr (std::is_same_v<T, IntrinsicCallNode>) {
            constexpr std::string_view names[] = { "print", "add", "sub" };
            auto i = static_cast<std::size_t>(data.kind);
            std::cout << indent << "IntrinsicCall(" << names[i] << ")\n";
            for (const auto &arg : data.args)
            if (arg) print_node(*arg, depth + 1);
        }
        else if constexpr (std::is_same_v<T, FunctionCallNode>) {
            std::cout << indent << "FunctionCall(" << data.name << ")\n";
            for (const auto &arg : data.args)
                if (arg) print_node(*arg, depth + 1);
        }
    }, node.data);
}
