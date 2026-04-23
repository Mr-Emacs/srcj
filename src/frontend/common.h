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
        LOADMODULE,
        STRUCT,
        EXTERN,
        COUNT,
    };

    enum class IntrinsicKind : uint8_t
    {
        PRINT,
        ADD,
        SUB,
        MULT,
        DIV,
        COUNT,
    };

    struct Intrinsic
    {
        std::string   name;
        IntrinsicKind kind;
    };
};
