#include <format>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <vector>
#include "clang-c/CXSourceLocation.h"
#include "clang-c/Index.h"

struct ParameterInfo
{
    std::string name;
    std::string lang_type;
    std::string c_spelling;
};

struct FunctionBinding
{
    std::string name;
    std::vector<ParameterInfo> params;
    std::string return_type;
    std::string return_spellin;
    bool is_variadic = false;
};

struct FieldInfo
{
    std::string name;
    std::string lang_type;
    std::string c_spelling;
};

struct StructBinding
{
    std::string name;
    std::vector<FieldInfo> fields;
    bool is_opaque = false;
};

struct EnumVariant
{
    std::string name;
    long long   value = 0;
};

struct EnumBinding
{
    std::string name;
    std::vector<EnumVariant> variants;
};

struct Bindings
{
    std::string lib_name;
    std::string lib_path;

    std::vector<FunctionBinding> functions;
    std::vector<StructBinding> structs;
    std::vector<EnumBinding> enums;
std::unordered_set<std::string> seen_structs;
    std::unordered_set<std::string> seen_enums;
    std::unordered_set<std::string> seen_functions;
};
    
std::string toString(CXString cx_str) 
{
    auto c = clang_getCString(cx_str);
    std::string s = c ? c : "";
    clang_disposeString(cx_str);
    return s;
}

static std::string spelling(CXType t)
{
    return toString(clang_getTypeSpelling(t));
}

static std::string map_type(CXType t);
static std::string map_type_imp(CXType t)
{
    auto unwrap = clang_getCanonicalType(t);
    switch(t.kind) {
        case CXType_Bool: return "bool";
        case CXType_Char_S: 
        case CXType_Char_U: return "char";
        case CXType_SChar: 
        case CXType_UChar: 
        case CXType_Short: 
        case CXType_UShort: 
        case CXType_Int: 
        case CXType_UInt: 
        case CXType_Long: 
        case CXType_ULong: 
        case CXType_LongLong: 
        case CXType_ULongLong: 
        case CXType_Float: 
        case CXType_Double:
        case CXType_LongDouble: return "number";
        case CXType_Void: return "void";
        
        case CXType_Pointer:
        {
            auto pointee = clang_getPointeeType(t);
            if (pointee.kind == CXType_Char_S || pointee.kind == CXType_Char_U)
                return "string";

            if (pointee.kind == CXType_Void)
                return "*void";

            if (pointee.kind == CXType_FunctionProto)
                return "*fn";
            return "*" + map_type(pointee);
        } 
    
        case CXType_ConstantArray:
        {
            auto n = clang_getArraySize(t);
            auto elem = clang_getArrayElementType(t);
            return "[" + std::to_string(n) + "]" + map_type(elem);
        }

        case CXType_IncompleteArray:
        {
            auto elem = clang_getArrayElementType(t);
            return "[]" + map_type(elem);
        }

        case CXType_Typedef:
        {
            std::string name = spelling(t);
            std::string canon_mapped = map_type(unwrap);
            if (unwrap.kind == CXType_Record || unwrap.kind == CXType_Enum)
                return name;
            return canon_mapped;
        }
        case CXType_Record:
        case CXType_Enum: 
        {
            return toString(clang_getTypeSpelling(t));
        }

        case CXType_Elaborated: 
        {
            return map_type(clang_Type_getNamedType(t));
        }
        default:
        return std::format("/* unknown: {} */", spelling(t));
    };
}

static std::string map_type(CXType t) {
    return map_type_imp(t);
}

static CXChildVisitResult field_vistor(CXCursor cur, CXCursor _, CXClientData data)
{
    auto *fields = static_cast<std::vector<FieldInfo> *>(data);
    if (clang_getCursorKind(cur) == CXCursor_FieldDecl)
    {
        auto field_type = clang_getCursorType(cur);
        auto field_name = toString(clang_getCursorSpelling(cur));
        if (field_name.empty()) field_name = "_pad";
        fields->push_back({field_name, map_type(field_type), spelling(field_type)});
    }
    return CXChildVisit_Continue;
}

static CXChildVisitResult enum_vistor(CXCursor cur, CXCursor _, CXClientData data)
{
    auto *variants = static_cast<std::vector<EnumVariant> *>(data);
    if (clang_getCursorKind(cur) == CXCursor_EnumConstantDecl)
    {
        auto name = toString(clang_getCursorSpelling(cur));
        auto value = clang_getEnumConstantDeclValue(cur);
        variants->push_back({name, value});
    }
    return CXChildVisit_Continue;
}

static CXChildVisitResult top_visitor(CXCursor cur, CXCursor _, CXClientData data)
{
    auto *bindings = static_cast<Bindings *>(data);
    auto loc = clang_getCursorLocation(cur);
    if (clang_Location_isInSystemHeader(loc)) return CXChildVisit_Continue;

    auto kind = clang_getCursorKind(cur);

    if (kind == CXCursor_FunctionDecl)
    {
        auto name = toString(clang_getCursorSpelling(cur));
        if (bindings->seen_functions.count(name)) return CXChildVisit_Continue;
        bindings->seen_functions.insert(name);

        auto func_type = clang_getCursorType(cur);
        auto ret_type = clang_getResultType(func_type);

        FunctionBinding fn;
        fn.name = name;
        fn.return_type = map_type(ret_type);
        fn.return_spellin = spelling(ret_type);
        fn.is_variadic = clang_isFunctionTypeVariadic(func_type);

        auto num_args = clang_Cursor_getNumArguments(cur);
        for (int i = 0; i < num_args; ++i) {
            auto param = clang_Cursor_getArgument(cur, i);
            auto param_type = clang_getCursorType(param);

            auto param_name = toString(clang_getCursorSpelling(param));
            if (param_name.empty()) param_name = "arg" + std::to_string(i);
            fn.params.push_back({param_name, map_type(param_type), spelling(param_type)});

        }
        bindings->functions.push_back(std::move(fn));
        return CXChildVisit_Continue;
    }

    if (kind == CXCursor_StructDecl || kind == CXCursor_UnionDecl)
    {
        auto name = toString(clang_getCursorSpelling(cur));
        if (name.empty()) return CXChildVisit_Continue;
        if (bindings->seen_structs.count(name)) return CXChildVisit_Continue;
        bindings->seen_structs.insert(name);

        StructBinding sb;
        sb.name = name;
        bool has_definition = clang_isCursorDefinition(cur);
        if (!has_definition) sb.is_opaque = true;
        else clang_visitChildren(cur, field_vistor, &sb.fields);

        bindings->structs.push_back(std::move(sb));
        return CXChildVisit_Continue;
    }


    if (kind == CXCursor_EnumDecl)
    {
        auto name = toString(clang_getCursorSpelling(cur));
        if (name.empty()) return CXChildVisit_Continue;
        if (bindings->seen_enums.count(name)) return CXChildVisit_Continue;
        bindings->seen_enums.insert(name);

        EnumBinding eb;
        eb.name = name;
        clang_visitChildren(cur, enum_vistor, &eb.variants);
        bindings->enums.push_back(std::move(eb));
        return CXChildVisit_Continue;
    }

    if (kind == CXCursor_TypedefDecl)
    {
        clang_visitChildren(cur, top_visitor, data);
        return CXChildVisit_Continue;
    }

    return CXChildVisit_Continue;
}

void printCursor(CXCursor cursor, int level, std::ostream &out)
{
    for (int i = 0; i < level; ++i) out << "  ";
    out << toString(clang_getCursorKindSpelling(clang_getCursorKind(cursor)))
        << ": " << toString(clang_getCursorSpelling(cursor)) << "\n";

    struct Data { std::ostream *out; int next_level; };
    Data d{ &out, level + 1 };

    clang_visitChildren(
        cursor,
        [](CXCursor c, CXCursor /*parent*/, CXClientData client_data) {
            auto *pd = static_cast<Data*>(client_data);
            printCursor(c, pd->next_level, *pd->out);
            return CXChildVisit_Continue;
        },
        &d);
}

static void emit_bindings(const Bindings &b)
{
    std::cout << std::format("extern \"{}\" path=\"{}\"\n", b.lib_name,  b.lib_path);
    if (!b.enums.empty())
    {
        std::cout << "// Enums\n" ;
        for (const auto &e : b.enums)
        {
            std::cout << "enum " << e.name << " {\n";
            for (std::size_t i = 0; i < e.variants.size(); i++)
            {
                const auto &v = e.variants[i];
                std::cout << "    " << v.name << " = " << v.value;
                if (i + 1 < e.variants.size()) std::cout << ",";
                std::cout << "\n";
            }
            std::cout << "}\n\n";
        }

    }

    if (!b.structs.empty())
    {
        std::cout << "// Structure\n" ;
        for (const auto &s : b.structs)
        {
            if (s.is_opaque)  {
                std::cout << std::format("struct {}; // opaque\n\n",  s.name) ;
                continue;
            }

            std::cout << "struct " << s.name << " {\n";
            for (const auto &f : s.fields) {
                std::cout << std::format("    {}: {},", f.name, f.lang_type);
                if (f.lang_type.find("unknown") != std::string::npos)
                    std::cout << " // C: " << f.c_spelling;
                std::cout << "\n";
            }
            std::cout << "}\n\n";
        }

    }

    if (!b.functions.empty())
    {
        std::cout << "// Functions \n" ;
        for (const auto &f : b.functions)
        {

            std::cout << std::format("fn {} (", f.name);

            for (std::size_t i = 0; i < f.params.size(); ++i) {
                if (i > 0) std::cout << ", ";
                const auto &p = f.params[i];
                std::cout << std::format("{}: {}", p.name, p.lang_type);
                if (p.lang_type.find("unknown") != std::string::npos) {
                    std::cout << std::format(" /* C: {} */" , p.c_spelling);
                }
            }
            if (f.is_variadic)  {
                if (!f.params.empty()) std::cout << ", ";
                std::cout << "..";
            }

            std::cout << ") -> " << f.return_type << ";\n";
        }

    }

}

int main() 
{
    auto idx = clang_createIndex(1, 1);
    auto path = "raylib.h";

    static const char *args[] = {
        "-I",
        "/usr/lib/gcc/x86_64-unknown-linux-gnu/14.2/include",
    };

    auto tu = clang_parseTranslationUnit(
        idx, path, args, 2, nullptr, 0, CXTranslationUnit_None);

    if (tu == nullptr) {
        std::cerr << "Failed to parse file " << path << "\n";
        clang_disposeIndex(idx);
        return 1;
    }

    auto root = clang_getTranslationUnitCursor(tu);

    Bindings bindings;
    bindings.lib_name = "";
    bindings.lib_path = path;

    clang_visitChildren(root, top_visitor, &bindings);

    emit_bindings(bindings);

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(idx);
    return 0;
}
