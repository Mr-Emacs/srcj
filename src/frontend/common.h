#pragma once

namespace lang
{
    struct Node;

    enum class NodeKind : uint8_t
    {
        ASSIGN,
        BINOP,
        FUNCTIONDECL,
        FUNCTIONCALL,
        NUMBER,
        BOOLEAN,
        STRING,
        IDENT,
        BLOCK,
        INTRINSICCALL,
        COUNT,
    };

    enum class IntrinsicKind : uint8_t
    {
        PRINT,
        ADD,
        SUB,
        COUNT,
    };

    struct Intrinsic
    {
        std::string   name;
        IntrinsicKind kind;
    };
};
