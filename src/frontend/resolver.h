#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <unordered_map>
#include "../middle/types.h"

namespace lang
{
    enum class SymbolKind : uint8_t
    {
        Variable,
        Function,
        Module,
        Struct,
        Intrinsic,
    };

    // TODO: Add type info
    struct Symbol
    {
        SymbolKind  kind;
        std::string name;
        std::string module_name;
        Type        type = Type::unknown();
        Intrinsic   intrinsic;
    };

    struct Scope
    {
        std::unordered_map<std::string, Symbol> symbols;
        std::unique_ptr<Scope>                  parent;

        explicit Scope(std::unique_ptr<Scope> parent = nullptr)
            : parent(std::move(parent))
        {
        }

        Symbol* lookup_local(const std::string& name)
        {
            auto it = symbols.find(name);
            return it != symbols.end() ? &it->second : nullptr;
        }
        Symbol* lookup(const std::string& name)
        {
            if (auto* s = lookup_local(name)) return s;
            return parent ? parent->lookup(name) : nullptr;
        }
        void define(Symbol sym)
        {
            symbols.emplace(sym.name, std::move(sym));
        }
    };

    struct Resolver
    {
        std::unique_ptr<Scope> current;
        Resolver()
            : current(std::make_unique<Scope>())
        {
            register_intrinsics();
        }
        void enter_scope()
        {
            current = std::make_unique<Scope>(std::move(current));
        }
        void exit_scope()
        {
            current = std::move(current->parent);
        }
        Symbol* lookup(const std::string& name)
        {
            return current->lookup(name);
        }
        void define(Symbol sym)
        {
            current->define(std::move(sym));
        }
        void load_module(const std::string& module_name)
        {
            current->define({
                .kind        = SymbolKind::Module,
                .name        = module_name,
                .module_name = module_name,
            });
        }

      private:
        void register_intrinsics()
        {
            current->define({
                .kind      = SymbolKind::Intrinsic,
                .name      = "print",
                .type      = Type::void_(),
                .intrinsic = {"print", IntrinsicKind::PRINT},
            });
            current->define({
                .kind      = SymbolKind::Intrinsic,
                .name      = "add",
                .type      = Type::number_(),
                .intrinsic = {"add", IntrinsicKind::ADD},
            });
            current->define({
                .kind      = SymbolKind::Intrinsic,
                .name      = "sub",
                .type      = Type::number_(),
                .intrinsic = {"sub", IntrinsicKind::SUB},
            });

            current->define({
                .kind      = SymbolKind::Intrinsic,
                .name      = "mult",
                .type      = Type::number_(),
                .intrinsic = {"mult", IntrinsicKind::MULT},
            });

            current->define({
                .kind      = SymbolKind::Intrinsic,
                .name      = "div",
                .type      = Type::number_(),
                .intrinsic = {"div", IntrinsicKind::DIV},
            });

            static_assert(static_cast<uint8_t>(IntrinsicKind::COUNT) == 5,
                          "register_intrinsics is missing an entry for a new IntrinsicKind");
        }
    };
}
