#include "parser.h"

#include <memory>

#include "ast.h"
#include "diagnostic.h"
#include "token.h"

Parser::Parser(Lexer& lex) : lexer(lex) {}

bool Parser::checkLookAhead(TokenKind tk) {
    return lexer.peekLookAhead().Kind == tk;
}

void Parser::popToken(void) { lexer.pop(); }

const Token& Parser::peekToken(void) { return lexer.peek(); }

const Locatable& Parser::getLoc(void) { return peekToken(); }

void Parser::expect(TokenKind tk, const char* txt) {
    const Token& current = peekToken();

    if (current.Kind != tk) {
        errorloc((Locatable)current, "Expected ", txt, " but got `",
                 *current.Text, "'");
    }
    popToken();
}

bool Parser::accept(TokenKind tk) {
    const Token& current = peekToken();

    if (current.Kind == tk) {
        popToken();
        return true;
    }
    return false;
}

bool Parser::check(TokenKind tk) {
    const Token& current = peekToken();

    if (current.Kind == tk) {
        return true;
    }
    return false;
}


// translation-unit:
//     external-declaration
//     translation-unit external-declaration
//
// external-declaration:
//     function-definition
//     declaration
//
// function-definition:
//     declaration-specifiers declarator declaration-list_opt compound-statement
//
// declaration-list:
//     declaration
//     declaration-list declaration
std::unique_ptr<TranslationUnit> Parser::parse(void) {
    if (check(TK_EOI)) {
        errorloc(getLoc(), "Empty translation unit!");
    }
    
    auto res = std::make_unique<TranslationUnit>();

    //translation-unit external-declaration
    while (!check(TK_EOI)) {
        Locatable loc(getLoc()); 
        //declaration-specifiers declarator
        auto specDecl = parseSpecDecl(Declarator::Kind::CONCRETE); 
        std::unique_ptr<CompoundStmt> body = nullptr;

        //definition-list_opt compound-statement
        if (check(TK_LBRACE)) {
            if (!specDecl->isFunSpecDecl()) { 
                errorloc(getLoc(),
                         "Unexpected `{' after non-function declaration!");
            }
            // we know this to be a compound statement
            body.reset(
                static_cast<CompoundStmt*>(parseNonDeclStatement().release())); 
            // is compound as we start with a brace
            body->is_fun_body = true;
        } else {
            //declaration ;
            expect(TK_SEMICOLON, ";");
        }

        //external-declaration
        res->addExternalDeclaration(std::make_unique<ExternalDeclaration>(
            loc, std::move(specDecl), std::move(body)));
    }
    return res;
}

// declaration:
//     declaration-specifiers init-declarator-list_opt ;
//
// declaration-specifiers:
//     storage-class-specifier declaration-specifiers_opt
//     type-specifier declaration-specifiers_opt
//     type-qualifier declaration-specifiers_opt
//     function-specifier declaration-specifiers_opt
//     alignment-specifier declaration-specifiers_opt
//
// init-declarator-list:
//     init-declarator
//     init-declarator-list , init-declarator
//
// init-declarator:
//     declarator
//     declarator = initializer
//
// type-specifier:
//     void
//     char
//     int
//     struct-or-union-specifier
//
// struct-or-union-specifier:
//     struct-or-union identifier_opt { struct-declaration-list }
//     struct-or-union identifier
//
// struct-or-union:
//     struct
//     union
//
// struct-declaration-list:
//     struct-declaration
//     struct-declaration-list struct-declaration
//
// struct-declaration:
//     specifier-qualifier-list struct-declarator-list_opt ;
//
// specifier-qualifier-list:
//     type-specifier specifier-qualifier-list_opt
//     type-qualifier specifier-qualifier-list_opt
//
// struct-declarator-list:
//     struct-declarator
//     struct-declarator-list , struct-declarator
//
// struct-declarator:
//     declarator
//
// type-name:
//     specifier-qualifier-list abstract-declarator_opt
//
// abstract-declarator:
//     pointer
//     pointer_opt direct-abstract-declarator
//
// direct-abstract-declarator:
//     ( abstract-declarator )
//     direct-abstract-declarator_opt [ type-qualifier-list_opt assignment-expression_opt ]
//     direct-abstract-declarator_opt ( parameter-type-list_opt )
//
// parseSpecDecl -> (void|char|int|struct) parseDeclarator
std::unique_ptr<SpecDecl> Parser::parseSpecDecl(Declarator::Kind dKind) {
    std::unique_ptr<Specifier> spec = nullptr;
    Locatable loc(getLoc());
    //type-specifier: void
    if (accept(TK_VOID)) {
        spec = std::make_unique<VoidSpecifier>(loc);
    //type-specifier: char
    } else if (accept(TK_CHAR)) {
        spec = std::make_unique<CharSpecifier>(loc);
    //type-specifier: int
    } else if (accept(TK_INT)) {
        spec = std::make_unique<IntSpecifier>(loc);
    //type-specifier: struct-or-union-specifier
    } else if (accept(TK_STRUCT)) {
        Symbol tag = nullptr; 
        //struct-or-union identifier
        if (check(TK_IDENTIFIER)) { 
            tag = peekToken().Text;
            popToken();
        } else if (!check(TK_LBRACE)) { 
            errorloc(getLoc(), "Expected struct declaration list but got `",
                     *peekToken().Text, "'");
            return std::make_unique<SpecDecl>(std::make_unique<StructSpecifier>(loc, nullptr), 
                                             std::make_unique<PrimitiveDeclarator>(loc, nullptr), dKind);
        }
        auto structSpec = std::make_unique<StructSpecifier>(loc, tag);

        //struct-or-union identifieropt { struct-declaration-list }
        if (accept(TK_LBRACE)) {
            //struct-declaration-list struct-declaration
            do {  // parse struct member declarations
                //specifier-qualifier-list struct-declarator-list_opt ;
                auto specDecl = parseSpecDecl(Declarator::Kind::CONCRETE);
                structSpec->addComponent(std::move(specDecl));
                expect(TK_SEMICOLON, ";");
            } while (!accept(TK_RBRACE));
        }
        spec = std::move(structSpec);

    } else {
        errorloc(loc, "Expected type specifier but got `", *peekToken().Text,
                 "'");
    }

    //declarator
    auto decl = parseDeclarator();

    return std::make_unique<SpecDecl>(std::move(spec), std::move(decl), dKind);
}


// declarator:
//     pointer_opt direct-declarator
//
// direct-declarator:
//     identifier
//     ( declarator )
//     direct-declarator [ type-qualifier-list_opt assignment-expression_opt ]
//     direct-declarator ( parameter-type-list )
//     direct-declarator ( identifier-list_opt )
//
// pointer:
//     * type-qualifier-list_opt
//     * type-qualifier-list_opt pointer
//
// parseNonFunDeclarator -> '('')'| '('(void|char|int|struct) parseDeclarator')' | * parseDeclarator | identifier | ε
std::unique_ptr<Declarator> Parser::parseNonFunDeclarator(void) {
    switch (peekToken().Kind) {
        case TK_LPAREN: {
            if (checkLookAhead(TK_RPAREN) || checkLookAhead(TK_VOID) ||
                checkLookAhead(TK_CHAR) || checkLookAhead(TK_INT) ||
                checkLookAhead(TK_STRUCT)) {
                // this has to be an abstract function declarator, not a
                // parenthesized declarator, so we add an empty primitive
                // declarator
                return std::make_unique<PrimitiveDeclarator>(getLoc(), nullptr);
            }
            //direct-declarator: ( declarator )
            popToken();
            auto res = parseDeclarator();
            expect(TK_RPAREN, ")");
            return res;
        }

        //pointer: * type-qualifier-list_opt pointer
        case TK_ASTERISK: {
            Locatable loc(getLoc());
            popToken();
            auto inner = parseDeclarator();
            return std::make_unique<PointerDeclarator>(loc, std::move(inner));
        }

        //direct-declarator: identifier
        case TK_IDENTIFIER: {
            auto res = std::make_unique<PrimitiveDeclarator>(getLoc(),
                                                             peekToken().Text);
            popToken();
            return res;
        }
        //abstract-declarator (empty)
        default:
            return std::make_unique<PrimitiveDeclarator>(getLoc(), nullptr);
    }
}



// direct-declarator:
//     identifier
//     ( declarator )
//     direct-declarator [ type-qualifier-list_opt assignment-expression_opt ]
//     direct-declarator ( parameter-type-list )
//     direct-declarator ( identifier-list_opt )
//
// parameter-type-list:
//     parameter-list
//     parameter-list , ...
//
// parameter-list:
//     parameter-declaration
//     parameter-list , parameter-declaration
//
// parameter-declaration:
//     declaration-specifiers declarator
//     declaration-specifiers abstract-declarator_opt
//
// identifier-list:
//     identifier
//     identifier-list , identifier
//
// parseDeclarator -> parseNonFunDeclarator ( '(' parseSpecDecl {',' parseSpecDecl} ')' )*
std::unique_ptr<Declarator> Parser::parseDeclarator(void) {
    auto res = parseNonFunDeclarator();
    //direct-declarator ( parameter-type-list )
    while (check(TK_LPAREN)) {
        res = std::make_unique<FunctionDeclarator>(getLoc(), std::move(res));
        popToken();

        if (accept(TK_RPAREN)) {
            continue; 
        }

        //parameter-list, parameter-declaration
        do {
            //declaration-specifiers declarator/abstract-declarator_opt
            auto param = parseSpecDecl(Declarator::Kind::ANY);
            static_cast<FunctionDeclarator*>(res.get())->addParameter(
                std::move(param));
        } while (accept(TK_COMMA));

        expect(TK_RPAREN, ")");
    }
    return res;
}

// statement:
//     labeled-statement
//     compound-statement
//     expression-statement
//     selection-statement
//     iteration-statement
//     jump-statement
//
// labeled-statement:
//     identifier : statement
//     case constant-expression : statement
//     default : statement
//
// compound-statement:
//     { block-item-list_opt }
//
// block-item-list:
//     block-item
//     block-item-list block-item
//
// block-item:
//     declaration
//     statement
//
// expression-statement:
//     expression_opt ;
//
// selection-statement:
//     if ( expression ) statement
//     if ( expression ) statement else statement
//     switch ( expression ) statement
//
// iteration-statement:
//     while ( expression ) statement
//     do statement while ( expression ) ;
//     for ( expression_opt ; expression_opt ; expression_opt ) statement
//     for ( declaration expression_opt ; expression_opt ) statement
//
// jump-statement:
//     goto identifier ;
//     continue ;
//     break ;
//     return expression_opt ;
//
// parseNonDeclStatement -> '{' {parseStatement} '}' | identifier ':' parseNonDeclStatement | ';' | 'if' '(' parseExpression ')' parseNonDeclStatement [ 'else' parseNonDeclStatement ] | 'while' '(' parseExpression ')' parseNonDeclStatement | 'return' [parseExpression] ';' | 'break' ';' | 'continue' ';' | 'goto' identifier ';' | parseExpression ';'
std::unique_ptr<Statement> Parser::parseNonDeclStatement(void) {
    std::unique_ptr<Statement> res{};
    switch (peekToken().Kind) {
        //compound-statement: { block-item-list_opt }
        case TK_LBRACE: {
            auto compstmt = std::make_unique<CompoundStmt>(getLoc());
            popToken();
            //block-item-list: block-item
            while (!accept(TK_RBRACE)) {
                //block-item: declaration | statement
                compstmt->addComponent(parseStatement());
            }
            res = std::move(compstmt);
            break;
        }

        case TK_IDENTIFIER:
            //labeled-statement: identifier : statement
            if (checkLookAhead(TK_COLON)) {
                auto label = peekToken().Text;
                popToken();
                Locatable loc(getLoc());
                popToken();
                auto stmt = parseNonDeclStatement();
                res = std::make_unique<LabelStmt>(loc, std::move(stmt), label);
            } else {
                goto parse_expr; 
            }
            break;

        //expression-statement: expression_opt ;
        case TK_SEMICOLON:
            res = std::make_unique<NullStmt>(getLoc());
            popToken();
            break;

        //selection-statement: if ( expression ) statement | if ( expression ) statement else statement
        case TK_IF: {
            Locatable loc(getLoc());
            popToken();
            expect(TK_LPAREN, "(");
            auto cond = parseExpression();
            expect(TK_RPAREN, ")");
            auto cons = parseNonDeclStatement();
            std::unique_ptr<Statement> alt{};

            if (accept(TK_ELSE)) {
                alt = parseNonDeclStatement();
            }

            res = std::make_unique<IfStmt>(loc, std::move(cond),
                                           std::move(cons), std::move(alt));
            break;
        }

        //iteration-statement: while ( expression ) statement
        case TK_WHILE: {
            Locatable loc(getLoc());
            popToken();
            expect(TK_LPAREN, "(");
            auto cond = parseExpression();
            expect(TK_RPAREN, ")");
            auto body = parseNonDeclStatement();

            res = std::make_unique<WhileStmt>(loc, std::move(cond),
                                              std::move(body));
            break;
        }

        //jump-statement: return expression_opt ;
        case TK_RETURN: {
            Locatable loc(getLoc());
            popToken();

            if (accept(TK_SEMICOLON)) {
                res = std::make_unique<ReturnStmt>(loc, nullptr);
                break;
            }
            auto expr = parseExpression();
            expect(TK_SEMICOLON, ";");
            res = std::make_unique<ReturnStmt>(loc, std::move(expr));
            break;
        }

        //jump-statement: break ;
        case TK_BREAK:
            res = std::make_unique<BreakContinueStmt>(getLoc(), false);
            popToken();
            expect(TK_SEMICOLON, ";");
            break;

        //jump-statement: continue ;
        case TK_CONTINUE:
            res = std::make_unique<BreakContinueStmt>(getLoc(), true);
            popToken();
            expect(TK_SEMICOLON, ";");
            break;

        //jump-statement: goto identifier ;
        case TK_GOTO: {
            Locatable loc(getLoc());
            popToken();

            if (!check(TK_IDENTIFIER)) {
                errorloc(getLoc(), "Expected label but got `",
                         *peekToken().Text, "'");
                break;  // Stop parsing on error
            }

            res = std::make_unique<GotoStmt>(loc, peekToken().Text);
            popToken();

            expect(TK_SEMICOLON, ";");
            break;
        }

        default: {
        parse_expr:
            //expression-statement: expression ;
            auto expr = parseExpression();
            res = std::make_unique<ExpressionStmt>(getLoc(), std::move(expr));
            expect(TK_SEMICOLON, ";");
        }
    }
    return res;
}

// statement:
//     labeled-statement
//     compound-statement
//     expression-statement
//     selection-statement
//     iteration-statement
//     jump-statement
//
// block-item:
//     declaration
//     statement
//
// declaration:
//     declaration-specifiers init-declarator-list_opt ;
//
// parseStatement -> parseSpecDecl ';' | parseNonDeclStatement
std::unique_ptr<Statement> Parser::parseStatement(void) {
    switch (peekToken().Kind) {
        //block-item: declaration
        //declaration: declaration-specifiers init-declarator-list_opt ;
        case TK_VOID:
        case TK_CHAR:
        case TK_INT:
        case TK_STRUCT: {
            Locatable loc(getLoc());
            auto specDecl = parseSpecDecl(Declarator::Kind::CONCRETE);
            expect(TK_SEMICOLON, ";");
            return std::make_unique<Declaration>(loc, std::move(specDecl));
        }
        //block-item: statement
        default:
            return parseNonDeclStatement();
    }
}

namespace {

#define UNARY_PREC 196  // higher than any binary operator

#define EXPRESSION_PREC 0 // lowest precedence for expressions

// Operator precedence 
int getPrecedenceHelper(TokenKind tk) {
    switch (tk) {
        case TK_EQUALS:
        case TK_PLUSEQUALS:
        case TK_MINUSEQUALS:
        case TK_ASTERISKEQUALS:
        case TK_SLASHEQUALS:
        case TK_PERCENTEQUALS:
        case TK_ANDEQUALS:
        case TK_PIPEEQUALS:
        case TK_CARETEQUALS:
        case TK_LANGLELANGLEEQUALS:
        case TK_RANGLERANGLEEQUALS:
            return 2;  // Assignment (right-associative)
        case TK_QUESTION:
            return 3;  // Ternary (right-associative)
        case TK_PIPEPIPE:
            return 4;
        case TK_ANDAND:
            return 5;
        case TK_PIPE:
            return 6;
        case TK_CARET:
            return 7; 
        case TK_AND:
            return 8;
        case TK_EQUALSEQUALS: 
        case TK_BANGEQUALS:
            return 9;
        case TK_LANGLE: 
        case TK_RANGLE: 
        case TK_LANGLEEQUALS:
        case TK_RANGLEEQUALS:
            return 10;
        case TK_LANGLELANGLE:
        case TK_RANGLERANGLE:
            return 11;
        case TK_PLUS:
        case TK_MINUS:
            return 12;
        case TK_ASTERISK:
        case TK_SLASH:
        case TK_PERCENT:
            return 13;
        default:
            return 0;
    }
}

bool isBinaryOpHelper(TokenKind tk) {
    return tk == TK_PLUS || tk == TK_MINUS || tk == TK_ASTERISK ||
           tk == TK_SLASH || tk == TK_PERCENT || tk == TK_EQUALSEQUALS ||
           tk == TK_BANGEQUALS || tk == TK_LANGLE || tk == TK_RANGLE ||
           tk == TK_LANGLEEQUALS || tk == TK_RANGLEEQUALS || tk == TK_ANDAND ||
           tk == TK_PIPEPIPE || tk == TK_AND || tk == TK_PIPE ||
           tk == TK_CARET || tk == TK_LANGLELANGLE || tk == TK_RANGLERANGLE;
}

bool isAssignmentOpHelper(TokenKind tk) {
    return tk == TK_EQUALS || tk == TK_PLUSEQUALS || tk == TK_MINUSEQUALS ||
           tk == TK_ASTERISKEQUALS || tk == TK_SLASHEQUALS ||
           tk == TK_PERCENTEQUALS || tk == TK_ANDEQUALS || tk == TK_PIPEEQUALS ||
           tk == TK_CARETEQUALS || tk == TK_LANGLELANGLEEQUALS ||
           tk == TK_RANGLERANGLEEQUALS;
}

bool isUnaryOpHelper(TokenKind tk) {
    return tk == TK_BANG || tk == TK_MINUS || tk == TK_PLUS || tk == TK_TILDE ||
           tk == TK_AND || tk == TK_ASTERISK || tk == TK_PLUSPLUS || tk == TK_MINUSMINUS;
}

}  // namespace

int Parser::getPrecedence(TokenKind tk) { return getPrecedenceHelper(tk); }

bool Parser::isBinaryOp(TokenKind tk) { return isBinaryOpHelper(tk); }

bool Parser::isUnaryOp(TokenKind tk) { return isUnaryOpHelper(tk); }

bool Parser::isAssignmentOp(TokenKind tk) { return isAssignmentOpHelper(tk); }


// primary-expression:
//     identifier
//     constant
//     string-literal
//     ( expression )
//
// unary-expression:
//     postfix-expression
//     ++ unary-expression
//     -- unary-expression
//     unary-operator cast-expression
//     sizeof unary-expression
//     sizeof ( type-name )
// unary-operator: one of
//     & * + - ~ !
//
// cast-expression:
//     unary-expression
//     ( type-name ) cast-expression
//
// parsePrimary -> sizeof'('(parseSpecDecl|parseExpression)')' |sizeof parseExpression |integer-literal | char-literal | string-literal | identifier | '(' parseExpression ')' | '(' parseSpecDecl ')' parseExpression | parseExpression
std::unique_ptr<Expr> Parser::parsePrimary(void) {
    const Token& current = peekToken();
    Locatable loc(getLoc());

    //unary-expression: sizeof ( type-name )
    //unary-expression: sizeof unary-expression
    if (current.Kind == TK_SIZEOF) {
        popToken();
        if (check(TK_LPAREN)) {
            // Peek ahead to see if this is sizeof(type) or sizeof(expr)
            const Token& next = lexer.peekLookAhead();
            if (next.Kind == TK_INT || next.Kind == TK_CHAR || 
                next.Kind == TK_VOID || next.Kind == TK_STRUCT) {
                // It's sizeof(type) - consume the paren and parse as a type
                accept(TK_LPAREN);
                auto type = parseSpecDecl(Declarator::Kind::ABSTRACT);
                expect(TK_RPAREN, ")");
                return std::make_unique<SizeOf>(loc, std::move(type));
            }
            // Otherwise, treat it as sizeof applied to an expression
            // The parentheses are part of the expression itself (like sizeof (g)(23))
            // Parse with unary precedence to get the full postfix expression
        }
        // sizeof with or without parens on expression - parse with unary precedence
        auto expr = parseExpression(14); // Precedence 14 = unary operators
        return std::make_unique<SizeOf>(loc, std::move(expr));
    }

    //primary-expression: constant (integer)
    if (current.Kind == TK_NUMCONSTANT) {
        long value = std::stol(*current.Text);
        popToken();
        return std::make_unique<IntLiteral>(loc, value);
    }

    //primary-expression: constant (character)
    if (current.Kind == TK_CHARCONSTANT) {
        auto text = current.Text;
        popToken();
        return std::make_unique<CharLiteral>(loc, text);
    }

    //primary-expression: string-literal
    if (current.Kind == TK_STRINGLITERAL) {
        auto text = current.Text;
        popToken();
        return std::make_unique<StringLiteral>(loc, text);
    }

    //primary-expression: identifier
    if (current.Kind == TK_IDENTIFIER) {
        auto name = current.Text;
        popToken();
        return std::make_unique<Identifier>(loc, name);
    }

    //primary-expression: ( expression )
    //cast-expression: ( type-name ) cast-expression
    if (accept(TK_LPAREN)) {
        // Check if this is a cast (type specifier at start)
        const Token& next = peekToken();
        bool is_cast = false;
        
        // A cast looks like: (type-specifier [tag/const] pointer-part?)
        if (next.Kind == TK_VOID || next.Kind == TK_CHAR || 
            next.Kind == TK_INT || next.Kind == TK_STRUCT) {
            is_cast = true; // Assume it's a cast if it starts with a type keyword
        }
        
        if (is_cast) {
            // Parse as cast
            auto type = parseSpecDecl(Declarator::Kind::ABSTRACT);
            expect(TK_RPAREN, ")");
            auto operand = parseExpression(UNARY_PREC);
            return std::make_unique<Cast>(loc, std::move(type), std::move(operand));
        } else {
            // Parse as parenthesized expression
            auto expr = parseExpression();
            expect(TK_RPAREN, ")");
            return expr;
        }
    }

    //unary-expression: unary-operator cast-expression
    //unary-operator: one of & * + - ~ !
    if (isUnaryOp(current.Kind)) {
        TokenKind op = current.Kind;
        popToken();
        auto operand = parseExpression(UNARY_PREC);
        return std::make_unique<UnaryOp>(loc, op, std::move(operand));
    }

    errorloc(loc, "Expected expression but got `", *current.Text, "'");
}

// postfix-expression:
//     primary-expression
//     postfix-expression [ expression ]
//     postfix-expression ( argument-expression-list_opt )
//     postfix-expression . identifier
//     postfix-expression -> identifier
//     postfix-expression ++
//     postfix-expression --
//     ( type-name ) { initializer-list }
//     ( type-name ) { initializer-list , }
//
// argument-expression-list:
//     assignment-expression
//     argument-expression-list , assignment-expression
//
// parsePostfix -> ( '(' {parseExpression {',' parseExpression}} ')' | '[' parseExpression ']' | '.' identifier | '->' identifier )*
std::unique_ptr<Expr> Parser::parsePostfix(std::unique_ptr<Expr>&& expr) {
    while (true) {
        const Token& current = peekToken();
        Locatable loc(getLoc());

        //postfix-expression ( argument-expression-list_opt )
        if (current.Kind == TK_LPAREN) {
            popToken();
            auto call = std::make_unique<FunctionCall>(loc, std::move(expr));

            if (!check(TK_RPAREN)) {
                //argument-expression-list , assignment-expression
                do {
                    call->addArgument(parseExpression());
                } while (accept(TK_COMMA));
            }

            expect(TK_RPAREN, ")");
            expr = std::move(call);
        //postfix-expression [ expression ]
        } else if (current.Kind == TK_LBRACKET) {
            popToken();
            auto index = parseExpression();
            expect(TK_RBRACKET, "]");
            expr = std::make_unique<Index>(loc, std::move(expr), std::move(index));
        //postfix-expression . identifier
        } else if (current.Kind == TK_DOT) {
            popToken();
            if (!check(TK_IDENTIFIER)) {
                errorloc(getLoc(), "Expected identifier after '.'");
                break;  // Stop parsing postfix operations on error
            }
            auto member = peekToken().Text;
            popToken();
            expr = std::make_unique<Member>(loc, std::move(expr), false, member);
        //postfix-expression -> identifier
        } else if (current.Kind == TK_ARROW) {
            popToken();
            if (!check(TK_IDENTIFIER)) {
                errorloc(getLoc(), "Expected identifier after '->'");
                break;  // Stop parsing postfix operations on error
            }
            auto member = peekToken().Text;
            popToken();
            expr = std::make_unique<Member>(loc, std::move(expr), true, member);
        } else {
            break;
        }
    }
    return std::move(expr);
}

// multiplicative-expression:
//     cast-expression
//     multiplicative-expression * cast-expression
//     multiplicative-expression / cast-expression
//     multiplicative-expression % cast-expression
//
// additive-expression:
//     multiplicative-expression
//     additive-expression + multiplicative-expression
//     additive-expression - multiplicative-expression
//
// shift-expression:
//     additive-expression
//     shift-expression << additive-expression
//     shift-expression >> additive-expression
//
// relational-expression:
//     shift-expression
//     relational-expression < shift-expression
//     relational-expression > shift-expression
//     relational-expression <= shift-expression
//     relational-expression >= shift-expression
//
// equality-expression:
//     relational-expression
//     equality-expression == relational-expression
//     equality-expression != relational-expression
//
// AND-expression:
//     equality-expression
//     AND-expression & equality-expression
//
// exclusive-OR-expression:
//     AND-expression
//     exclusive-OR-expression ^ AND-expression
//
// inclusive-OR-expression:
//     exclusive-OR-expression
//     inclusive-OR-expression | exclusive-OR-expression
//
// logical-AND-expression:
//     inclusive-OR-expression
//     logical-AND-expression && inclusive-OR-expression
//
// logical-OR-expression:
//     logical-AND-expression
//     logical-OR-expression || logical-AND-expression
//
// conditional-expression:
//     logical-OR-expression
//     logical-OR-expression ? expression : conditional-expression
//
// assignment-expression:
//     conditional-expression
//     unary-expression assignment-operator assignment-expression
// assignment-operator: one of
//     = *= /= %= += -= <<= >>= &= ^= |=
//
// expression:
//     assignment-expression
//     expression , assignment-expression
//
// constant-expression:
//     conditional-expression
//
// parseExpression -> parsePrimary parsePostfix { (binary-operator | '?' parseExpression ':' | assignment-operator) parseExpression }
std::unique_ptr<Expr> Parser::parseExpression(int precedence) {
    // Parse primary or unary expression
    auto expr = parsePrimary();

    // Parse postfix operations
    expr = parsePostfix(std::move(expr));

    // Precedence climbing for binary and ternary operators
    while (true) {
        const Token& current = peekToken();
        Locatable loc(getLoc());

        //conditional-expression: logical-OR-expression ? expression : conditional-expression
        if (current.Kind == TK_QUESTION && precedence <= 3) {
            popToken();
            auto consequent = parseExpression(0);
            expect(TK_COLON, ":");
            auto alternative = parseExpression(3);
            expr = std::make_unique<TernaryOp>(loc, std::move(expr),
                                               std::move(consequent),
                                               std::move(alternative));
            continue;
        }

        //assignment-expression: unary-expression assignment-operator assignment-expression
        //assignment-operator: = *= /= %= += -= <<= >>= &= ^= |=
        if (isAssignmentOp(current.Kind) && precedence <= 2) {
            TokenKind op = current.Kind;
            popToken();
            auto rhs = parseExpression(2);
            expr = std::make_unique<Assignment>(loc, std::move(expr), op,
                                               std::move(rhs));
            continue;
        }

        // Binary operators (multiplicative, additive, shift, relational, 
        // equality, AND, exclusive-OR, inclusive-OR, logical-AND, logical-OR)
        if (isBinaryOp(current.Kind)) {
            int prec = getPrecedence(current.Kind);
            if (prec < precedence) {
                break;
            }

            TokenKind op = current.Kind;
            popToken();
            auto rhs = parseExpression(prec + 1);
            expr = std::make_unique<BinaryOp>(loc, std::move(expr), op,
                                             std::move(rhs));
            continue;
        }

        break;
    }

    return expr;
}
