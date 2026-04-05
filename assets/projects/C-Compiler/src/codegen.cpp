#include "ast.h"
#include "sccp.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

#include <filesystem>
#include <map>

using namespace llvm;

// Holds the global LLVM state during traversal
struct CodeGenEnv {
    LLVMContext ctx;
    std::unique_ptr<Module> module;
    IRBuilder<> builder;
    IRBuilder<> allocaBuilder;
    
    // Symbol Table: Maps variable names (Symbols) to their memory address (AllocaInst*)
    std::map<Symbol, Value*> varToAddress;
    std::map<Symbol, Value*> globalVars;
    std::map<Symbol, Function*> functions;
    std::map<Symbol, StructType*> structs;
    
    // Current Function being generated (useful for creating blocks)
    Function* currentFunction = nullptr;

    CodeGenEnv(const std::string& FileName)
        : builder(ctx), allocaBuilder(ctx) {
        module = std::make_unique<Module>(FileName, ctx);
    }
};
////////////////////////////////////////////////////////////////////////////////
//                             Global Elements                                //
////////////////////////////////////////////////////////////////////////////////

void ExternalDeclaration::codeGen(CodeGenEnv& cge) {

  // Function definition
  if (body) {
    Symbol funcName = spec_decl->declarator->getInnermost().getName();
    auto* funcType = (FunctionType*)c4type->getLLVMType(cge.builder);

    Function* func = cge.functions[funcName];
    if (!func) {
      func = Function::Create(
        funcType,
        Function::ExternalLinkage,
        *funcName,
        cge.module.get()
      );
      cge.functions[funcName] = func;
    }

    cge.currentFunction = func;
    BasicBlock* entryBB = BasicBlock::Create(cge.ctx, "entry", func);
    cge.builder.SetInsertPoint(entryBB);

    // Reset Symbol Table for local scope
    cge.varToAddress.clear();

    // Initialize the alloca builder to the beginning of the entry block
    // so that local variable declarations can use it even when there are no arguments
    cge.allocaBuilder.SetInsertPoint(entryBB, entryBB->begin());

    // function arguments
    Function::arg_iterator FuncArgIt = func->arg_begin();
    for (auto& argName : varNames) {
      Argument* arg = FuncArgIt;
      FuncArgIt++;
      arg->setName(*argName);

      // Reset the alloca builder
      cge.allocaBuilder.SetInsertPoint(entryBB);
      cge.allocaBuilder.SetInsertPoint(cge.allocaBuilder.GetInsertBlock(), cge.allocaBuilder.GetInsertBlock()->begin());

      // Store each argument on the stack
      Value *argVarPtr = cge.allocaBuilder.CreateAlloca(arg->getType());

      // Store the argument value
      cge.builder.CreateStore(arg, argVarPtr);
      cge.varToAddress[argName] = argVarPtr;
    }

    // Generate Body
    body->codeGen(cge);

    // Implicit Return
    // If the last block has no terminator, add one.
    if (cge.builder.GetInsertBlock()->getTerminator() == nullptr) {
      Type *CurFuncReturnType = cge.builder.getCurrentFunctionReturnType();
      if (CurFuncReturnType->isVoidTy()) {
        cge.builder.CreateRetVoid();
      } else {
        cge.builder.CreateRet(Constant::getNullValue(CurFuncReturnType));
      }
    }

    verifyFunction(*func);
  } else if (spec_decl->isFunSpecDecl()) {
    // Function declaration without body
    auto* c4FuncType = dynamic_cast<C4FunctionType*>(c4type.get());

    Type* retType = c4FuncType->getReturnType()->getLLVMType(cge.builder);
    std::vector<Type*> paramTypes;
    for (const auto& param : c4FuncType->getParams()) {
      paramTypes.push_back(param->getLLVMType(cge.builder));
    }

    FunctionType* funcType = FunctionType::get(retType, paramTypes, false);
    Symbol funcName = spec_decl->declarator->getInnermost().getName();

    Function* func = cge.functions[funcName];
    if (!func) {
      func = Function::Create(
        funcType,
        Function::ExternalLinkage,
        *funcName,
        cge.module.get()
      );
      cge.functions[funcName] = func;
    }

  } else {
    // Check if this is a struct-only declaration (no variable)
    Symbol varName = spec_decl->declarator->getInnermost().getName();
    if (!varName) {
      // Struct tag declaration without variable — just ensure the type exists
      c4type->getLLVMType(cge.builder);
      return;
    }
    // Global variable
    Type* varType = c4type->getLLVMType(cge.builder);

    auto* globalVar = new GlobalVariable(*cge.module, varType, false, GlobalValue::CommonLinkage, Constant::getNullValue(varType), *varName);
    cge.globalVars[varName] = globalVar;
  }
}

void TranslationUnit::codeGen(const std::string& FileName, bool optimize) {
  CodeGenEnv cge(FileName);

  for (const auto & decl : ext_decls) {
    decl->codeGen(cge);
  }

  verifyModule(*cge.module);

  if (optimize) {
    // Apply mem2reg pass to promote allocas to registers (SSA construction)
    legacy::FunctionPassManager FPM(cge.module.get());
    FPM.add(static_cast<Pass*>(createPromoteMemoryToRegisterPass()));
    FPM.doInitialization();
    
    for (Function& F : *cge.module) {
      if (!F.isDeclaration()) {
        FPM.run(F);
      }
    }
    FPM.doFinalization();

    // Run SCCP optimization pass
    SCCPPass sccp;
    sccp.runOnModule(*cge.module);

    // Verify module after optimization
    verifyModule(*cge.module);
  }

  std::error_code EC;
  std::string outFileName = std::filesystem::path(FileName).replace_extension(".ll").filename();
  raw_fd_ostream stream(outFileName, EC, sys::fs::OF_Text);

  cge.module->print(stream, nullptr);
  
  std::cout << "Generated IR: " << outFileName << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
//                               Declarations                                 //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//                                Expressions                                 //
////////////////////////////////////////////////////////////////////////////////
void Expr::makeCF(CodeGenEnv &cge, BasicBlock *trueBB, BasicBlock *falseBB) {
    // get the R-value of the expression
    Value* val = makeRValue(cge);

    Value* cond;
    if (val->getType()->isPointerTy()) {
      // Compare pointer with null
      cond = cge.builder.CreateICmpNE(
          val,
          ConstantPointerNull::get(cast<PointerType>(val->getType())),
          "condition"
      );
    } else {
      // Compare integer/char with 0
      cond = cge.builder.CreateICmpNE(
          val,
          Constant::getNullValue(val->getType()),
          "condition"
      );
    }

    // create the branch
    cge.builder.CreateCondBr(cond, trueBB, falseBB);
}

Value* Expr::makeRValue(CodeGenEnv& cge) {

  if (std::dynamic_pointer_cast<C4FunctionType>(exprType))
    return makeLValue(cge);

  Type* eType = exprType->getLLVMType(cge.builder);
  return cge.builder.CreateLoad(eType, makeLValue(cge));
}

// primary expressions

Value* Identifier::makeLValue(CodeGenEnv& cge) {
  if (cge.varToAddress.count(name))
    return cge.varToAddress[name];
  
  if (cge.globalVars.count(name))
    return cge.globalVars[name];
  
  return cge.functions[name];
}

Value* IntLiteral::makeRValue(CodeGenEnv& cge) {
  return cge.builder.getInt32(value);
}

Value* CharLiteral::makeRValue(CodeGenEnv& cge) {
  // text is like 'A' — the lexer already handles escape sequences
  return cge.builder.getInt8(static_cast<uint8_t>((*text)[1]));
}

Value* ::StringLiteral::makeRValue(CodeGenEnv& cge) {
  // text includes surrounding quotes — the lexer already handles escape sequences
  // Strip the surrounding quote marks
  const std::string& raw = *text;
  std::string content = raw.substr(1, raw.size() - 2);
  return cge.builder.CreateGlobalString(content);
}

Value * UnaryOp::makeRValue(CodeGenEnv& cge) {
  switch (op) {
    case TK_AND: {
      return operand->makeLValue(cge);
    }
    case TK_ASTERISK: {
      // Dereference: load the value pointed to
      // makeLValue returns the pointer value, Expr::makeRValue loads from it
      // using this->exprType (the pointee type)
      return Expr::makeRValue(cge);
    }
    case TK_PLUS: {
      // Unary plus is a no-op
      return operand->makeRValue(cge);
    }
    case TK_MINUS: {
      Value* val = operand->makeRValue(cge);
      return cge.builder.CreateNeg(val);
    }
    case TK_TILDE: {
      Value* val = operand->makeRValue(cge);
      return cge.builder.CreateNot(val);
    }
    case TK_BANG: {
      Value* val = operand->makeRValue(cge);
      Value* cmp;
      if (val->getType()->isPointerTy()) {
        cmp = cge.builder.CreateICmpEQ(val, ConstantPointerNull::get(cast<PointerType>(val->getType())));
      } else {
        cmp = cge.builder.CreateICmpEQ(val, Constant::getNullValue(val->getType()));
      }
      return cge.builder.CreateZExt(cmp, cge.builder.getInt32Ty());
    }
    case TK_PLUSPLUS: {
      // Pre-increment: ++x
      Value* addr = operand->makeLValue(cge);
      Type* valTy = operand->exprType->getLLVMType(cge.builder);
      Value* val = cge.builder.CreateLoad(valTy, addr);
      Value* result;
      if (val->getType()->isPointerTy()) {
        auto ptrType = std::dynamic_pointer_cast<C4PointerType>(operand->exprType);
        Type* elemTy = ptrType->getBase()->getLLVMType(cge.builder);
        result = cge.builder.CreateGEP(elemTy, val, cge.builder.getInt64(1));
      } else {
        result = cge.builder.CreateAdd(val, ConstantInt::get(val->getType(), 1));
      }
      cge.builder.CreateStore(result, addr);
      return result;
    }
    case TK_MINUSMINUS: {
      // Pre-decrement: --x
      Value* addr = operand->makeLValue(cge);
      Type* valTy = operand->exprType->getLLVMType(cge.builder);
      Value* val = cge.builder.CreateLoad(valTy, addr);
      Value* result;
      if (val->getType()->isPointerTy()) {
        auto ptrType = std::dynamic_pointer_cast<C4PointerType>(operand->exprType);
        Type* elemTy = ptrType->getBase()->getLLVMType(cge.builder);
        result = cge.builder.CreateGEP(elemTy, val, cge.builder.getInt64(-1));
      } else {
        result = cge.builder.CreateSub(val, ConstantInt::get(val->getType(), 1));
      }
      cge.builder.CreateStore(result, addr);
      return result;
    }
    default:
      errorloc(*this, "not implemented yet");
  }
}

void UnaryOp::makeCF(CodeGenEnv &cge, BasicBlock *trueBB, BasicBlock *falseBB) {
  switch (op) {
    case TK_BANG:
      operand->makeCF(cge, falseBB, trueBB);
      return;
    default:
      // Fall back to evaluating the expression and comparing to 0/null
      Expr::makeCF(cge, trueBB, falseBB);
      return;
  }
}

Value* UnaryOp::makeLValue(CodeGenEnv& cge) {
  switch (op) {
    case TK_AND:
      errorloc(*this, "not L-evaluable");
    case TK_ASTERISK:
      return operand->makeRValue(cge);
    default:
      errorloc(*this, "not implemented yet");
  }
}

Value* BinaryOp::makeRValue(CodeGenEnv& cge) {
  // Handle short-circuit operators specially — they need control flow
  if (op == TK_ANDAND || op == TK_PIPEPIPE) {
    BasicBlock* trueBB = BasicBlock::Create(cge.ctx, "sc.true", cge.currentFunction);
    BasicBlock* falseBB = BasicBlock::Create(cge.ctx, "sc.false", cge.currentFunction);
    BasicBlock* mergeBB = BasicBlock::Create(cge.ctx, "sc.merge", cge.currentFunction);

    makeCF(cge, trueBB, falseBB);

    cge.builder.SetInsertPoint(trueBB);
    cge.builder.CreateBr(mergeBB);

    cge.builder.SetInsertPoint(falseBB);
    cge.builder.CreateBr(mergeBB);

    cge.builder.SetInsertPoint(mergeBB);
    PHINode* phi = cge.builder.CreatePHI(cge.builder.getInt32Ty(), 2);
    phi->addIncoming(cge.builder.getInt32(1), trueBB);
    phi->addIncoming(cge.builder.getInt32(0), falseBB);
    return phi;
  }

  Value* leftVal = left->makeRValue(cge);
  Value* rightVal = right->makeRValue(cge);

  // Handle pointer arithmetic
  bool leftIsPtr = leftVal->getType()->isPointerTy();
  bool rightIsPtr = rightVal->getType()->isPointerTy();

  if (op == TK_PLUS && leftIsPtr && !rightIsPtr) {
    // pointer + integer
    auto ptrType = std::dynamic_pointer_cast<C4PointerType>(left->exprType);
    Type* elemTy = ptrType->getBase()->getLLVMType(cge.builder);
    Value* idx = cge.builder.CreateSExtOrTrunc(rightVal, cge.builder.getInt64Ty());
    return cge.builder.CreateGEP(elemTy, leftVal, idx);
  }
  if (op == TK_PLUS && !leftIsPtr && rightIsPtr) {
    // integer + pointer
    auto ptrType = std::dynamic_pointer_cast<C4PointerType>(right->exprType);
    Type* elemTy = ptrType->getBase()->getLLVMType(cge.builder);
    Value* idx = cge.builder.CreateSExtOrTrunc(leftVal, cge.builder.getInt64Ty());
    return cge.builder.CreateGEP(elemTy, rightVal, idx);
  }
  if (op == TK_MINUS && leftIsPtr && !rightIsPtr) {
    // pointer - integer
    auto ptrType = std::dynamic_pointer_cast<C4PointerType>(left->exprType);
    Type* elemTy = ptrType->getBase()->getLLVMType(cge.builder);
    Value* idx = cge.builder.CreateSExtOrTrunc(rightVal, cge.builder.getInt64Ty());
    Value* negIdx = cge.builder.CreateNeg(idx);
    return cge.builder.CreateGEP(elemTy, leftVal, negIdx);
  }
  if (op == TK_MINUS && leftIsPtr && rightIsPtr) {
    // pointer - pointer → integer (ptrdiff)
    auto ptrType = std::dynamic_pointer_cast<C4PointerType>(left->exprType);
    Type* elemTy = ptrType->getBase()->getLLVMType(cge.builder);
    Value* diff = cge.builder.CreatePtrDiff(elemTy, leftVal, rightVal);
    return cge.builder.CreateTrunc(diff, cge.builder.getInt32Ty());
  }

  switch (op) {
    case TK_ASTERISK:
      return cge.builder.CreateMul(leftVal, rightVal);
    case TK_PLUS:
      return cge.builder.CreateAdd(leftVal, rightVal);
    case TK_MINUS:
      return cge.builder.CreateSub(leftVal, rightVal);
    case TK_SLASH:
      return cge.builder.CreateSDiv(leftVal, rightVal);
    case TK_PERCENT:
      return cge.builder.CreateSRem(leftVal, rightVal);
    case TK_PIPE:
      return cge.builder.CreateOr(leftVal, rightVal);
    case TK_AND:
      return cge.builder.CreateAnd(leftVal, rightVal);
    case TK_CARET:
      return cge.builder.CreateXor(leftVal, rightVal);
    case TK_LANGLELANGLE:
      return cge.builder.CreateShl(leftVal, rightVal);
    case TK_RANGLERANGLE:
      return cge.builder.CreateAShr(leftVal, rightVal);
    case TK_LANGLE:
      if (leftIsPtr && rightIsPtr)
        return cge.builder.CreateZExt(cge.builder.CreateICmpULT(leftVal, rightVal), cge.builder.getInt32Ty());
      return cge.builder.CreateZExt(cge.builder.CreateICmpSLT(leftVal, rightVal), cge.builder.getInt32Ty());
    case TK_LANGLEEQUALS:
      if (leftIsPtr && rightIsPtr)
        return cge.builder.CreateZExt(cge.builder.CreateICmpULE(leftVal, rightVal), cge.builder.getInt32Ty());
      return cge.builder.CreateZExt(cge.builder.CreateICmpSLE(leftVal, rightVal), cge.builder.getInt32Ty());
    case TK_RANGLE:
      if (leftIsPtr && rightIsPtr)
        return cge.builder.CreateZExt(cge.builder.CreateICmpUGT(leftVal, rightVal), cge.builder.getInt32Ty());
      return cge.builder.CreateZExt(cge.builder.CreateICmpSGT(leftVal, rightVal), cge.builder.getInt32Ty());
    case TK_RANGLEEQUALS:
      if (leftIsPtr && rightIsPtr)
        return cge.builder.CreateZExt(cge.builder.CreateICmpUGE(leftVal, rightVal), cge.builder.getInt32Ty());
      return cge.builder.CreateZExt(cge.builder.CreateICmpSGE(leftVal, rightVal), cge.builder.getInt32Ty());
    case TK_EQUALSEQUALS: {
      // Handle pointer vs integer comparison (e.g., ptr == 0)
      if (leftIsPtr && !rightIsPtr)
        rightVal = cge.builder.CreateIntToPtr(rightVal, leftVal->getType());
      else if (!leftIsPtr && rightIsPtr)
        leftVal = cge.builder.CreateIntToPtr(leftVal, rightVal->getType());
      return cge.builder.CreateZExt(cge.builder.CreateICmpEQ(leftVal, rightVal), cge.builder.getInt32Ty());
    }
    case TK_BANGEQUALS: {
      if (leftIsPtr && !rightIsPtr)
        rightVal = cge.builder.CreateIntToPtr(rightVal, leftVal->getType());
      else if (!leftIsPtr && rightIsPtr)
        leftVal = cge.builder.CreateIntToPtr(leftVal, rightVal->getType());
      return cge.builder.CreateZExt(cge.builder.CreateICmpNE(leftVal, rightVal), cge.builder.getInt32Ty());
    }
    default:
      errorloc(*this, "not implemented yet");
  }
}

void BinaryOp::makeCF(CodeGenEnv& cge, BasicBlock* trueBB, BasicBlock* falseBB) {
  switch (op) {
  case TK_ANDAND: {
    // this is the block beta in the slides
    BasicBlock* checkRightBB = BasicBlock::Create(cge.ctx, "and.rhs", cge.currentFunction);

    // Generate alpha: If true, go to checkRight
    left->makeCF(cge, checkRightBB, falseBB);

    // Generate beta
    cge.builder.SetInsertPoint(checkRightBB);
    right->makeCF(cge, trueBB, falseBB);
    break;
  }
  case TK_PIPEPIPE: {
    BasicBlock* checkRightBB = BasicBlock::Create(cge.ctx, "or.rhs", cge.currentFunction);

    left->makeCF(cge, trueBB, checkRightBB);

    cge.builder.SetInsertPoint(checkRightBB);
    right->makeCF(cge, trueBB, falseBB);
    break;
  }
  case TK_PLUS:
  case TK_MINUS:
  case TK_ASTERISK:
  case TK_SLASH:
  case TK_PERCENT:
  case TK_PIPE:
  case TK_AND:
  case TK_CARET: {
    // != 0
    Expr::makeCF(cge, trueBB, falseBB);
    break;
  }
  default: {
    // relational operators
    Value* lhs = left->makeRValue(cge);
    Value* rhs = right->makeRValue(cge);
    Value* cond = nullptr;
    // Handle mixed pointer/integer comparisons
    if (lhs->getType()->isPointerTy() && !rhs->getType()->isPointerTy())
      rhs = cge.builder.CreateIntToPtr(rhs, lhs->getType());
    else if (!lhs->getType()->isPointerTy() && rhs->getType()->isPointerTy())
      lhs = cge.builder.CreateIntToPtr(lhs, rhs->getType());
    bool ptrCmp = lhs->getType()->isPointerTy() && rhs->getType()->isPointerTy();
    switch (op) {
    case TK_LANGLE: cond = ptrCmp ? cge.builder.CreateICmpULT(lhs, rhs) : cge.builder.CreateICmpSLT(lhs, rhs); break;
    case TK_LANGLEEQUALS: cond = ptrCmp ? cge.builder.CreateICmpULE(lhs, rhs) : cge.builder.CreateICmpSLE(lhs, rhs); break;
    case TK_RANGLE: cond = ptrCmp ? cge.builder.CreateICmpUGT(lhs, rhs) : cge.builder.CreateICmpSGT(lhs, rhs); break;
    case TK_RANGLEEQUALS: cond = ptrCmp ? cge.builder.CreateICmpUGE(lhs, rhs) : cge.builder.CreateICmpSGE(lhs, rhs); break;
    case TK_EQUALSEQUALS: cond = cge.builder.CreateICmpEQ(lhs, rhs); break;
    case TK_BANGEQUALS: cond = cge.builder.CreateICmpNE(lhs, rhs); break;
    default:
      errorloc(*this, "cannot be");
    }
    cge.builder.CreateCondBr(cond, trueBB, falseBB);
    break;
  }
  }
}

Value* TernaryOp::makeRValue(CodeGenEnv& cge) {

  BasicBlock* alphaTrue = BasicBlock::Create(cge.ctx, "tern-consequence", cge.currentFunction);
  BasicBlock* alphaFalse = BasicBlock::Create(cge.ctx, "tern-alternative", cge.currentFunction);
  BasicBlock* phiBB = BasicBlock::Create(cge.ctx, "tern-phi", cge.currentFunction);

  condition->makeCF(cge, alphaTrue, alphaFalse);

  cge.builder.SetInsertPoint(alphaTrue);
  Value* betaVal = consequent->makeRValue(cge);
  BasicBlock* betaFromBB = cge.builder.GetInsertBlock();
  cge.builder.CreateBr(phiBB);

  cge.builder.SetInsertPoint(alphaFalse);
  Value* gammaVal = alternative->makeRValue(cge);
  BasicBlock* gammaFromBB = cge.builder.GetInsertBlock();
  cge.builder.CreateBr(phiBB);

  cge.builder.SetInsertPoint(phiBB);
  PHINode* phi = cge.builder.CreatePHI(betaVal->getType(), 2);
  phi->addIncoming(betaVal, betaFromBB);
  phi->addIncoming(gammaVal, gammaFromBB);

  return phi;
}

void TernaryOp::makeCF(CodeGenEnv &cge, BasicBlock *trueBB, BasicBlock *falseBB) {
  BasicBlock* alphaTrue = BasicBlock::Create(cge.ctx, "tern-consequence", cge.currentFunction);
  BasicBlock* alphaFalse = BasicBlock::Create(cge.ctx, "tern-alternative", cge.currentFunction);

  condition->makeCF(cge, alphaTrue, alphaFalse);
  cge.builder.SetInsertPoint(alphaTrue);
  consequent->makeCF(cge, trueBB, falseBB);
  cge.builder.SetInsertPoint(alphaFalse);
  alternative->makeCF(cge, trueBB, falseBB);
}

Value* Assignment::makeRValue(CodeGenEnv& cge) {
  Value* rhsVal = rhs->makeRValue(cge);
  Value* lhsAddr = lhs->makeLValue(cge);

  if (op ==  TK_EQUALS) {
    // Coerce rhs to match lhs type if necessary
    Type* lhsTy = lhs->exprType->getLLVMType(cge.builder);
    if (rhsVal->getType() != lhsTy) {
      if (lhsTy->isPointerTy() && rhsVal->getType()->isIntegerTy()) {
        rhsVal = cge.builder.CreateIntToPtr(rhsVal, lhsTy);
      } else if (lhsTy->isIntegerTy() && rhsVal->getType()->isIntegerTy()) {
        rhsVal = cge.builder.CreateIntCast(rhsVal, lhsTy, true);
      }
    }
    cge.builder.CreateStore(rhsVal, lhsAddr);
    return rhsVal;
  }

  // Load lhs value using its own type, not rhs type
  Type* lhsTy = lhs->exprType->getLLVMType(cge.builder);
  Value* lhsVal = cge.builder.CreateLoad(lhsTy, lhsAddr);
  Value* res = nullptr;

  switch (op) {
    case TK_PLUSEQUALS:
      res = cge.builder.CreateAdd(lhsVal, rhsVal);
      break;
    case TK_MINUSEQUALS:
      res = cge.builder.CreateSub(lhsVal, rhsVal);
      break;
    case TK_ASTERISKEQUALS:
      res = cge.builder.CreateMul(lhsVal, rhsVal);
      break;
    case TK_SLASHEQUALS:
      res = cge.builder.CreateSDiv(lhsVal, rhsVal);
      break;
    case TK_PERCENTEQUALS:
      res = cge.builder.CreateSRem(lhsVal, rhsVal);
      break;
    case TK_ANDEQUALS:
      res = cge.builder.CreateAnd(lhsVal, rhsVal);
      break;
    case TK_PIPEEQUALS:
      res = cge.builder.CreateOr(lhsVal, rhsVal);
      break;
    case TK_CARETEQUALS:
      res = cge.builder.CreateXor(lhsVal, rhsVal);
      break;
    case TK_LANGLELANGLEEQUALS:
      res = cge.builder.CreateShl(lhsVal, rhsVal);
      break;
    case TK_RANGLERANGLEEQUALS:
      res = cge.builder.CreateAShr(lhsVal, rhsVal);
      break;
    default:
      errorloc(*this, "shouldn't happen");
  }
  cge.builder.CreateStore(res, lhsAddr);
  return res;
}

Value* FunctionCall::makeRValue(CodeGenEnv& cge) {
    // Get function value
    llvm::Value* func_val = function->makeRValue(cge);
    
    // Get the C4 function type for parameter type info
    std::shared_ptr<C4Type> funcC4Type = function->exprType;
    if (funcC4Type->isPointer()) {
        funcC4Type = std::dynamic_pointer_cast<C4PointerType>(funcC4Type)->getBase();
    }
    auto c4FuncType = std::dynamic_pointer_cast<C4FunctionType>(funcC4Type);
    
    // Generate arguments with type coercion
    std::vector<llvm::Value*> args;
    for (size_t i = 0; i < arguments.size(); ++i) {
        llvm::Value* argVal = arguments[i]->makeRValue(cge);
        // Coerce argument type to match parameter type if needed
        if (c4FuncType && i < c4FuncType->getParams().size()) {
            llvm::Type* paramTy = c4FuncType->getParams()[i]->getLLVMType(cge.builder);
            if (argVal->getType() != paramTy) {
                if (paramTy->isIntegerTy() && argVal->getType()->isIntegerTy()) {
                    argVal = cge.builder.CreateIntCast(argVal, paramTy, true);
                } else if (paramTy->isPointerTy() && argVal->getType()->isIntegerTy()) {
                    argVal = cge.builder.CreateIntToPtr(argVal, paramTy);
                }
            }
        }
        args.push_back(argVal);
    }
    
    // Check if it's a direct function or function pointer
    if (llvm::Function* func = llvm::dyn_cast<llvm::Function>(func_val)) {
        // Direct function call
        return cge.builder.CreateCall(func, args);
    } else {
        // Function pointer call — derive the function type from the expression's C4 type
        auto* llvmFuncTy = cast<FunctionType>(c4FuncType->getLLVMType(cge.builder));
        return cge.builder.CreateCall(llvmFuncTy, func_val, args);
    }
}

Value* Index::makeLValue(CodeGenEnv& cge) {
  /* Load the value of I */
  Value *ValI = is_swapped ? array->makeRValue(cge) : index->makeRValue(cge);

  /* Adjust the value to i64 for an index computation in an array
   *   The SExtOrTrunc will not change the type if it is already i64 */
  Value *ValI2I64 = cge.builder.CreateSExtOrTrunc(ValI, cge.builder.getInt64Ty());

  /* Create the index vector to access the 'i-th' element */
  std::vector<Value *> ArgvIndices;
  ArgvIndices.push_back(ValI2I64);

  /* Load the base pointer */
  Value *ValArgv = is_swapped ? index->makeRValue(cge) : array->makeRValue(cge);

  /* Use the actual element type for GEP offset computation */
  Type* elemTy = exprType->getLLVMType(cge.builder);

  /* Compute the addr of the 'i-th' element */
  return cge.builder.CreateGEP(elemTy, ValArgv, ArgvIndices);
}

Value* Cast::makeRValue(CodeGenEnv& cge) {
  Value* opVal = operand->makeRValue(cge);

  Type* destTy = exprType->getLLVMType(cge.builder);
  Type* srcTy = opVal->getType();

  // The result is void and cannot be used, so we return nullptr.
  if (destTy->isVoidTy()) {
    return nullptr; 
  }

  // Handle Redundant Casts
  if (srcTy == destTy) {
    return opVal;
  }

  // Integer to Integer Casts (char <-> int)
  if (srcTy->isIntegerTy() && destTy->isIntegerTy()) {
    return cge.builder.CreateIntCast(opVal, destTy, /*isSigned=*/true);
  }

  // Pointer to Integer Casts
  if (srcTy->isPointerTy() && destTy->isIntegerTy()) {
    return cge.builder.CreatePtrToInt(opVal, destTy);
  }

  // Integer to Pointer Casts
  if (srcTy->isIntegerTy() && destTy->isPointerTy()) {
    return cge.builder.CreateIntToPtr(opVal, destTy);
  }

  // Fallback
  return cge.builder.CreateBitCast(opVal, destTy);
}

Value* SizeOf::makeRValue(CodeGenEnv& cge) {
  Type* targetType = operandType->getLLVMType(cge.builder);

  // Function types are unsized in LLVM; sizeof(function) is 1 (GCC extension)
  if (targetType->isFunctionTy()) {
    return cge.builder.getInt32(1);
  }

  uint64_t size = cge.module->getDataLayout().getTypeAllocSize(targetType);
  
  return cge.builder.getInt32(size);
}

Value* Member::makeLValue(CodeGenEnv& cge) {
  Value* basePtr = nullptr;
  std::shared_ptr<C4StructType> structType = nullptr;

  if (is_arrow) {
    // Case: ptr->member
    // 'object' is a pointer to a struct. We load the pointer's value (R-Value).
    basePtr = object->makeRValue(cge);

    // object->exprType should be a C4PointerType wrapping a C4StructType
    auto ptrType = std::dynamic_pointer_cast<C4PointerType>(object->exprType);
    if (ptrType) {
        structType = std::dynamic_pointer_cast<C4StructType>(ptrType->getBase());
    }
  } else {
    // Case: obj.member
    // 'object' is a struct instance. We get its address (L-Value).
    basePtr = object->makeLValue(cge);

    // object->exprType is the C4StructType itself
    structType = std::dynamic_pointer_cast<C4StructType>(object->exprType);
  }

  int memberIndex = -1;
  int currentIndex = 0;
  
  // Iterate over members of the C4StructType to find the matching name
  for (const auto& field : structType->getMembers()) { 
    if (field.name == member) {
      memberIndex = currentIndex;
      break;
    }
    currentIndex++;
  }

  // Create the GEP instruction
  // We need the LLVM Type of the Struct to ensure GEP calculates offsets correctly.
  Type* llvmStructType = structType->getLLVMType(cge.builder);

  std::vector<Value*> indices;
  indices.push_back(cge.builder.getInt32(0));
  indices.push_back(cge.builder.getInt32(memberIndex));

  return cge.builder.CreateInBoundsGEP(llvmStructType, basePtr, indices);
}

////////////////////////////////////////////////////////////////////////////////
//                                Statements                                  //
////////////////////////////////////////////////////////////////////////////////

void CompoundStmt::codeGen(CodeGenEnv& cge) {
  for (auto& stmt : components) {
    stmt->codeGen(cge);
  }
}

void ReturnStmt::codeGen(CodeGenEnv& cge) {
  if (expr) {
    Value* val = expr->makeRValue(cge);
    // Coerce return value to match function return type
    Type* retTy = cge.builder.getCurrentFunctionReturnType();
    if (val->getType() != retTy) {
      if (retTy->isPointerTy() && val->getType()->isIntegerTy()) {
        val = cge.builder.CreateIntToPtr(val, retTy);
      } else if (retTy->isIntegerTy() && val->getType()->isIntegerTy()) {
        val = cge.builder.CreateIntCast(val, retTy, true);
      }
    }
    cge.builder.CreateRet(val);
  } else {
    cge.builder.CreateRetVoid();
  }

  BasicBlock *ReturnDeadBlock = BasicBlock::Create(cge.ctx, "DEAD_BLOCK", cge.currentFunction);
  cge.builder.SetInsertPoint(ReturnDeadBlock);
}

void IfStmt::codeGen(CodeGenEnv& cge) {
  Function* func = cge.currentFunction;

  // Create Blocks
  BasicBlock* ifHeader = BasicBlock::Create(cge.ctx, "if-header", func);
  BasicBlock* trueBB = BasicBlock::Create(cge.ctx, "if-consequence", func);
  BasicBlock* falseBB = BasicBlock::Create(cge.ctx, "if-alternative", func);
  BasicBlock* ifEnd = BasicBlock::Create(cge.ctx, "if-end", func);

  // create a branch from current BB to ifHeader
  cge.builder.CreateBr(ifHeader);

  // Set the header of the IfStmt as the new insert point
  cge.builder.SetInsertPoint(ifHeader);

  condition->makeCF(cge, trueBB, falseBB);

  // emit then
  cge.builder.SetInsertPoint(trueBB);
  consequence->codeGen(cge);
  if (!cge.builder.GetInsertBlock()->getTerminator())
    cge.builder.CreateBr(ifEnd);

  // emit else (if exists)
  cge.builder.SetInsertPoint(falseBB);
  if (alternative)
    alternative->codeGen(cge);

  if (!cge.builder.GetInsertBlock()->getTerminator())
    cge.builder.CreateBr(ifEnd);
  cge.builder.SetInsertPoint(ifEnd);
}

void NullStmt::codeGen(CodeGenEnv& cge) {}

void ExpressionStmt::codeGen(CodeGenEnv& cge) {
  if (expr) expr->makeRValue(cge);
}

void LabelStmt::codeGen(CodeGenEnv& cge) {
    Function *func = cge.currentFunction;

    if (!labelBlock) {
      labelBlock = BasicBlock::Create(cge.ctx, *label, func);
    }

    // Branch from current block to the label block (fall-through)
    if (!cge.builder.GetInsertBlock()->getTerminator()) {
      cge.builder.CreateBr(labelBlock);
    }

    cge.builder.SetInsertPoint(labelBlock);

    stmt->codeGen(cge);
}

void GotoStmt::codeGen(CodeGenEnv& cge) {
    LabelStmt* target = this->destination; 

    // Access the block stored
    if (!target->labelBlock) {
        target->labelBlock = BasicBlock::Create(cge.ctx, *label, cge.currentFunction);
    }
    
    // Jump to it
    cge.builder.CreateBr(target->labelBlock);
    
    BasicBlock *deadBlock = BasicBlock::Create(cge.ctx, "DEAD_BLOCK", cge.currentFunction);
    cge.builder.SetInsertPoint(deadBlock);
}

void BreakContinueStmt::codeGen(CodeGenEnv& cge) {
    // Retrieve targets directly from the linked AST node
    BasicBlock *targetBB = is_continue ? destination->condBlock : destination->endBlock;

    cge.builder.CreateBr(targetBB);

    BasicBlock *deadBlock = BasicBlock::Create(cge.ctx, "DEAD_BLOCK", cge.currentFunction);
    cge.builder.SetInsertPoint(deadBlock);
}

void WhileStmt::codeGen(CodeGenEnv& cge) {
  Function* func = cge.currentFunction;

  // Create Blocks
  condBlock = BasicBlock::Create(cge.ctx, "while-header", func);
  BasicBlock* whileBody = BasicBlock::Create(cge.ctx, "while-body", func);
  endBlock = BasicBlock::Create(cge.ctx, "while-end", func);

  cge.builder.CreateBr(condBlock);
  cge.builder.SetInsertPoint(condBlock);

  condition->makeCF(cge, whileBody, endBlock);

  cge.builder.SetInsertPoint(whileBody);

  body->codeGen(cge);

  if (!cge.builder.GetInsertBlock()->getTerminator())
    cge.builder.CreateBr(condBlock);
  cge.builder.SetInsertPoint(endBlock);
}

void Declaration::codeGen(CodeGenEnv& cge) {
  Symbol varName = spec_decl->declarator->getInnermost().getName();
  if (!varName) return;

  // Local function declaration (e.g. "int func(void);")
  if (auto* funcType = dynamic_cast<C4FunctionType*>(c4Type.get())) {
    if (!cge.functions.count(varName)) {
      auto* llvmFuncType = cast<FunctionType>(funcType->getLLVMType(cge.builder));
      Function* func = Function::Create(
        llvmFuncType,
        Function::ExternalLinkage,
        *varName,
        cge.module.get()
      );
      cge.functions[varName] = func;
    }
    return;
  }

  // Regular variable declaration
  // Reset the alloca builder
  cge.allocaBuilder.SetInsertPoint(cge.allocaBuilder.GetInsertBlock(),
                               cge.allocaBuilder.GetInsertBlock()->begin());
  // Allocate stack space for the variable
  Value *varPtr = cge.allocaBuilder.CreateAlloca(c4Type->getLLVMType(cge.builder));
  cge.varToAddress[varName] = varPtr;
}