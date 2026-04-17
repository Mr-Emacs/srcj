#pragma once

#include "ast.h"
#include "resolver.h"

namespace lang
{
    std::unique_ptr<Node> resolve_node(Resolver& r, std::unique_ptr<Node> node)
    {
        if (!node) return nullptr;

        if (node->kind == NodeKind::FUNCTIONCALL)
        {
            auto&   call = std::get<FunctionCallNode>(node->data);
            Symbol* sym  = r.lookup(call.name);
            if (sym && sym->kind == SymbolKind::Intrinsic)
            {
                for (auto& arg : call.args)
                    arg = resolve_node(r, std::move(arg));
                return std::make_unique<Node>(NodeKind::INTRINSICCALL,
                                              IntrinsicCallNode{sym->intrinsic.kind, std::move(call.args)});
            }
        }

        else if (node->kind == NodeKind::BLOCK)
        {
            auto& block = std::get<BlockNode>(node->data);
            r.enter_scope();
            for (auto& stmt : block.statements)
                stmt = resolve_node(r, std::move(stmt));
            r.exit_scope();
        }
        else if (node->kind == NodeKind::FUNCTIONDECL)
        {
            auto& decl = std::get<FunctionDeclNode>(node->data);
            r.define({decl.name, SymbolKind::Function, {}, {}});
            decl.body = resolve_node(r, std::move(decl.body));
        }

        if (node->kind == NodeKind::ASSIGN)
        {
            auto& assign = std::get<AssignNode>(node->data);
            assign.value = resolve_node(r, std::move(assign.value));
            r.define({assign.name, SymbolKind::Variable, {}, {}});
        }

        return node;
    }
};
