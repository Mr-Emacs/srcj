#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

#include "../frontend/ast.h"
#include "../middle/types.h"

#include <stdexcept>
#include <string>
#include <unordered_map>

namespace lang
{
    inline llvm::Type* to_llvm_type(TypeKind k, llvm::LLVMContext& ctx)
    {
        switch (k)
        {
            case TypeKind::Void:
                return llvm::Type::getVoidTy(ctx);
            case TypeKind::Boolean:
                return llvm::Type::getInt1Ty(ctx);
            case TypeKind::Number:
                return llvm::Type::getInt64Ty(ctx);
            case TypeKind::String:
                return llvm::PointerType::getUnqual(ctx);
            default:
                return llvm::Type::getInt64Ty(ctx);
        }
    }

    struct Codegen
    {
        llvm::LLVMContext ctx;
        llvm::Module      mod;
        llvm::IRBuilder<> builder;

        struct Slot
        {
            llvm::Value* alloca;
            llvm::Type*  type;
        };
        std::unordered_map<std::string, Slot> locals;

        std::unordered_map<std::string, llvm::Function*> functions;

        struct StructInfo
        {
            llvm::StructType*                         ty;
            std::unordered_map<std::string, unsigned> field_index;
        };
        std::unordered_map<std::string, StructInfo> structs;

        explicit Codegen(const std::string& module_name)
            : mod(module_name, ctx),
              builder(ctx)
        {

            llvm::InitializeNativeTarget();
            llvm::InitializeNativeTargetAsmPrinter();
            llvm::InitializeNativeTargetAsmParser();
        }

        llvm::Value* emit(const Node& node)
        {
            return std::visit([&](const auto& d) -> llvm::Value*
                              {
                    using T = std::decay_t<decltype(d)>;


                    if constexpr (std::is_same_v<T, NumberLiteral>)
                    return llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx),
                            d.value, false);

                    if constexpr (std::is_same_v<T, BooleanLiteral>)
                    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(ctx),
                            d.value ? 1 : 0);

                    if constexpr (std::is_same_v<T, StringLiteral>)
                    return builder.CreateGlobalString(d.value, ".str");


                    if constexpr (std::is_same_v<T, Identifier>)
                    {
                    auto it = locals.find(d.value);
                    if (it == locals.end())
                        throw std::runtime_error("Undefined variable: " + d.value);
                    return builder.CreateLoad(it->second.type,
                            it->second.alloca,
                            d.value);
                    }


                    if constexpr (std::is_same_v<T, AssignNode>)
                    {
                        llvm::Value* rhs = d.value ? emit(*d.value) : nullptr;
                        if (!rhs) return nullptr;

                        auto it = locals.find(d.name);
                        if (it == locals.end())
                        {


                            auto* fn    = builder.GetInsertBlock()->getParent();
                            auto& entry = fn->getEntryBlock();
                            llvm::IRBuilder<> entry_b(&entry, entry.begin());
                            auto* slot = entry_b.CreateAlloca(rhs->getType(),
                                    nullptr, d.name);
                            locals[d.name] = {slot, rhs->getType()};
                            builder.CreateStore(rhs, slot);
                        }
                        else
                        {
                            builder.CreateStore(rhs, it->second.alloca);
                        }
                        return nullptr;
                    }


                    if constexpr (std::is_same_v<T, BinOpNode>)
                    {
                        llvm::Value* lhs = d.lhs ? emit(*d.lhs) : nullptr;
                        llvm::Value* rhs = d.rhs ? emit(*d.rhs) : nullptr;
                        if (!lhs || !rhs) throw std::runtime_error("BinOp missing operand");

                        switch (d.kind)
                        {
                            case OpKind::ADD:  return builder.CreateAdd(lhs, rhs, "add");
                            case OpKind::SUB:  return builder.CreateSub(lhs, rhs, "sub");
                            case OpKind::MULT: return builder.CreateMul(lhs, rhs, "mul");
                            case OpKind::DIV:  return builder.CreateSDiv(lhs, rhs, "div");
                        }
                        return nullptr;
                    }

                    if constexpr (std::is_same_v<T, BlockNode>)
                    {
                        llvm::Value* last = nullptr;
                        for (const auto& stmt : d.statements)
                            if (stmt) last = emit(*stmt);
                        return last;
                    }

                    if constexpr (std::is_same_v<T, FunctionDeclNode>)
                    {
                        bool is_main = (d.name == "main");

                        llvm::Type* ret_ty = is_main
                            ? llvm::Type::getInt32Ty(ctx)
                            : llvm::Type::getVoidTy(ctx);

                        auto* fn_ty = llvm::FunctionType::get(ret_ty, {}, false);
                        auto* fn    = llvm::Function::Create(fn_ty,
                                llvm::Function::ExternalLinkage,
                                d.name, mod);
                        functions[d.name] = fn;


                        auto* prev_bb   = builder.GetInsertBlock();
                        auto  prev_vars = locals;

                        auto* entry = llvm::BasicBlock::Create(ctx, "entry", fn);
                        builder.SetInsertPoint(entry);
                        locals.clear();

                        if (d.body) emit(*d.body);


                        if (!builder.GetInsertBlock()->getTerminator())
                        {
                            if (is_main)
                                builder.CreateRet(
                                        llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0));
                            else
                                builder.CreateRetVoid();
                        }

                        llvm::verifyFunction(*fn, &llvm::errs());


                        locals = std::move(prev_vars);
                        if (prev_bb) builder.SetInsertPoint(prev_bb);

                        return fn;
                    }


                    if constexpr (std::is_same_v<T, FunctionCallNode>)
                    {
                        auto fit = functions.find(d.name);
                        if (fit == functions.end())
                            throw std::runtime_error("Call to undefined function: " + d.name);

                        std::vector<llvm::Value*> args;
                        args.reserve(d.args.size());
                        for (const auto& arg : d.args)
                            if (arg) args.push_back(emit(*arg));

                        return builder.CreateCall(fit->second, args);
                    }


                    if constexpr (std::is_same_v<T, IntrinsicCallNode>)
                        return emit_intrinsic(d);


                    if constexpr (std::is_same_v<T, StructNode>)
                    {
                        std::vector<llvm::Type*> field_tys;
                        StructInfo info;

                        for (unsigned i = 0; i < d.fields.size(); ++i)
                        {
                            const auto& f = d.fields[i];
                            info.field_index[f.name] = i;

                            llvm::Type* ty;
                            if (f.type_name == "number")       ty = llvm::Type::getInt64Ty(ctx);
                            else if (f.type_name == "bool")    ty = llvm::Type::getInt1Ty(ctx);
                            else if (f.type_name == "string" ||
                                    f.is_pointer)             ty = llvm::PointerType::getUnqual(ctx);
                            else if (structs.count(f.type_name))
                                ty = llvm::PointerType::getUnqual(ctx);
                            else                               ty = llvm::Type::getInt64Ty(ctx);

                            field_tys.push_back(ty);
                        }

                        info.ty = llvm::StructType::create(ctx, field_tys, d.name);
                        structs[d.name] = std::move(info);
                        return nullptr;
                    }


                    if constexpr (std::is_same_v<T, ExternNode>)
                    {


                        if (!mod.getFunction(d.name))
                        {
                            auto* ty = llvm::FunctionType::get(
                                    llvm::Type::getVoidTy(ctx), true);
                            llvm::Function::Create(ty,
                                    llvm::Function::ExternalLinkage, d.name, mod);
                        }
                        return nullptr;
                    }

                    if constexpr (std::is_same_v<T, LoadModuleNode>)
                        return nullptr;

                    return nullptr; },
                              node.data);
        }

        llvm::Value* emit_intrinsic(const IntrinsicCallNode& d)
        {
            auto arg = [&](std::size_t i) -> llvm::Value*
            {
                return (i < d.args.size() && d.args[i]) ? emit(*d.args[i]) : nullptr;
            };

            switch (d.kind)
            {
                case IntrinsicKind::PRINT:
                {
                    llvm::Value* v = arg(0);
                    if (!v) return nullptr;

                    auto printf_fn = get_or_declare_printf();

                    const char* fmt_str;
                    if (v->getType()->isPointerTy())
                        fmt_str = "%s\n";
                    else if (v->getType()->isIntegerTy(1))
                        fmt_str = "%d\n";
                    else
                        fmt_str = "%lld\n";

                    auto* fmt = builder.CreateGlobalString(fmt_str, ".fmt");
                    return builder.CreateCall(printf_fn, {fmt, v});
                }

                case IntrinsicKind::ADD:
                    return builder.CreateAdd(arg(0), arg(1), "add");
                case IntrinsicKind::SUB:
                    return builder.CreateSub(arg(0), arg(1), "sub");
                case IntrinsicKind::MULT:
                    return builder.CreateMul(arg(0), arg(1), "mul");
                case IntrinsicKind::DIV:
                    return builder.CreateSDiv(arg(0), arg(1), "div");

                default:
                    throw std::runtime_error("Unknown intrinsic kind");
            }
        }

        bool emit_object(const std::string& path)
        {
            auto triple = llvm::sys::getDefaultTargetTriple();

            std::string err;
            auto*       target = llvm::TargetRegistry::lookupTarget(triple, err);
            if (!target)
            {
                llvm::errs() << "Target lookup failed: " << err << "\n";
                return false;
            }

            llvm::TargetOptions opts;
            auto*               tm = target->createTargetMachine(
                triple, "generic", "", opts, llvm::Reloc::PIC_);

            mod.setDataLayout(tm->createDataLayout());
            mod.setTargetTriple(triple);

            std::error_code      ec;
            llvm::raw_fd_ostream dest(path, ec, llvm::sys::fs::OF_None);
            if (ec)
            {
                llvm::errs() << "Cannot open output file: " << ec.message() << "\n";
                return false;
            }

            llvm::legacy::PassManager pm;
            if (tm->addPassesToEmitFile(pm, dest, nullptr, llvm::CodeGenFileType::ObjectFile))
            {
                llvm::errs() << "Target cannot emit object file\n";
                return false;
            }

            pm.run(mod);
            dest.flush();
            return true;
        }
        void dump() { mod.print(llvm::outs(), nullptr); }
        void write_ir(const std::string& path)
        {
            std::error_code      ec;
            llvm::raw_fd_ostream out(path, ec);
            mod.print(out, nullptr);
        }

      private:
        llvm::FunctionCallee get_or_declare_printf()
        {
            auto* i8p = llvm::PointerType::getUnqual(ctx);
            auto* ty  = llvm::FunctionType::get(
                llvm::Type::getInt32Ty(ctx), {i8p}, true);
            return mod.getOrInsertFunction("printf", ty);
        }
    };
}
