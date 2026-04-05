#pragma once

#include "ast.h"
#include "symbol_internalizer.h"

#include <unordered_map>
#include <utility>

class Scope {
  std::unordered_map<Symbol, std::shared_ptr<SpecDecl>> table;
  std::shared_ptr<Scope> parent;
  WhileStmt* relatedWhile;
  ExternalDeclaration* relatedFunc;
  std::unordered_map<Symbol, LabelStmt*> declaredLabels;
  std::unordered_set<GotoStmt*> pendingGoTos;
  std::unordered_set<Symbol> definedFunctions;
  bool isFuncScope = false;
public:

  Scope(const std::shared_ptr<Scope> &p) : parent(p) {
    if (parent) {
      relatedFunc = parent->relatedFunc;
      relatedWhile = parent->relatedWhile;
    }
  }

  explicit Scope(const std::shared_ptr<Scope> &p, WhileStmt* w) : Scope(p) {
    relatedWhile = w;
  }

  explicit Scope(const std::shared_ptr<Scope> &p, ExternalDeclaration* f) : Scope(p) {
    relatedFunc = f;
    isFuncScope = true;
  }
  
  ~Scope() = default;

  void add(const Symbol name, const std::shared_ptr<SpecDecl> &spec_decl) {
    table[name] = spec_decl;
  }

  void addFuncDefined(const Symbol name) {
    definedFunctions.insert(name);
  }

  bool isFuncDefined(const Symbol name) {
    return definedFunctions.find(name) != definedFunctions.end();
  }

  std::shared_ptr<SpecDecl> lookup(Symbol name) {
    if (table.find(name) != table.end()) {
      return table[name];
    }
    else if (parent != nullptr) {
      return parent->lookup(name);
    }
    return nullptr;
  }

  bool lookupLabel(Symbol name) {
    if (isFuncScope)
      return declaredLabels.find(name) != declaredLabels.end();
    return parent->lookupLabel(name);
  }

  bool contains(const Symbol name) {return table.find(name) != table.end();}

  WhileStmt* getRelatedWhile() const { return relatedWhile;}

  ExternalDeclaration* getRelatedFunc() const { return relatedFunc;}

  void addDeclaredLabel(LabelStmt* labelStmt) {
    if (isFuncScope)
      declaredLabels[labelStmt->label] = labelStmt;
    else
      parent->addDeclaredLabel(labelStmt);
  }

  void addGoTo(GotoStmt* gotoStmt) {
    if (isFuncScope)
      pendingGoTos.insert(gotoStmt);
    else
      parent->addGoTo(gotoStmt);
  }

  void resolveGoTos() {
    for (const auto& record : pendingGoTos) {
      if (declaredLabels.find(record->label) == declaredLabels.end())
        errorloc(*record, "Goto jumps to undefined label '", record->label, "'");
      else {
        record->destination = declaredLabels[record->label];
      }
    }
  }
};
