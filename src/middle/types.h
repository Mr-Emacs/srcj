#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace lang
{
    enum class TypeKind : uint8_t
    {
        Unknown,
        Void,
        Boolean,
        Character,
        Number,
        String,
        Function,
        Count,
    };

    struct FunctionType
    {
        std::vector<TypeKind> param_types;
        TypeKind              return_type;
    };

    struct Type
    {
        TypeKind kind = TypeKind::Unknown;

        bool operator==(const Type& o) const { return kind == o.kind; }
        bool operator!=(const Type& o) const { return !(*this == o); }

        static Type unknown() { return {TypeKind::Unknown}; }
        static Type void_() { return {TypeKind::Void}; }
        static Type bool_() { return {TypeKind::Boolean}; }
        static Type char_() { return {TypeKind::Character}; }
        static Type number_() { return {TypeKind::Number}; }
        static Type string_() { return {TypeKind::String}; }
    };

    inline std::string type_name(TypeKind k)
    {
        static_assert(static_cast<std::size_t>(TypeKind::Count) == 7,
                      "TypeKind added — update type_name()");
        switch (k)
        {
            case TypeKind::Unknown:
                return "unknown";
            case TypeKind::Void:
                return "void";
            case TypeKind::Boolean:
                return "bool";
            case TypeKind::Character:
                return "char";
            case TypeKind::Number:
                return "number";
            case TypeKind::String:
                return "string";
            case TypeKind::Function:
                return "fn";
            case TypeKind::Count:
                return "?";
        }
        return "?";
    }
}
