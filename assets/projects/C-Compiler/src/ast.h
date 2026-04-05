#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include "diagnostic.h"
#include "symbol_internalizer.h"
#include "token.h"
#include "c4type.h"
#include "llvm/IR/IRBuilder.h"             /* IRBuilder */

struct CodeGenEnv;
class Scope;
class SpecDecl;
class CompoundStmt;
class Statement;

using StmtTransformer = std::function<void(std::unique_ptr<Statement>&)>;

////////////////////////////////////////////////////////////////////////////////
//                             Global Elements                                //
////////////////////////////////////////////////////////////////////////////////

class ExternalDeclaration : public Locatable {
   public:
    ExternalDeclaration(const Locatable& loc,
                        std::unique_ptr<SpecDecl>&& specDecl,
                        std::unique_ptr<CompoundStmt>&& body);

    ~ExternalDeclaration(void);

    void print(std::ostream& stream) const;
    void checkSemantics(const std::shared_ptr<Scope> &scope);
    void codeGen(CodeGenEnv& cge);
    // private:
    std::unique_ptr<SpecDecl> spec_decl;
    std::shared_ptr<C4Type> c4type;
    std::vector<Symbol> varNames;
    std::unique_ptr<CompoundStmt> body;  // may be nullptr
};

class TranslationUnit {
   public:
    TranslationUnit(void);

    ~TranslationUnit(void);

    void addExternalDeclaration(
        std::unique_ptr<ExternalDeclaration>&& ext_decl);

    void print(std::ostream& stream) const;
    void checkSemantics() const;
    void codeGen(const std::string& FileName, bool optimize = false);

    // private:
    std::vector<std::unique_ptr<ExternalDeclaration>> ext_decls;
};

////////////////////////////////////////////////////////////////////////////////
//                               Declarations                                 //
////////////////////////////////////////////////////////////////////////////////

class Specifier : public Locatable {
   public:
    Specifier(const Locatable& loc);

    virtual ~Specifier(void);

    virtual void print(std::ostream& stream, size_t indentlevel) const = 0;

    virtual bool declaresTag(void) const;

    virtual std::unique_ptr<Specifier> copy(void) const = 0;
    virtual std::shared_ptr<C4Type> checkSemantics() = 0;
};

class VoidSpecifier : public Specifier {
   public:
    VoidSpecifier(const Locatable& loc);

    virtual ~VoidSpecifier(void);

    virtual void print(std::ostream& stream, size_t indentlevel) const override;

    virtual std::unique_ptr<Specifier> copy(void) const override;
    std::shared_ptr<C4Type> checkSemantics() override;
};

class CharSpecifier : public Specifier {
   public:
    CharSpecifier(const Locatable& loc);

    virtual ~CharSpecifier(void);

    virtual void print(std::ostream& stream, size_t indentlevel) const override;

    virtual std::unique_ptr<Specifier> copy(void) const override;
    std::shared_ptr<C4Type> checkSemantics() override;
};

class IntSpecifier : public Specifier {
   public:
    IntSpecifier(const Locatable& loc);

    virtual ~IntSpecifier(void);

    virtual void print(std::ostream& stream, size_t indentlevel) const override;

    virtual std::unique_ptr<Specifier> copy(void) const override;
    std::shared_ptr<C4Type> checkSemantics() override;
};

class StructSpecifier : public Specifier {
   public:
    StructSpecifier(const Locatable& loc, Symbol tag);

    virtual ~StructSpecifier(void);

    virtual void print(std::ostream& stream, size_t indentlevel) const override;

    void addComponent(std::unique_ptr<SpecDecl>&& spec_decl);

    virtual bool declaresTag(void) const override;

    virtual std::unique_ptr<Specifier> copy(void) const override;
    std::shared_ptr<C4Type> checkSemantics() override;
    // private:
    Symbol tag;                                         // may be nullptr
    std::vector<std::unique_ptr<SpecDecl>> components;  // may be empty
};

class PrimitiveDeclarator;
class FunctionDeclarator;

class Declarator : public Locatable {
   public:
    // Declarators and abstract declarators share a lot of syntactical and
    // semantical properties. Therefore, we use the same AST data structures to
    // represent both. If at some place, a specific kind of declarators is
    // required (by giving one of the below `DeclKind`s), we need to check
    // separately whether the declarator satisfies this requirement.
    enum class Kind {
        ABSTRACT,
        CONCRETE,
        ANY,
    };

    Declarator(const Locatable& loc);

    ~Declarator(void);

    virtual Kind checkKind(Kind kind) = 0;

    virtual bool declaresFunType(void) const = 0;

    virtual bool declaresTag(void) const = 0;
    virtual bool declaresVar(void) const = 0;

    virtual bool isEmptyDeclarator(void) const = 0;

    virtual const PrimitiveDeclarator& getInnermost(void) const = 0;

    virtual const FunctionDeclarator* getInnermostFunDecl(void) const = 0;

    virtual void print(std::ostream& stream, size_t indentlevel) const = 0;

    virtual std::unique_ptr<Declarator> copy(void) const = 0;
    virtual std::shared_ptr<C4Type> buildType(const std::shared_ptr<C4Type> & acc) = 0;
};

class FunctionDeclarator : public Declarator {
   public:
    using Kind = Declarator::Kind;

    FunctionDeclarator(const Locatable& loc,
                       std::unique_ptr<Declarator>&& inner);

    ~FunctionDeclarator(void);

    void addParameter(std::unique_ptr<SpecDecl>&& spec_decl);

    virtual Kind checkKind(Kind kind) override;

    virtual bool declaresFunType(void) const override;

    virtual bool declaresTag(void) const override;
    virtual bool declaresVar(void) const override;

    virtual bool isEmptyDeclarator(void) const override;

    virtual const PrimitiveDeclarator& getInnermost(void) const override;

    virtual const FunctionDeclarator* getInnermostFunDecl(void) const override;

    virtual void print(std::ostream& stream, size_t indentlevel) const override;

    virtual std::unique_ptr<Declarator> copy(void) const override;
    std::shared_ptr<C4Type> buildType(const std::shared_ptr<C4Type> & acc) override;
    // private:
    std::unique_ptr<Declarator> inner;
    std::vector<std::unique_ptr<SpecDecl>> params;
};

class PointerDeclarator : public Declarator {
   public:
    using Kind = Declarator::Kind;

    PointerDeclarator(const Locatable& loc,
                      std::unique_ptr<Declarator>&& inner);

    ~PointerDeclarator(void);

    virtual Kind checkKind(Kind kind) override;

    virtual bool declaresFunType(void) const override;

    virtual bool declaresTag(void) const override;
    virtual bool declaresVar(void) const override;

    virtual bool isEmptyDeclarator(void) const override;

    virtual const PrimitiveDeclarator& getInnermost(void) const override;

    virtual const FunctionDeclarator* getInnermostFunDecl(void) const override;

    virtual void print(std::ostream& stream, size_t indentlevel) const override;

    virtual std::unique_ptr<Declarator> copy(void) const override;
    std::shared_ptr<C4Type> buildType(const std::shared_ptr<C4Type> & acc) override;
    // private:
    std::unique_ptr<Declarator> inner;
};

class PrimitiveDeclarator : public Declarator {
   public:
    using Kind = Declarator::Kind;

    PrimitiveDeclarator(const Locatable& loc, Symbol name);

    ~PrimitiveDeclarator(void);

    Symbol getName(void) const;

    virtual Kind checkKind(Kind kind) override;

    virtual bool declaresFunType(void) const override;

    virtual bool declaresTag(void) const override;
    virtual bool declaresVar(void) const override;

    virtual bool isEmptyDeclarator(void) const override;

    virtual const PrimitiveDeclarator& getInnermost(void) const override;

    virtual const FunctionDeclarator* getInnermostFunDecl(void) const override;

    virtual void print(std::ostream& stream, size_t indentlevel) const override;

    virtual std::unique_ptr<Declarator> copy(void) const override;
    std::shared_ptr<C4Type> buildType(const std::shared_ptr<C4Type> & acc) override;
    // private:
    Symbol name;  // may be nullptr
};

class SpecDecl {
   public:
    using Kind = Declarator::Kind;

    SpecDecl(std::unique_ptr<Specifier>&& spec,
             std::unique_ptr<Declarator>&& decl, Kind kind);

    ~SpecDecl(void);

    void print(std::ostream& stream, size_t indentlevel) const;

    bool isFunSpecDecl(void) const;

    bool declaresSomething(void) const;

    std::unique_ptr<SpecDecl> copy(void) const;
    std::shared_ptr<C4Type> checkSemantics();

    // private:
    std::unique_ptr<Specifier> specifier;
    std::unique_ptr<Declarator> declarator;
};

////////////////////////////////////////////////////////////////////////////////
//                                Expressions                                 //
////////////////////////////////////////////////////////////////////////////////

class Expr : public Locatable {
   public:
    virtual ~Expr(void);

    virtual void print(std::ostream& stream) const = 0;
    std::shared_ptr<C4Type> exprType = nullptr;
    virtual std::unique_ptr<Expr> copy(void) const = 0;
    virtual std::shared_ptr<C4Type> checkSemantics(std::shared_ptr<Scope> scope) = 0;
    virtual bool isLValue() const { return false; }
    virtual void makeCF(CodeGenEnv& cge, llvm::BasicBlock* trueBB, llvm::BasicBlock* falseBB);
    virtual llvm::Value* makeRValue(CodeGenEnv& cge);
    virtual llvm::Value* makeLValue(CodeGenEnv& cge) { errorloc(*this, "Not defined");}

  protected:
    Expr(const Locatable& loc);
};

class IntLiteral : public Expr {
   public:
    IntLiteral(const Locatable& loc, long value);
    virtual ~IntLiteral(void);
    std::shared_ptr<C4Type> checkSemantics(std::shared_ptr<Scope> scope) override;
    llvm::Value* makeRValue(CodeGenEnv& cge) override;
    void print(std::ostream& stream) const override;
    std::unique_ptr<Expr> copy(void) const override;
    long value;
};

class CharLiteral : public Expr {
   public:
    CharLiteral(const Locatable& loc, Symbol text);
    virtual ~CharLiteral(void);
    std::shared_ptr<C4Type> checkSemantics(std::shared_ptr<Scope> scope) override;
    llvm::Value* makeRValue(CodeGenEnv& cge) override;
    virtual void print(std::ostream& stream) const override;
    virtual std::unique_ptr<Expr> copy(void) const override;
    Symbol text;
};

class StringLiteral : public Expr {
   public:
    StringLiteral(const Locatable& loc, Symbol text);
    virtual ~StringLiteral(void);
  std::shared_ptr<C4Type> checkSemantics(std::shared_ptr<Scope> scope) override;
    llvm::Value* makeRValue(CodeGenEnv& cge) override;
    virtual void print(std::ostream& stream) const override;
    virtual std::unique_ptr<Expr> copy(void) const override;
    Symbol text;
};

class Identifier : public Expr {
   public:
    Identifier(const Locatable& loc, Symbol name);
    virtual ~Identifier(void);
    std::shared_ptr<C4Type> checkSemantics(std::shared_ptr<Scope> scope) override;
    llvm::Value* makeLValue(CodeGenEnv& cge) override;
    virtual void print(std::ostream& stream) const override;
    virtual std::unique_ptr<Expr> copy(void) const override;
    bool isLValue() const override { return true; }
    Symbol name;
};

class BinaryOp : public Expr {
   public:
    BinaryOp(const Locatable& loc, std::unique_ptr<Expr>&& left,
             TokenKind op, std::unique_ptr<Expr>&& right);
    virtual ~BinaryOp(void);
    std::shared_ptr<C4Type> checkSemantics(std::shared_ptr<Scope> scope) override;
    virtual void print(std::ostream& stream) const override;
    virtual std::unique_ptr<Expr> copy(void) const override;
    llvm::Value* makeRValue(CodeGenEnv& cge) override;
    void makeCF(CodeGenEnv &cge, llvm::BasicBlock *trueBB,
                   llvm::BasicBlock *falseBB) override;
    std::unique_ptr<Expr> left;
    TokenKind op;
    std::unique_ptr<Expr> right;
};

class UnaryOp : public Expr {
   public:
    UnaryOp(const Locatable& loc, TokenKind op, std::unique_ptr<Expr>&& operand);
    virtual ~UnaryOp(void);
  std::shared_ptr<C4Type> checkSemantics(std::shared_ptr<Scope> scope) override;
    virtual void print(std::ostream& stream) const override;
    virtual std::unique_ptr<Expr> copy(void) const override;
    bool isLValue() const override { return op == TK_ASTERISK; } // Dereference is lvalue
    llvm::Value* makeRValue(CodeGenEnv& cge) override;
    void makeCF(CodeGenEnv &cge, llvm::BasicBlock *trueBB, llvm::BasicBlock *falseBB) override;
    llvm::Value* makeLValue(CodeGenEnv& cge) override;
    TokenKind op;
    std::unique_ptr<Expr> operand;
};

class TernaryOp : public Expr {
   public:
    TernaryOp(const Locatable& loc, std::unique_ptr<Expr>&& condition,
              std::unique_ptr<Expr>&& consequent,
              std::unique_ptr<Expr>&& alternative);
    virtual ~TernaryOp(void);
  std::shared_ptr<C4Type> checkSemantics(std::shared_ptr<Scope> scope) override;
    llvm::Value* makeRValue(CodeGenEnv& cge) override;
    void makeCF(CodeGenEnv &cge, llvm::BasicBlock *trueBB, llvm::BasicBlock *falseBB) override;
    virtual void print(std::ostream& stream) const override;
    virtual std::unique_ptr<Expr> copy(void) const override;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> consequent;
    std::unique_ptr<Expr> alternative;
};

class Assignment : public Expr {
   public:
    Assignment(const Locatable& loc, std::unique_ptr<Expr>&& lhs,
               TokenKind op, std::unique_ptr<Expr>&& rhs);
    virtual ~Assignment(void);
    std::shared_ptr<C4Type> checkSemantics(std::shared_ptr<Scope> scope) override;
    llvm::Value* makeRValue(CodeGenEnv& cge) override;
    virtual void print(std::ostream& stream) const override;
    virtual std::unique_ptr<Expr> copy(void) const override;
    std::unique_ptr<Expr> lhs;
    TokenKind op;
    std::unique_ptr<Expr> rhs;
};

class Cast : public Expr {
   public:
    Cast(const Locatable& loc, std::unique_ptr<SpecDecl>&& type,
         std::unique_ptr<Expr>&& operand);
    virtual ~Cast(void);
    std::shared_ptr<C4Type> checkSemantics(std::shared_ptr<Scope> scope) override;
    llvm::Value* makeRValue(CodeGenEnv& cge) override;
    virtual void print(std::ostream& stream) const override;
    virtual std::unique_ptr<Expr> copy(void) const override;
    std::unique_ptr<SpecDecl> type;
    std::unique_ptr<Expr> operand;
};

class SizeOf : public Expr {
   public:
    SizeOf(const Locatable& loc, std::unique_ptr<Expr>&& operand);
    SizeOf(const Locatable& loc, std::unique_ptr<SpecDecl>&& type);
    virtual ~SizeOf(void);
    std::shared_ptr<C4Type> checkSemantics(std::shared_ptr<Scope> scope) override;
    llvm::Value* makeRValue(CodeGenEnv& cge) override;
    virtual void print(std::ostream& stream) const override;
    virtual std::unique_ptr<Expr> copy(void) const override;
    std::unique_ptr<Expr> expr_operand;
    std::unique_ptr<SpecDecl> type_operand;
    std::shared_ptr<C4Type> operandType = nullptr;
};

class FunctionCall : public Expr {
   public:
    FunctionCall(const Locatable& loc, std::unique_ptr<Expr>&& function);
    virtual ~FunctionCall(void);
  std::shared_ptr<C4Type> checkSemantics(std::shared_ptr<Scope> scope) override;
    llvm::Value* makeRValue(CodeGenEnv& cge) override;
    void addArgument(std::unique_ptr<Expr>&& arg);
    virtual void print(std::ostream& stream) const override;
    virtual std::unique_ptr<Expr> copy(void) const override;
    std::unique_ptr<Expr> function;
    std::vector<std::unique_ptr<Expr>> arguments;
};

class Index : public Expr {
   public:
    Index(const Locatable& loc, std::unique_ptr<Expr>&& array,
          std::unique_ptr<Expr>&& index);
    virtual ~Index(void);
  std::shared_ptr<C4Type> checkSemantics(std::shared_ptr<Scope> scope) override;
    llvm::Value* makeLValue(CodeGenEnv& cge) override;
    virtual void print(std::ostream& stream) const override;
    virtual std::unique_ptr<Expr> copy(void) const override;
    bool isLValue() const override { return true; }
    std::unique_ptr<Expr> array;
    std::unique_ptr<Expr> index;
    bool is_swapped = false;
};

class Member : public Expr {
   public:
    Member(const Locatable& loc, std::unique_ptr<Expr>&& object, bool is_arrow,
           Symbol member);
    virtual ~Member(void);
  std::shared_ptr<C4Type> checkSemantics(std::shared_ptr<Scope> scope) override;
    llvm::Value* makeLValue(CodeGenEnv& cge) override;
    virtual void print(std::ostream& stream) const override;
    virtual std::unique_ptr<Expr> copy(void) const override;
    bool isLValue() const override { return true; }
    std::unique_ptr<Expr> object;
    bool is_arrow;
    Symbol member;
};

////////////////////////////////////////////////////////////////////////////////
//                                Statements                                  //
////////////////////////////////////////////////////////////////////////////////

class Statement : public Locatable {
   public:
    virtual ~Statement(void) {}

    virtual void checkSemantics(std::shared_ptr<Scope> scope) = 0;
    virtual void print(std::ostream& stream, size_t indentlevel,
                       bool compoundskip) const = 0;

    virtual std::unique_ptr<Statement> copy(void) const = 0;
    virtual void codeGen(CodeGenEnv& cge) = 0;

   protected:
    Statement(const Locatable& loc) : Locatable(loc) {}
};

class CompoundStmt : public Statement {
   public:
    CompoundStmt(const Locatable& loc);

    virtual ~CompoundStmt(void);

    void checkSemantics(std::shared_ptr<Scope> scope) override;
    void codeGen(CodeGenEnv& cge) override;
    void addComponent(std::unique_ptr<Statement>&& stmt);

    virtual void print(std::ostream& stream, size_t indentlevel,
                       bool compoundskip) const override;

    virtual std::unique_ptr<Statement> copy(void) const override;

    // private:
    std::vector<std::unique_ptr<Statement>> components;
    bool is_fun_body = false;
};

class LabelStmt : public Statement {
   public:
    LabelStmt(const Locatable& loc, std::unique_ptr<Statement>&& stmt,
              Symbol label);

    virtual ~LabelStmt(void);
    void checkSemantics(std::shared_ptr<Scope> scope) override;

    virtual void print(std::ostream& stream, size_t indentlevel,
                       bool compoundskip) const override;

    virtual std::unique_ptr<Statement> copy(void) const override;
    void codeGen(CodeGenEnv &cge) override;

    // private:
    std::unique_ptr<Statement> stmt;
    Symbol label;
    llvm::BasicBlock* labelBlock = nullptr;
};

class ExpressionStmt : public Statement {
   public:
    ExpressionStmt(const Locatable& loc, std::unique_ptr<Expr>&& expr);

    virtual ~ExpressionStmt(void);
    void checkSemantics(std::shared_ptr<Scope> scope) override;
    void codeGen(CodeGenEnv& cge) override;

    virtual void print(std::ostream& stream, size_t indentlevel,
                       bool compoundskip) const override;

    virtual std::unique_ptr<Statement> copy(void) const override;

    // private:
    std::unique_ptr<Expr> expr;
};

class NullStmt : public Statement {
   public:
    NullStmt(const Locatable& loc);

    virtual ~NullStmt(void);
    void checkSemantics(std::shared_ptr<Scope> scope) override;
    void codeGen(CodeGenEnv& cge) override;

    virtual void print(std::ostream& stream, size_t indentlevel,
                       bool compoundskip) const override;

    virtual std::unique_ptr<Statement> copy(void) const override;
};

class IfStmt : public Statement {
   public:
    IfStmt(const Locatable& loc, std::unique_ptr<Expr>&& cond,
           std::unique_ptr<Statement>&& cons, std::unique_ptr<Statement>&& alt);

    virtual ~IfStmt(void);
    void checkSemantics(std::shared_ptr<Scope> scope) override;

    virtual void print(std::ostream& stream, size_t indentlevel,
                       bool compoundskip) const override;

    virtual std::unique_ptr<Statement> copy(void) const override;
    void codeGen(CodeGenEnv &cge) override;

    // private:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Statement> consequence;
    std::unique_ptr<Statement> alternative;  // may be nullptr
};

class WhileStmt : public Statement {
   public:
    WhileStmt(const Locatable& loc, std::unique_ptr<Expr>&& cond,
              std::unique_ptr<Statement>&& body);

    virtual ~WhileStmt(void);
    void checkSemantics(std::shared_ptr<Scope> scope) override;
    void codeGen(CodeGenEnv& cge) override;

    virtual void print(std::ostream& stream, size_t indentlevel,
                       bool compoundskip) const override;

    virtual std::unique_ptr<Statement> copy(void) const override;

    // private:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Statement> body;
    llvm::BasicBlock* condBlock = nullptr;
    llvm::BasicBlock* endBlock = nullptr;
};

class ReturnStmt : public Statement {
   public:
    ReturnStmt(const Locatable& loc, std::unique_ptr<Expr>&& expr);

    virtual ~ReturnStmt(void);
    void checkSemantics(std::shared_ptr<Scope> scope) override;

    virtual void print(std::ostream& stream, size_t indentlevel,
                       bool compoundskip) const override;

    virtual std::unique_ptr<Statement> copy(void) const override;
    void codeGen(CodeGenEnv &cge) override;

    // private:
    std::unique_ptr<Expr> expr;  // may be nullptr
};

class BreakContinueStmt : public Statement {
   public:
    BreakContinueStmt(const Locatable& loc, bool is_continue);

    virtual ~BreakContinueStmt(void);
    void checkSemantics(std::shared_ptr<Scope> scope) override;

    virtual void print(std::ostream& stream, size_t indentlevel,
                       bool compoundskip) const override;

    virtual std::unique_ptr<Statement> copy(void) const override;
    void codeGen(CodeGenEnv &cge) override;

    // private:
    bool is_continue;

    WhileStmt* destination = nullptr;
};

class GotoStmt : public Statement {
   public:
    GotoStmt(const Locatable& loc, Symbol label);

    virtual ~GotoStmt(void);
    void checkSemantics(std::shared_ptr<Scope> scope) override;

    virtual void print(std::ostream& stream, size_t indentlevel,
                       bool compoundskip) const override;

    virtual std::unique_ptr<Statement> copy(void) const override;
    void codeGen(CodeGenEnv &cge) override;

    // private:
    Symbol label;

    LabelStmt* destination = nullptr;
};

class Declaration : public Statement {
    // Note that according to the grammar, declarations cannot occur wherever
    // statements can, but the parser ensures this already. This simplifies our
    // AST by avoiding an unnecessary BlockItem class.
   public:
    Declaration(const Locatable& loc, std::unique_ptr<SpecDecl>&& tn);

    ~Declaration(void);
    void checkSemantics(std::shared_ptr<Scope> scope) override;
    void codeGen(CodeGenEnv& cge) override;

    virtual void print(std::ostream& stream, size_t indentlevel,
                       bool compoundskip) const override;

    virtual std::unique_ptr<Statement> copy(void) const override;

    // private:
    std::shared_ptr<C4Type> c4Type;
    std::unique_ptr<SpecDecl> spec_decl;
};
