#pragma once

#include <fstream>
#include <unordered_map>

#include "symbol_internalizer.h"
#include "token.h"

class Lexer {
   public:
    void pop(void);
    const Token &peek(void);
    const Token &peekLookAhead(void);

    Lexer(std::ifstream &f, const std::string &file_name);

    Symbol freshSymbol(void);

   private:
    SymbolInternalizer Strings;
    std::unordered_map<std::string, TokenKind> KeyWordIdMap;
    Locatable TokenLocation;
    Locatable CurrentLocation;
    std::string CurrentText = "";
    Token CurrentToken;
    Token PrevToken;
    bool hasPrev = false;

    int CurrentChar = 0;

    std::ifstream &File;

    int peekChar(void) const;
    void popChar(void);
    bool checkNextChar(int c);
    void addAndPopChar(void);
    void setToken(TokenKind kind);
    void setToken(TokenKind kind, Symbol text);

    void consumeMultiLineComment(void);
    void consumeSingleLineComment(void);
    Symbol lexNumber(void);
    Symbol lexStringLiteral(void);
    std::pair<Symbol, TokenKind> lexIdentifierOrKeyword(void);
};
