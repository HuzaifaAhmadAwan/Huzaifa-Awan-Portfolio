#include "ast.h"

#include <assert.h>

#include <memory>

////////////////////////////////////////////////////////////////////////////////
//                             Global Elements                                //
////////////////////////////////////////////////////////////////////////////////

ExternalDeclaration::ExternalDeclaration(const Locatable& loc,
                                         std::unique_ptr<SpecDecl>&& specDecl,
                                         std::unique_ptr<CompoundStmt>&& body)
    : Locatable(loc), spec_decl(std::move(specDecl)), body(std::move(body)) {
    assert(spec_decl);
}

ExternalDeclaration::~ExternalDeclaration(void) {}

TranslationUnit::TranslationUnit(void) {}

TranslationUnit::~TranslationUnit(void) {}

void TranslationUnit::addExternalDeclaration(
    std::unique_ptr<ExternalDeclaration>&& ext_decl) {
    assert(ext_decl);
    ext_decls.push_back(std::move(ext_decl));
}

////////////////////////////////////////////////////////////////////////////////
//                               Declarations                                 //
////////////////////////////////////////////////////////////////////////////////

Specifier::Specifier(const Locatable& loc) : Locatable(loc) {}

Specifier::~Specifier(void) {}

bool Specifier::declaresTag(void) const { return false; }

VoidSpecifier::VoidSpecifier(const Locatable& loc) : Specifier(loc) {}

VoidSpecifier::~VoidSpecifier(void) {}

std::unique_ptr<Specifier> VoidSpecifier::copy(void) const {
    return std::make_unique<VoidSpecifier>(*(Locatable*)this);
}

CharSpecifier::CharSpecifier(const Locatable& loc) : Specifier(loc) {}

CharSpecifier::~CharSpecifier(void) {}

std::unique_ptr<Specifier> CharSpecifier::copy(void) const {
    return std::make_unique<CharSpecifier>(*(Locatable*)this);
}

IntSpecifier::IntSpecifier(const Locatable& loc) : Specifier(loc) {}

IntSpecifier::~IntSpecifier(void) {}

std::unique_ptr<Specifier> IntSpecifier::copy(void) const {
    return std::make_unique<IntSpecifier>(*(Locatable*)this);
}

StructSpecifier::StructSpecifier(const Locatable& loc, Symbol tag)
    : Specifier(loc), tag(tag) {}

StructSpecifier::~StructSpecifier(void) {}

bool StructSpecifier::declaresTag(void) const {
    if (tag != nullptr) {
        return true;
    }
    for (auto& spec_decl : components) {
        if (spec_decl->specifier->declaresTag()) {
            return true;
        }
        if (spec_decl->declarator->declaresTag()) {
            return true;
        }
    }
    return false;
}

void StructSpecifier::addComponent(std::unique_ptr<SpecDecl>&& spec_decl) {
    assert(spec_decl);
    components.push_back(std::move(spec_decl));
}

std::unique_ptr<Specifier> StructSpecifier::copy(void) const {
    auto res = std::make_unique<StructSpecifier>(*(Locatable*)this, tag);
    for (const auto& ds : components) {
        res->addComponent(ds->copy());
    }
    return res;
}

Declarator::Declarator(const Locatable& loc) : Locatable(loc) {}

Declarator::~Declarator(void) {}

FunctionDeclarator::FunctionDeclarator(const Locatable& loc,
                                       std::unique_ptr<Declarator>&& inner)
    : Declarator(loc), inner(std::move(inner)) {
    assert(this->inner);
}

FunctionDeclarator::~FunctionDeclarator(void) {}

void FunctionDeclarator::addParameter(std::unique_ptr<SpecDecl>&& spec_decl) {
    assert(spec_decl);
    params.push_back(std::move(spec_decl));
}

Declarator::Kind FunctionDeclarator::checkKind(Declarator::Kind kind) {
    auto ik = inner->checkKind(kind);

    if (ik == Kind::CONCRETE && params.empty()) {
        errorloc(*this, "Found empty parameter list in concrete declarator!");
    }
    return ik;
}

bool FunctionDeclarator::declaresFunType(void) const {
    if (dynamic_cast<PrimitiveDeclarator*>(inner.get())) {
        return true;
    }
    return inner->declaresFunType();
}

bool FunctionDeclarator::declaresTag(void) const {
    if (inner->declaresTag()) {
        return true;
    }
    for (auto& spec_decl : params) {
        if (spec_decl->specifier->declaresTag() ||
            spec_decl->declarator->declaresTag()) {
            return true;
        }
    }
    return false;
}

bool FunctionDeclarator::declaresVar(void) const {
    return inner->declaresVar();
}

bool FunctionDeclarator::isEmptyDeclarator(void) const { return false; }

std::unique_ptr<Declarator> FunctionDeclarator::copy(void) const {
    auto res =
        std::make_unique<FunctionDeclarator>(*(Locatable*)this, inner->copy());
    for (const auto& ds : params) {
        res->addParameter(ds->copy());
    }
    return res;
}

const PrimitiveDeclarator& FunctionDeclarator::getInnermost(void) const {
    return inner->getInnermost();
}

const FunctionDeclarator* FunctionDeclarator::getInnermostFunDecl(void) const {
    auto* res = inner->getInnermostFunDecl();
    return res ? res : this;
}

PointerDeclarator::PointerDeclarator(const Locatable& loc,
                                     std::unique_ptr<Declarator>&& inner)
    : Declarator(loc), inner(std::move(inner)) {
    assert(this->inner);
}

PointerDeclarator::~PointerDeclarator(void) {}

Declarator::Kind PointerDeclarator::checkKind(Declarator::Kind kind) {
    return inner->checkKind(kind);
}

bool PointerDeclarator::declaresFunType(void) const {
    return inner->declaresFunType();
}

bool PointerDeclarator::declaresTag(void) const { return inner->declaresTag(); }

bool PointerDeclarator::declaresVar(void) const { return inner->declaresVar(); }

bool PointerDeclarator::isEmptyDeclarator(void) const { return false; }

std::unique_ptr<Declarator> PointerDeclarator::copy(void) const {
    return std::make_unique<PointerDeclarator>(*(Locatable*)this,
                                               inner->copy());
}

const PrimitiveDeclarator& PointerDeclarator::getInnermost(void) const {
    return inner->getInnermost();
}

const FunctionDeclarator* PointerDeclarator::getInnermostFunDecl(void) const {
    return inner->getInnermostFunDecl();
}

PrimitiveDeclarator::PrimitiveDeclarator(const Locatable& loc, Symbol name)
    : Declarator(loc), name(name) {}

PrimitiveDeclarator::~PrimitiveDeclarator(void) {}

bool PrimitiveDeclarator::declaresFunType(void) const { return false; }

bool PrimitiveDeclarator::declaresTag(void) const { return false; }

bool PrimitiveDeclarator::declaresVar(void) const { return name != nullptr; }

bool PrimitiveDeclarator::isEmptyDeclarator(void) const {
    return getName() == nullptr;
}

Declarator::Kind PrimitiveDeclarator::checkKind(Declarator::Kind kind) {
    auto res = (name == nullptr) ? Kind::ABSTRACT : Kind::CONCRETE;

    if (kind != Kind::ANY && kind != res) {
        if (res == Kind::ABSTRACT) {
            errorloc(
                *this,
                "Expected concrete declarator but got abstract declarator!");
        } else {
            errorloc(
                *this,
                "Expected abstract declarator but got concrete declarator!");
        }
    }

    return res;
}

const PrimitiveDeclarator& PrimitiveDeclarator::getInnermost(void) const {
    return *this;
}

const FunctionDeclarator* PrimitiveDeclarator::getInnermostFunDecl(void) const {
    return nullptr;
}

Symbol PrimitiveDeclarator::getName(void) const { return name; }

std::unique_ptr<Declarator> PrimitiveDeclarator::copy(void) const {
    return std::make_unique<PrimitiveDeclarator>(*(Locatable*)this, name);
}

SpecDecl::SpecDecl(std::unique_ptr<Specifier>&& spec,
                   std::unique_ptr<Declarator>&& decl, Declarator::Kind kind)
    : specifier(std::move(spec)), declarator(std::move(decl)) {
    assert(specifier);
    assert(declarator);
    if (!declarator->isEmptyDeclarator()) {
        // At all places where non-abstractness is required, declarators are
        // also optional in the grammar, therefore it's okay to accept a simple
        // empty primitive declarator here anyway.
        declarator->checkKind(kind);
    }
}

SpecDecl::~SpecDecl(void) {}

bool SpecDecl::isFunSpecDecl(void) const {
    return declarator->declaresFunType();
}

bool SpecDecl::declaresSomething(void) const {
    return specifier->declaresTag() || declarator->declaresTag() ||
           declarator->declaresVar();
}

std::unique_ptr<SpecDecl> SpecDecl::copy(void) const {
    return std::make_unique<SpecDecl>(specifier->copy(), declarator->copy(),
                                      Kind::ANY);
}

////////////////////////////////////////////////////////////////////////////////
//                                Expressions                                 //
////////////////////////////////////////////////////////////////////////////////

Expr::Expr(const Locatable& loc) : Locatable(loc) {}

Expr::~Expr(void) {}

IntLiteral::IntLiteral(const Locatable& loc, long val)
    : Expr(loc), value(val) {}
IntLiteral::~IntLiteral(void) {}

std::unique_ptr<Expr> IntLiteral::copy(void) const {
    return std::make_unique<IntLiteral>(*(Locatable*)this, value);
}

CharLiteral::CharLiteral(const Locatable& loc, Symbol txt)
    : Expr(loc), text(txt) {}
CharLiteral::~CharLiteral(void) {}

std::unique_ptr<Expr> CharLiteral::copy(void) const {
    return std::make_unique<CharLiteral>(*(Locatable*)this, text);
}

StringLiteral::StringLiteral(const Locatable& loc, Symbol txt)
    : Expr(loc), text(txt) {}
StringLiteral::~StringLiteral(void) {}

std::unique_ptr<Expr> StringLiteral::copy(void) const {
    return std::make_unique<StringLiteral>(*(Locatable*)this, text);
}

Identifier::Identifier(const Locatable& loc, Symbol nm)
    : Expr(loc), name(nm) {}
Identifier::~Identifier(void) {}

std::unique_ptr<Expr> Identifier::copy(void) const {
    return std::make_unique<Identifier>(*(Locatable*)this, name);
}

BinaryOp::BinaryOp(const Locatable& loc, std::unique_ptr<Expr>&& l,
                   TokenKind o, std::unique_ptr<Expr>&& r)
    : Expr(loc), left(std::move(l)), op(o), right(std::move(r)) {}
BinaryOp::~BinaryOp(void) {}

std::unique_ptr<Expr> BinaryOp::copy(void) const {
    return std::make_unique<BinaryOp>(*(Locatable*)this, left->copy(), op,
                                       right->copy());
}

UnaryOp::UnaryOp(const Locatable& loc, TokenKind o,
                 std::unique_ptr<Expr>&& operand_)
    : Expr(loc), op(o), operand(std::move(operand_)) {}
UnaryOp::~UnaryOp(void) {}

std::unique_ptr<Expr> UnaryOp::copy(void) const {
    return std::make_unique<UnaryOp>(*(Locatable*)this, op, operand->copy());
}

TernaryOp::TernaryOp(const Locatable& loc, std::unique_ptr<Expr>&& c,
                     std::unique_ptr<Expr>&& cons,
                     std::unique_ptr<Expr>&& alt)
    : Expr(loc), condition(std::move(c)), consequent(std::move(cons)),
      alternative(std::move(alt)) {}
TernaryOp::~TernaryOp(void) {}

std::unique_ptr<Expr> TernaryOp::copy(void) const {
    return std::make_unique<TernaryOp>(
        *(Locatable*)this, condition->copy(), consequent->copy(),
        alternative->copy());
}

Assignment::Assignment(const Locatable& loc, std::unique_ptr<Expr>&& l,
                       TokenKind o, std::unique_ptr<Expr>&& r)
    : Expr(loc), lhs(std::move(l)), op(o), rhs(std::move(r)) {}
Assignment::~Assignment(void) {}

std::unique_ptr<Expr> Assignment::copy(void) const {
    return std::make_unique<Assignment>(*(Locatable*)this, lhs->copy(), op,
                                         rhs->copy());
}

Cast::Cast(const Locatable& loc, std::unique_ptr<SpecDecl>&& t,
           std::unique_ptr<Expr>&& o)
    : Expr(loc), type(std::move(t)), operand(std::move(o)) {}
Cast::~Cast(void) {}

std::unique_ptr<Expr> Cast::copy(void) const {
    return std::make_unique<Cast>(*(Locatable*)this, type->copy(),
                                   operand->copy());
}

SizeOf::SizeOf(const Locatable& loc, std::unique_ptr<Expr>&& o)
    : Expr(loc), expr_operand(std::move(o)), type_operand(nullptr) {}
SizeOf::SizeOf(const Locatable& loc, std::unique_ptr<SpecDecl>&& t)
    : Expr(loc), expr_operand(nullptr), type_operand(std::move(t)) {}
SizeOf::~SizeOf(void) {}

std::unique_ptr<Expr> SizeOf::copy(void) const {
    if (expr_operand) {
        return std::make_unique<SizeOf>(*(Locatable*)this,
                                         expr_operand->copy());
    } else {
        return std::make_unique<SizeOf>(*(Locatable*)this,
                                         type_operand->copy());
    }
}

FunctionCall::FunctionCall(const Locatable& loc,
                           std::unique_ptr<Expr>&& func)
    : Expr(loc), function(std::move(func)) {}
FunctionCall::~FunctionCall(void) {}
void FunctionCall::addArgument(std::unique_ptr<Expr>&& arg) {
    arguments.push_back(std::move(arg));
}

std::unique_ptr<Expr> FunctionCall::copy(void) const {
    auto res = std::make_unique<FunctionCall>(*(Locatable*)this,
                                               function->copy());
    for (const auto& arg : arguments) {
        res->addArgument(arg->copy());
    }
    return res;
}

Index::Index(const Locatable& loc, std::unique_ptr<Expr>&& arr,
             std::unique_ptr<Expr>&& idx)
    : Expr(loc), array(std::move(arr)), index(std::move(idx)) {}
Index::~Index(void) {}

std::unique_ptr<Expr> Index::copy(void) const {
    return std::make_unique<Index>(*(Locatable*)this, array->copy(),
                                    index->copy());
}

Member::Member(const Locatable& loc, std::unique_ptr<Expr>&& obj,
               bool arrow, Symbol mem)
    : Expr(loc), object(std::move(obj)), is_arrow(arrow), member(mem) {}
Member::~Member(void) {}

std::unique_ptr<Expr> Member::copy(void) const {
    return std::make_unique<Member>(*(Locatable*)this, object->copy(),
                                     is_arrow, member);
}

////////////////////////////////////////////////////////////////////////////////
//                                Statements                                  //
////////////////////////////////////////////////////////////////////////////////

CompoundStmt::CompoundStmt(const Locatable& loc) : Statement(loc) {}

CompoundStmt::~CompoundStmt(void) {}

void CompoundStmt::addComponent(std::unique_ptr<Statement>&& stmt) {
    assert(stmt);
    components.push_back(std::move(stmt));
}

std::unique_ptr<Statement> CompoundStmt::copy(void) const {
    auto res = std::make_unique<CompoundStmt>(*(Locatable*)this);
    for (const auto& c : components) {
        res->addComponent(c->copy());
    }
    return res;
}

LabelStmt::LabelStmt(const Locatable& loc, std::unique_ptr<Statement>&& stmt,
                     Symbol label)
    : Statement(loc), stmt(std::move(stmt)), label(label) {
    assert(this->label);
    assert(this->stmt);
}

LabelStmt::~LabelStmt(void) {}

std::unique_ptr<Statement> LabelStmt::copy(void) const {
    return std::make_unique<LabelStmt>(*(Locatable*)this, stmt->copy(), label);
}

ExpressionStmt::ExpressionStmt(const Locatable& loc,
                               std::unique_ptr<Expr>&& expr)
    : Statement(loc), expr(std::move(expr)) {
    assert(this->expr);
}

ExpressionStmt::~ExpressionStmt(void) {}

std::unique_ptr<Statement> ExpressionStmt::copy(void) const {
    return std::make_unique<ExpressionStmt>(*(Locatable*)this, expr->copy());
}

NullStmt::NullStmt(const Locatable& loc) : Statement(loc) {}

NullStmt::~NullStmt(void) {}

std::unique_ptr<Statement> NullStmt::copy(void) const {
    return std::make_unique<NullStmt>(*(Locatable*)this);
}

IfStmt::IfStmt(const Locatable& loc, std::unique_ptr<Expr>&& cond,
               std::unique_ptr<Statement>&& cons,
               std::unique_ptr<Statement>&& alt)
    : Statement(loc),
      condition(std::move(cond)),
      consequence(std::move(cons)),
      alternative(std::move(alt)) {
    assert(condition);
    assert(consequence);
}

IfStmt::~IfStmt(void) {}

std::unique_ptr<Statement> IfStmt::copy(void) const {
    return std::make_unique<IfStmt>(
        *(Locatable*)this, condition->copy(), consequence->copy(),
        alternative ? alternative->copy() : nullptr);
}

WhileStmt::WhileStmt(const Locatable& loc, std::unique_ptr<Expr>&& cond,
                     std::unique_ptr<Statement>&& body)
    : Statement(loc), condition(std::move(cond)), body(std::move(body)) {
    assert(condition);
    assert(this->body);
}

WhileStmt::~WhileStmt(void) {}

std::unique_ptr<Statement> WhileStmt::copy(void) const {
    return std::make_unique<WhileStmt>(*(Locatable*)this, condition->copy(),
                                       body->copy());
}

ReturnStmt::ReturnStmt(const Locatable& loc, std::unique_ptr<Expr>&& expr)
    : Statement(loc), expr(std::move(expr)) {}

ReturnStmt::~ReturnStmt(void) {}

std::unique_ptr<Statement> ReturnStmt::copy(void) const {
    return std::make_unique<ReturnStmt>(*(Locatable*)this,
                                        expr ? expr->copy() : nullptr);
}

BreakContinueStmt::BreakContinueStmt(const Locatable& loc, bool is_continue)
    : Statement(loc), is_continue(is_continue) {}

BreakContinueStmt::~BreakContinueStmt(void) {}

std::unique_ptr<Statement> BreakContinueStmt::copy(void) const {
    return std::make_unique<BreakContinueStmt>(*(Locatable*)this, is_continue);
}

GotoStmt::GotoStmt(const Locatable& loc, Symbol label)
    : Statement(loc), label(label) {}

GotoStmt::~GotoStmt(void) {}

std::unique_ptr<Statement> GotoStmt::copy(void) const {
    return std::make_unique<GotoStmt>(*(Locatable*)this, label);
}

Declaration::Declaration(const Locatable& loc, std::unique_ptr<SpecDecl>&& tn)
    : Statement(loc), spec_decl(std::move(tn)) {
    assert(spec_decl);
}

Declaration::~Declaration(void) {}

std::unique_ptr<Statement> Declaration::copy(void) const {
    return std::make_unique<Declaration>(*(Locatable*)this, spec_decl->copy());
}
