#ifndef C4_TYPE_H
#define C4_TYPE_H

#endif // C4_TYPE_H
#pragma once

#include <memory>
#include <string>
#include <vector>
#include "symbol_internalizer.h"

#include "llvm/IR/Type.h"
#include "llvm/IR/IRBuilder.h"

class C4Type {
public:
    virtual ~C4Type() = default;

    virtual bool equals(const C4Type& other) const = 0;

    virtual bool isScalar() const { return false; }     // int, char, pointer
    virtual bool isArithmetic() const { return false; } // int, char
    virtual bool isVoid() const { return false; }
    virtual bool isPointer() const { return false; }
    virtual bool isFunction() const { return false; }
    virtual bool isStruct() const { return false; }
    virtual llvm::Type* getLLVMType(llvm::IRBuilder<> & builder) const = 0;
};

//Primitives
class C4VoidType : public C4Type {
public:
    bool equals(const C4Type& other) const override {
        return dynamic_cast<const C4VoidType*>(&other) != nullptr;
    }
   bool isVoid() const override { return true; }
   llvm::Type* getLLVMType(llvm::IRBuilder<> & builder) const override {
      return builder.getVoidTy();
    }
};

class C4IntType : public C4Type {
public:
    bool equals(const C4Type& other) const override {
        return dynamic_cast<const C4IntType*>(&other) != nullptr;
    }
    bool isScalar() const override { return true; }
    bool isArithmetic() const override { return true; }
    llvm::Type* getLLVMType(llvm::IRBuilder<> & builder) const override {
        return builder.getInt32Ty();
      }
};

class C4CharType : public C4Type {
public:
    bool equals(const C4Type& other) const override {
        return dynamic_cast<const C4CharType*>(&other) != nullptr;
    }
    bool isScalar() const override { return true; }
    bool isArithmetic() const override { return true; }
    llvm::Type* getLLVMType(llvm::IRBuilder<> & builder) const override {
        return builder.getInt8Ty();
      }
};

// Composite Types
class C4PointerType : public C4Type {
    std::shared_ptr<C4Type> base;
public:
    explicit C4PointerType(std::shared_ptr<C4Type> b) : base(std::move(b)) {}
    std::shared_ptr<C4Type> getBase() const { return base; }

    bool equals(const C4Type& other) const override {
        if (auto* p = dynamic_cast<const C4PointerType*>(&other)) {
            return base->equals(*p->base);
        }
        return false;
    }
    bool isScalar() const override { return true; }
    bool isPointer() const override { return true; }
    llvm::Type* getLLVMType(llvm::IRBuilder<> & builder) const override {
        return llvm::PointerType::getUnqual(builder.getContext());
    }
};

class C4StructType : public C4Type {
public:
    struct Field {
        Symbol name;
        std::shared_ptr<C4Type> type;
    };

private:
    Symbol tag;
    std::vector<Field> members;
public:
    explicit C4StructType(Symbol t, std::vector<Field> m) : tag(t), members(std::move(m)) {}

    Symbol getTag() const { return tag; }
    const std::vector<Field>& getMembers() const { return members; }

    bool equals(const C4Type& other) const override {
        if (auto* s = dynamic_cast<const C4StructType*>(&other)) {
            return tag == s->tag; 
        }
        return false;
    }

    bool isStruct() const override { return true; }

    llvm::Type* getLLVMType(llvm::IRBuilder<>& builder) const override {
        llvm::LLVMContext& ctx = builder.getContext();
        
        // Check if the struct already exists by name
        if (llvm::StructType* existing = llvm::StructType::getTypeByName(ctx, *tag)) {
            return existing;
        }

        // Create Opaque Struct to allow recursive references
        llvm::StructType* res = llvm::StructType::create(ctx, *tag);

        // Convert member types
        std::vector<llvm::Type*> llvmMemberTypes;
        llvmMemberTypes.reserve(members.size());
        
        for (const auto& field : members) {
            llvmMemberTypes.push_back(field.type->getLLVMType(builder));
        }

        if (res->isOpaque()) {
            res->setBody(llvmMemberTypes);
        }
        
        return res;
    }
};

class C4FunctionType : public C4Type {
    std::shared_ptr<C4Type> returnType;
    std::vector<std::shared_ptr<C4Type>> params;
public:
    C4FunctionType(std::shared_ptr<C4Type> ret, std::vector<std::shared_ptr<C4Type>> p)
        : returnType(std::move(ret)), params(std::move(p)) {}

    std::shared_ptr<C4Type> getReturnType() const { return returnType; }
    const std::vector<std::shared_ptr<C4Type>>& getParams() const { return params; }

    bool equals(const C4Type& other) const override {
        if (auto* f = dynamic_cast<const C4FunctionType*>(&other)) {
            if (!returnType->equals(*f->returnType)) return false;
            if (params.size() != f->params.size()) return false;
            for (size_t i = 0; i < params.size(); ++i) {
                if (!params[i]->equals(*f->params[i])) return false;
            }
            return true;
        }
        return false;
    }
    bool isFunction() const override { return true; }
    llvm::Type* getLLVMType(llvm::IRBuilder<> & builder) const override {
        std::vector<llvm::Type*> paramTypes;
        paramTypes.reserve(params.size());
        for (auto& p : params) {
          paramTypes.push_back(p->getLLVMType(builder));
        }
        return llvm::FunctionType::get(returnType->getLLVMType(builder),paramTypes, false);
      }
};
