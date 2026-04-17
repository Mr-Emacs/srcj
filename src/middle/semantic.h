#pragma once
#include "ast.h"
#include "resolver.h"
#include "types.h"
#include "diagnostic.h"

namespace lang
{
    struct Sema
    {
        Resolver&                             resolver;
        DiagnosticEngine&                     diag;
        std::unordered_map<std::string, Type> type_env;

        Type check(const Node& node)
        {
            return std::visit([&](const auto& data) -> Type
                              {
                using T = std::decay_t<decltype(data)>;

                if constexpr (std::is_same_v<T, NumberLiteral>)
                    return Type::number_();

                if constexpr (std::is_same_v<T, BooleanLiteral>)
                    return Type::bool_();

                if constexpr (std::is_same_v<T, StringLiteral>)
                    return Type::string_();

if constexpr (std::is_same_v<T, Identifier>)
{
    auto it = type_env.find(data.value);
    if (it == type_env.end()) {
        diag.error(std::format("Use of undeclared identifier '{}'", data.value));
        return Type::unknown();
    }
    return it->second;
}

                if constexpr (std::is_same_v<T, AssignNode>)
                {
                    Type rhs = data.value ? check(*data.value) : Type::unknown();
                    Symbol* sym = resolver.lookup(data.name);
                    if (sym)
                    {
                        if (sym->type != Type::unknown() && sym->type != rhs)
                            diag.error(std::format(
                                "Type mismatch: '{}' is '{}' but assigned '{}'",
                                data.name,
                                type_name(sym->type.kind),
                                type_name(rhs.kind)));
                        else
                            sym->type = rhs;
                        }
                        type_env[data.name] = rhs;
                    return Type::void_();
                }

                if constexpr (std::is_same_v<T, BinOpNode>)
                {
                    Type lhs_t = data.lhs ? check(*data.lhs) : Type::unknown();
                    Type rhs_t = data.rhs ? check(*data.rhs) : Type::unknown();
                    if (lhs_t != rhs_t)
                        diag.error(std::format(
                            "Binary op type mismatch: '{}' vs '{}'",
                            type_name(lhs_t.kind), type_name(rhs_t.kind)));
                    return lhs_t;
                }

                if constexpr (std::is_same_v<T, IntrinsicCallNode>)
                {
                    for (const auto& arg : data.args)
                        if (arg) check(*arg);

                    switch (data.kind)
                    {
                        case IntrinsicKind::PRINT:
                            if (data.args.size() != 1)
                                diag.error(std::format(
                                    "print() expects 1 argument, got {}", data.args.size()));
                            return Type::void_();

                        case IntrinsicKind::ADD:
                        case IntrinsicKind::SUB:
                            if (data.args.size() != 2)
                                diag.error("add/sub expect exactly 2 arguments");
                            if (data.args.size() >= 1 && data.args[0])
                            {
                                Type a = check(*data.args[0]);
                                if (a.kind != TypeKind::Number && a.kind != TypeKind::Unknown)
                                    diag.error("add/sub require int operands");
                            }
                            return Type::number_();

                        default: return Type::unknown();
                    }
                }

                if constexpr (std::is_same_v<T, FunctionCallNode>)
                {
                    Symbol* sym = resolver.lookup(data.name);
                    if (!sym)
                        diag.error(std::format("Call to undefined function '{}'", data.name));
                    for (const auto& arg : data.args)
                        if (arg) check(*arg);
                    return sym ? sym->type : Type::unknown();
                }

                if constexpr (std::is_same_v<T, FunctionDeclNode>)
                {
                    resolver.enter_scope();
                    if (data.body) check(*data.body);
                    resolver.exit_scope();
                    return Type::void_();
                }

                if constexpr (std::is_same_v<T, BlockNode>)
                {
                    for (const auto& stmt : data.statements)
                        if (stmt) check(*stmt);
                    return Type::void_();
                }

                return Type::unknown(); },
                              node.data);
        }
    };
}
