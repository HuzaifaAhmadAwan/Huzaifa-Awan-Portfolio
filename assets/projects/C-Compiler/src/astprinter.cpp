#include "ast.h"

namespace {

void doIndentation(std::ostream& stream, size_t indentLevel) {
    stream << '\n';
    for (size_t i = 0; i < indentLevel; ++i) {
        stream << '\t';
    }
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////
//                             Global Elements                                //
////////////////////////////////////////////////////////////////////////////////

void ExternalDeclaration::print(std::ostream& stream) const {
    spec_decl->print(stream, 0);
    if (body) {
        body->print(stream, 0, false);
    } else {
        stream << ';';
    }
}

void TranslationUnit::print(std::ostream& stream) const {
    bool first = true;
    for (auto& ed : ext_decls) {
        if (!first) {
            stream << '\n';
        }
        first = false;
        ed->print(stream);
        stream << '\n';
    }
}

////////////////////////////////////////////////////////////////////////////////
//                               Declarations                                 //
////////////////////////////////////////////////////////////////////////////////

void VoidSpecifier::print(std::ostream& stream, size_t) const {
    stream << "void";
}

void CharSpecifier::print(std::ostream& stream, size_t) const {
    stream << "char";
}

void IntSpecifier::print(std::ostream& stream, size_t) const {
    stream << "int";
}

void StructSpecifier::print(std::ostream& stream, size_t indentLevel) const {
    stream << "struct";

    if (tag) {
        stream << ' ' << *tag;
    }

    if (components.size() > 0) {
        doIndentation(stream, indentLevel);
        stream << '{';

        for (auto& comp : components) {
            doIndentation(stream, indentLevel + 1);
            comp->print(stream, indentLevel + 1);
            stream << ';';
        }

        doIndentation(stream, indentLevel);
        stream << '}';
    }
}

void FunctionDeclarator::print(std::ostream& stream, size_t indentlevel) const {
    stream << '(';
    inner->print(stream, indentlevel);
    stream << '(';

    bool first = true;
    for (auto& ds : params) {
        if (!first) {
            stream << ", ";
        }
        first = false;
        ds->print(stream, indentlevel);
    }

    stream << ')';
    stream << ')';
}

void PointerDeclarator::print(std::ostream& stream, size_t indentlevel) const {
    stream << '(';
    stream << '*';
    inner->print(stream, indentlevel);
    stream << ')';
}

void PrimitiveDeclarator::print(std::ostream& stream, size_t) const {
    if (name) {
        stream << *name;
    }
}

void SpecDecl::print(std::ostream& stream, size_t indentLevel) const {
    specifier->print(stream, indentLevel);
    if (!declarator->isEmptyDeclarator()) {
        stream << ' ';
        declarator->print(stream, indentLevel);
    }
}

////////////////////////////////////////////////////////////////////////////////
//                                Expressions                                 //
////////////////////////////////////////////////////////////////////////////////
namespace {
const char* tokenKindToString(TokenKind tk) {
    switch (tk) {
        case TK_PLUS: return "+";
        case TK_MINUS: return "-";
        case TK_ASTERISK: return "*";
        case TK_SLASH: return "/";
        case TK_PERCENT: return "%";
        case TK_EQUALSEQUALS: return "==";
        case TK_BANGEQUALS: return "!=";
        case TK_LANGLE: return "<";
        case TK_RANGLE: return ">";
        case TK_LANGLEEQUALS: return "<=";
        case TK_RANGLEEQUALS: return ">=";
        case TK_ANDAND: return "&&";
        case TK_PIPEPIPE: return "||";
        case TK_AND: return "&";
        case TK_PIPE: return "|";
        case TK_CARET: return "^";
        case TK_LANGLELANGLE: return "<<";
        case TK_RANGLERANGLE: return ">>";
        case TK_EQUALS: return "=";
        case TK_PLUSEQUALS: return "+=";
        case TK_MINUSEQUALS: return "-=";
        case TK_ASTERISKEQUALS: return "*=";
        case TK_SLASHEQUALS: return "/=";
        case TK_PERCENTEQUALS: return "%=";
        case TK_ANDEQUALS: return "&=";
        case TK_PIPEEQUALS: return "|=";
        case TK_CARETEQUALS: return "^=";
        case TK_LANGLELANGLEEQUALS: return "<<=";
        case TK_RANGLERANGLEEQUALS: return ">>=";
        case TK_BANG: return "!";
        case TK_TILDE: return "~";
        case TK_PLUSPLUS: return "++";
        case TK_MINUSMINUS: return "--";
        default: return "?";
    }
}
}  // namespace

void IntLiteral::print(std::ostream& stream) const { stream << value; }
void CharLiteral::print(std::ostream& stream) const { stream << *text; }
void StringLiteral::print(std::ostream& stream) const { stream << *text; }
void Identifier::print(std::ostream& stream) const { stream << *name; }
void BinaryOp::print(std::ostream& stream) const {
    stream << "(";
    left->print(stream);
    stream << " " << tokenKindToString(op) << " ";
    right->print(stream);
    stream << ")";
}
void UnaryOp::print(std::ostream& stream) const {
    stream << "(" << tokenKindToString(op);
    operand->print(stream);
    stream << ")";
}
void TernaryOp::print(std::ostream& stream) const {
    stream << "(";
    condition->print(stream);
    stream << " ? ";
    consequent->print(stream);
    stream << " : ";
    alternative->print(stream);
    stream << ")";
}
void Assignment::print(std::ostream& stream) const {
    stream << "(";
    lhs->print(stream);
    stream << " " << tokenKindToString(op) << " ";
    rhs->print(stream);
    stream << ")";
}
void Cast::print(std::ostream& stream) const {
    stream << "(";
    stream << "(";
    type->print(stream, 0);
    stream << ")";
    operand->print(stream);
    stream << ")";
}
void SizeOf::print(std::ostream& stream) const {
    if (expr_operand) {
        stream << "(sizeof ";
        expr_operand->print(stream);
        stream << ")";
    } else {
        stream << "(sizeof(";
        type_operand->print(stream, 0);
        stream << "))";
    }
}
void FunctionCall::print(std::ostream& stream) const {
    stream << "(";
    function->print(stream);
    stream << "(";
    for (size_t i = 0; i < arguments.size(); ++i) {
        if (i > 0) stream << ", ";
        arguments[i]->print(stream);
    }
    stream << ")";
    stream << ")";
}
void Index::print(std::ostream& stream) const {
    stream << "(";
    array->print(stream);
    stream << "[";
    index->print(stream);
    stream << "]";
    stream << ")";
}
void Member::print(std::ostream& stream) const {
    stream << "(";
    object->print(stream);
    stream << (is_arrow ? "->" : ".");
    stream << *member;
    stream << ")";
}
////////////////////////////////////////////////////////////////////////////////
//                                Statements                                  //
////////////////////////////////////////////////////////////////////////////////

void CompoundStmt::print(std::ostream& stream, size_t indentlevel,
                         bool compoundskip) const {
    if (!compoundskip) {
        doIndentation(stream, indentlevel);
    } else {
        stream << ' ';
        indentlevel--;
    }
    stream << "{";
    for (auto& it : components) {
        it->print(stream, indentlevel + 1, false);
    }
    doIndentation(stream, indentlevel);
    stream << "}";
}

void LabelStmt::print(std::ostream& stream, size_t indentlevel, bool) const {
    stream << '\n' << *label << ':';
    stmt->print(stream, indentlevel, false);
}

void ExpressionStmt::print(std::ostream& stream, size_t indentlevel,
                           bool) const {
    doIndentation(stream, indentlevel);
    expr->print(stream);
    stream << ';';
}

void NullStmt::print(std::ostream& stream, size_t indentlevel, bool) const {
    doIndentation(stream, indentlevel);
    stream << ';';
}

void IfStmt::print(std::ostream& stream, size_t indentlevel,
                   bool compoundskip) const {
    if (!compoundskip) {
        doIndentation(stream, indentlevel);
    } else {
        stream << ' ';
        indentlevel--;
    }
    stream << "if (";
    condition->print(stream);
    stream << ")";
    if (dynamic_cast<IfStmt*>(consequence.get())) {
        consequence->print(stream, indentlevel + 1, false);
    } else {
        consequence->print(stream, indentlevel + 1, true);
    }

    if (alternative) {
        if (!dynamic_cast<CompoundStmt*>(consequence.get())) {
            doIndentation(stream, indentlevel);
        } else {
            stream << ' ';
        }
        stream << "else";
        alternative->print(stream, indentlevel + 1, true);
    }
}

void WhileStmt::print(std::ostream& stream, size_t indentlevel, bool) const {
    doIndentation(stream, indentlevel);
    stream << "while (";
    condition->print(stream);
    stream << ")";
    body->print(stream, indentlevel + 1, true);
}

void ReturnStmt::print(std::ostream& stream, size_t indentlevel, bool) const {
    doIndentation(stream, indentlevel);
    stream << "return";
    if (expr) {
        stream << ' ';
        expr->print(stream);
    }
    stream << ';';
}

void BreakContinueStmt::print(std::ostream& stream, size_t indentlevel,
                              bool) const {
    doIndentation(stream, indentlevel);
    if (is_continue) {
        stream << "continue;";
    } else {
        stream << "break;";
    }
}

void GotoStmt::print(std::ostream& stream, size_t indentlevel, bool) const {
    doIndentation(stream, indentlevel);
    stream << "goto " << *label << ';';
}

void Declaration::print(std::ostream& stream, size_t indentlevel, bool) const {
    doIndentation(stream, indentlevel);
    spec_decl->print(stream, indentlevel);
    stream << ';';
}
