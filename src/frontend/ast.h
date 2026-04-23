#pragma once
#include <memory>
#include <vector>
#include <string>
#include <variant>
#include "common.h"

namespace lang
{
    enum class OpKind : uint8_t
    {
        ADD,
        SUB,
        MULT,
        DIV,
    };

    struct NumberLiteral
    {
        std::size_t value;
    };
    struct BooleanLiteral
    {
        bool value;
    };
    struct StringLiteral
    {
        std::string value;
    };
    struct Identifier
    {
        std::string value;
    };

    struct BinOpNode
    {
        OpKind                kind;
        std::unique_ptr<Node> lhs;
        std::unique_ptr<Node> rhs;
    };

    struct AssignNode
    {
        std::string           name;
        std::unique_ptr<Node> value;
    };

    struct BlockNode
    {
        std::vector<std::unique_ptr<Node>> statements;
    };

    struct FunctionDeclNode
    {
        std::string           name;
        std::unique_ptr<Node> body;
    };

    struct FunctionCallNode
    {
        std::string                        name;
        std::vector<std::unique_ptr<Node>> args;
    };

    struct FieldNode
    {
        std::string name;
        std::string type_name;
        bool        is_pointer = false;
    };

    struct IntrinsicCallNode
    {
        IntrinsicKind                      kind;
        std::vector<std::unique_ptr<Node>> args;
    };

    struct LoadModuleNode
    {
        std::string name;
    };

    struct StructNode
    {
        std::string            name;
        std::vector<FieldNode> fields;
    };

    struct ExternNode
    {
        std::string name;
        std::string path;
    };

    using NodeData = std::variant<
        NumberLiteral,
        BooleanLiteral,
        StringLiteral,
        Identifier,
        BinOpNode,
        AssignNode,
        BlockNode,
        FunctionDeclNode,
        FunctionCallNode,
        LoadModuleNode,
        StructNode,
        ExternNode,
        IntrinsicCallNode>;

    struct Node
    {
        NodeKind kind;
        NodeData data;
    };
}
