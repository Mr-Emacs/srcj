#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <unordered_map>

namespace lang
{
    enum class SymbolKind : uint8_t
    {
        Variable,
        Function,
        Intrinsic,
    };

    // TODO: Add type info
    struct Symbol
    {
        std::string name;
        SymbolKind  kind;
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

      private:
        void register_intrinsics()
        {
            current->define({"print", SymbolKind::Intrinsic, {"print", IntrinsicKind::PRINT}});
            current->define({"add", SymbolKind::Intrinsic, {"add", IntrinsicKind::ADD}});
            current->define({"sub", SymbolKind::Intrinsic, {"sub", IntrinsicKind::SUB}});

            static_assert(static_cast<uint8_t>(IntrinsicKind::COUNT) == 3,
                          "register_intrinsics is missing an entry for a new IntrinsicKind");
        }
    };
}
