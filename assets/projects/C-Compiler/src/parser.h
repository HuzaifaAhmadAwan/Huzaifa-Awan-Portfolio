#pragma once

#include <memory>

#include "ast.h"
#include "lexer.h"

/// This class implements the parser that translates a stream of tokens
/// into an abstract syntax tree.
class Parser {
   public:
    Parser(Lexer& lex);

    std::unique_ptr<TranslationUnit> parse(void);

   private:
    Lexer& lexer;

    /// Advance the parsing state to the next token.
    void popToken(void);

    /// Return (a reference to) the current token.
    const Token& peekToken(void);

    /// Return the location of the current token.
    const Locatable& getLoc(void);

    /// Check if the current token has TokenKind `tk`. If it has, advance the
    /// parsing state to the next token, otherwise note an error that `txt` was
    /// expected but something else encountered.
    void expect(TokenKind tk, const char* txt);

    /// Check if the current token has TokenKind `tk`. If it has, advance the
    /// parsing state to the next token and return true. Otherwise, return
    /// false and keep the parsing state unchanged.
    bool accept(TokenKind tk);

    /// Check if the current token has TokenKind `tk`. If it has, return true.
    /// Otherwise, return false. Keep the parsing state unchanged in both cases.
    bool check(TokenKind tk);

    /// Check if the token after the current token has TokenKind `tk` and
    /// return true iff this is the case. Keep the parsing state unchanged in
    /// both cases.
    bool checkLookAhead(TokenKind tk);

    /// Internal methods for use in parseSpecDecl()
    std::unique_ptr<Declarator> parseDeclarator(void);
    std::unique_ptr<Declarator> parseNonFunDeclarator(void);

    /// Parse a type specifier followed by an abstract or non-abstract
    /// declarator (i.e. a typename or a declaration).
    /// The current token is expected to be the first token of the type
    /// specifier
    ///
    /// If `dKind` is `Declarator::Kind::ABSTRACT`: verify that the declarator
    /// is a valid abstract declarator. If `dKind` is
    /// `Declarator::Kind::CONCRETE`: verify that the declarator is a valid
    /// non-abstract declarator. If `dKind` is `Declarator::Kind::ANY`: do not
    /// verify abstractness.
    std::unique_ptr<SpecDecl> parseSpecDecl(Declarator::Kind dKind);

    /// Statement parsing
    /// parseStatement handles declaration statements and delegates
    /// non-declaration statements to parseNonDeclStatement.
    std::unique_ptr<Statement> parseNonDeclStatement(void);
    std::unique_ptr<Statement> parseStatement(void);

    /// Expression parsing
    std::unique_ptr<Expr> parseExpression(int precedence = 0);

   private:
    /// Helper to get operator precedence
    int getPrecedence(TokenKind tk);

    /// Helper to check if a token is a binary operator
    bool isBinaryOp(TokenKind tk);

    /// Helper to check if a token is a unary operator
    bool isUnaryOp(TokenKind tk);

    /// Helper to check if a token is an assignment operator
    bool isAssignmentOp(TokenKind tk);

    /// Parse primary expressions (literals, identifiers, parenthesized)
    std::unique_ptr<Expr> parsePrimary(void);

    /// Parse postfix expressions (function calls, array subscripts, member access)
    std::unique_ptr<Expr> parsePostfix(std::unique_ptr<Expr>&& expr);
};
