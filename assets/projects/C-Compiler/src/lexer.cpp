#include "lexer.h"

#include <assert.h>

#include <fstream>
#include <sstream>

namespace {
bool isNum(int c) { return '0' <= c && c <= '9'; }

bool isInitialIdChar(int c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

bool isIdChar(int c) { return isInitialIdChar(c) || isNum(c); }
}  // namespace

Symbol Lexer::freshSymbol(void) {
    static long i = 0;

    std::stringstream ss;
    ss << "__internal_" << i;
    auto str = ss.str();
    auto res = Strings.internalize(str);
    ++i;
    return res;
}

Lexer::Lexer(std::ifstream& f, const std::string& file_name)
    : KeyWordIdMap(256),
      TokenLocation(file_name, 1, 0),
      CurrentLocation(file_name, 1, 0),
      CurrentToken(TokenLocation, TK_ERROR, &CurrentText),
      PrevToken(TokenLocation, TK_ERROR, &CurrentText),
      File(f) {
#define KEYWORD_ACTION(KND, STR) KeyWordIdMap.emplace(STR, KND);
#include "keywords.def"

#undef KEYWORD_ACTION

    popChar();
    pop();
}

void Lexer::setToken(TokenKind kind) {
    CurrentToken = Token(TokenLocation, kind, Strings.internalize(CurrentText));
}

void Lexer::setToken(TokenKind kind, Symbol text) {
    CurrentToken = Token(TokenLocation, kind, text);
}

#define ACCEPT(CHR, KND)     \
    if (peekChar() == CHR) { \
        addAndPopChar();     \
        setToken(KND);       \
        return;              \
    }

#define ONE_CHAR(CHR, KND) \
    case CHR:              \
        addAndPopChar();   \
        setToken(KND);     \
        return;

#define ONE_OR_TWO_CHAR(CHR1, KND1, CHR2, KND2) \
    case CHR1:                                  \
        addAndPopChar();                        \
        ACCEPT(CHR2, KND2);                     \
        setToken(KND1);                         \
        return;

#define ONE_OR_TWO_OR_TWO_CHAR(CHR1, KND1, CHR2, KND2, CHR3, KND3) \
    case CHR1:                                                     \
        addAndPopChar();                                           \
        ACCEPT(CHR2, KND2);                                        \
        ACCEPT(CHR3, KND3);                                        \
        setToken(KND1);                                            \
        return;

void Lexer::pop(void) {
    if (hasPrev) {
        hasPrev = false;
        return;
    }
    if (CurrentToken.Kind == TK_EOI) {
        return;
    }

    while (true) {
        CurrentText.clear();
        TokenLocation = CurrentLocation;

        switch (peekChar()) {
            case ' ':
            case '\t':
            case '\v':
            case '\r':
            case '\n':
                popChar();
                continue;

            case EOF:
                setToken(TK_EOI, Strings.internalize("EOF"));
                return;

                ONE_CHAR(',', TK_COMMA);
                ONE_CHAR(';', TK_SEMICOLON);
                ONE_CHAR('?', TK_QUESTION);
                ONE_CHAR('~', TK_TILDE);
                ONE_CHAR('(', TK_LPAREN);
                ONE_CHAR(')', TK_RPAREN);
                ONE_CHAR('[', TK_LBRACKET);
                ONE_CHAR(']', TK_RBRACKET);
                ONE_CHAR('{', TK_LBRACE);
                ONE_CHAR('}', TK_RBRACE);

                ONE_OR_TWO_CHAR('=', TK_EQUALS, '=', TK_EQUALSEQUALS);
                ONE_OR_TWO_CHAR('!', TK_BANG, '=', TK_BANGEQUALS);
                ONE_OR_TWO_CHAR('*', TK_ASTERISK, '=', TK_ASTERISKEQUALS);
                ONE_OR_TWO_CHAR('^', TK_CARET, '=', TK_CARETEQUALS);
                ONE_OR_TWO_CHAR('#', TK_HASH, '#', TK_HASHHASH);
                ONE_OR_TWO_CHAR(':', TK_COLON, '>', TK_RBRACKET);

                ONE_OR_TWO_OR_TWO_CHAR('+', TK_PLUS, '+', TK_PLUSPLUS, '=',
                                       TK_PLUSEQUALS);
                ONE_OR_TWO_OR_TWO_CHAR('&', TK_AND, '=', TK_ANDEQUALS, '&',
                                       TK_ANDAND);
                ONE_OR_TWO_OR_TWO_CHAR('|', TK_PIPE, '=', TK_PIPEEQUALS, '|',
                                       TK_PIPEPIPE);

            case '-':
                addAndPopChar();
                ACCEPT('-', TK_MINUSMINUS);
                ACCEPT('=', TK_MINUSEQUALS);
                ACCEPT('>', TK_ARROW);
                setToken(TK_MINUS);
                return;

            case '<':
                addAndPopChar();
                ACCEPT('=', TK_LANGLEEQUALS);
                ACCEPT(':', TK_LBRACKET);
                ACCEPT('%', TK_LBRACE);
                if (peekChar() == '<') {
                    addAndPopChar();
                    ACCEPT('=', TK_LANGLELANGLEEQUALS);
                    setToken(TK_LANGLELANGLE);
                    return;
                }
                setToken(TK_LANGLE);
                return;

            case '>':
                addAndPopChar();
                ACCEPT('=', TK_RANGLEEQUALS);
                if (peekChar() == '>') {
                    addAndPopChar();
                    ACCEPT('=', TK_RANGLERANGLEEQUALS);
                    setToken(TK_RANGLERANGLE);
                    return;
                }
                setToken(TK_RANGLE);
                return;

            case '%':
                addAndPopChar();
                ACCEPT('=', TK_PERCENTEQUALS);
                ACCEPT('>', TK_RBRACE);
                if (peekChar() == ':') {
                    addAndPopChar();
                    if (peekChar() == '%' && checkNextChar(':')) {
                        addAndPopChar();
                        addAndPopChar();
                        setToken(TK_HASHHASH);
                        return;
                    }
                    setToken(TK_HASH);
                    return;
                }
                setToken(TK_PERCENT);
                return;

            case '.':
                addAndPopChar();
                if (peekChar() == '.' && checkNextChar('.')) {
                    addAndPopChar();
                    addAndPopChar();
                    setToken(TK_TRIPLEDOT);
                    return;
                }
                setToken(TK_DOT);
                return;

            case '/':
                addAndPopChar();
                if (peekChar() == '*') {
                    consumeMultiLineComment();
                    continue;
                }
                if (peekChar() == '/') {
                    consumeSingleLineComment();
                    continue;
                }
                ACCEPT('=', TK_SLASHEQUALS);
                setToken(TK_SLASH);
                return;

            case '\'':
                setToken(TK_CHARCONSTANT, lexStringLiteral());
                return;

            case '"':
                setToken(TK_STRINGLITERAL, lexStringLiteral());
                return;

            default:
                int c = peekChar();
                if ('0' <= c && c <= '9') {
                    setToken(TK_NUMCONSTANT, lexNumber());
                    return;
                }
                auto p = lexIdentifierOrKeyword();
                setToken(p.second, p.first);
                return;
        }
    }
}

const Token& Lexer::peek(void) {
    if (hasPrev) {
        return PrevToken;
    }
    return CurrentToken;
}

const Token& Lexer::peekLookAhead(void) {
    if (!hasPrev) {
        PrevToken = CurrentToken;
        pop();
        hasPrev = true;
    }
    return CurrentToken;
}

int Lexer::peekChar(void) const { return CurrentChar; }

bool Lexer::checkNextChar(int c) {
    if (CurrentChar == EOF) {
        return false;
    }

    int nextChar = File.get();
    File.unget();

    return nextChar == c;
}

void Lexer::addAndPopChar(void) {
    CurrentText.push_back(peekChar());
    popChar();
}

void Lexer::popChar(void) {
    assert(CurrentChar != EOF);

    int prev = CurrentChar;
    CurrentChar = File.get();

    if (prev == '\n' || (CurrentChar != '\n' && prev == '\r')) {
        CurrentLocation.Line++;
        CurrentLocation.Column = 0;
    }
    CurrentLocation.Column++;
}

void Lexer::consumeMultiLineComment(void) {
    popChar();  // starting *

    int prev;
    do {
        prev = peekChar();
        if (prev == EOF) {
            errorloc(CurrentLocation,
                     "Reached end of file while lexing a multi-line comment!");
        }
        popChar();
    } while (!(prev == '*' && peekChar() == '/'));
    popChar();
}

void Lexer::consumeSingleLineComment(void) {
    int c;
    do {
        c = peekChar();
        if (c == EOF) {
            errorloc(CurrentLocation,
                     "Reached end of file while lexing a single-line comment!");
        }
        popChar();
    } while (c != '\n');
}

Symbol Lexer::lexNumber(void) {
    if (peekChar() == '0') {
        addAndPopChar();
        if (isIdChar(peekChar())) {
            errorloc(TokenLocation, "Found unsupported octal constant!");
        }
        return Strings.internalize(CurrentText);
    }

    while (isNum(peekChar())) {
        addAndPopChar();
    }
    if (isInitialIdChar(peekChar())) {
        errorloc(TokenLocation, "Found unsupported number suffix!");
    }

    return Strings.internalize(CurrentText);
}

Symbol Lexer::lexStringLiteral(void) {
    int delim = peekChar();
    assert(delim == '"' || delim == '\'');

    addAndPopChar();
    size_t num = 1;

    int c;
    do {
        c = peekChar();
        switch (c) {
            case EOF:
                errorloc(
                    CurrentLocation,
                    "Reached end of file while lexing a string/char literal!");
            case '\n':
                errorloc(
                    CurrentLocation,
                    "Reached end of line while lexing a string/char literal!");

            case '\\':
                addAndPopChar();
                switch (peekChar()) {
                    case '\'':
                    case '"':
                    case '?':
                    case '\\':
                    case 'a':
                    case 'b':
                    case 'f':
                    case 'n':
                    case 'r':
                    case 't':
                    case 'v':
                        break;
                    default:
                        errorloc(CurrentLocation,
                                 "Found invalid escape sequence `\\",
                                 (char)peekChar(), "'!");
                }

            default:;
        }
        addAndPopChar();
        num++;
    } while (c != delim);
    if (delim == '\'' && num != 3) {
        errorloc(TokenLocation, "Found char literal with invalid length!");
    }
    return Strings.internalize(CurrentText);
}

std::pair<Symbol, TokenKind> Lexer::lexIdentifierOrKeyword(void) {
    if (!isInitialIdChar(peekChar())) {
        errorloc(CurrentLocation, "Found invalid char `", (char)peekChar(),
                 "' at beginning of a token!");
    }

    while (isIdChar(peekChar())) {
        addAndPopChar();
    }

    const auto& it = KeyWordIdMap.find(CurrentText);
    if (it != KeyWordIdMap.end()) {
        return std::make_pair(std::addressof(it->first), it->second);
    }

    const auto& p = KeyWordIdMap.emplace(CurrentText, TK_IDENTIFIER);

    assert(p.second);

    return std::make_pair(std::addressof(p.first->first), TK_IDENTIFIER);
}
