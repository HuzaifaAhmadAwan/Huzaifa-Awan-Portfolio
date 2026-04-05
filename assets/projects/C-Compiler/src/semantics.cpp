#include "ast.h"
#include "scope.h"
#include "c4type.h"
#include <unordered_set>

////////////////////////////////////////////////////////////////////////////////
//                             Global Elements                                //
////////////////////////////////////////////////////////////////////////////////

void ExternalDeclaration::checkSemantics(const std::shared_ptr<Scope> &scope) {
  // Check if this declares a struct tag BEFORE checking semantics
  if (auto* structSpec = dynamic_cast<StructSpecifier*>(spec_decl->specifier.get())) {
    if (structSpec->tag && !structSpec->components.empty()) {
      // This is a struct definition - register the tag
      if (scope->lookup(structSpec->tag))
        errorloc(*this, "Redefinition of struct tag '", *structSpec->tag, "'");
      
      // Check for duplicate fields in struct
      std::unordered_set<Symbol> fieldNames;
      for (const auto& component : structSpec->components) {
        Symbol fieldName = component->declarator->getInnermost().getName();
        if (fieldName) {
          if (fieldNames.count(fieldName)) {
            errorloc(*this, "Duplicate field '", *fieldName, "' in struct");
          }
          fieldNames.insert(fieldName);
        }
      }
      
      // Create a copy of the spec_decl to store for the struct tag
      scope->add(structSpec->tag, spec_decl->copy());
    }
  }
  
  c4type = spec_decl->checkSemantics();
  Symbol name = spec_decl->declarator->getInnermost().getName();

  // Reject declarations without declarator (e.g., "int;")
  // BUT allow forward declarations and struct definitions without declarators (e.g. "struct S;", "struct S { int a; };")
  if (!name) {
    bool isStructDef = false;
    if (auto* structSpec = dynamic_cast<StructSpecifier*>(spec_decl->specifier.get())) {
      if (structSpec->tag) {
        isStructDef = true;  // This is a struct definition, which is allowed without declarator
      }
    }
    if (!isStructDef) {
      errorloc(*this, "Declaration without declarator");
    }
  }

  // Check redeclaration
  if (scope->contains(name)) {
    auto oldDecl = scope->lookup(name);
    auto oldType = oldDecl->checkSemantics();
    
    // Allow redeclaration if both are functions and types match
    if (oldType->isFunction() && c4type->isFunction()) {
      if (oldType->equals(*c4type)) {
        // check for multiple definition
        if (body && scope->isFuncDefined(name))
          errorloc(*this, "Duplicate definition of function '", *name, "'");
      } else {
        errorloc(*this, "Redeclaration of '", *name, "'");
      }
    }
    else
      errorloc(*this, "Redeclaration of '", *name, "'");
  }
  
  // Add to scope BEFORE processing the function body
  // This allows recursive calls and forward references to work
  if (name) {
    scope->add(name, spec_decl->copy());
  }
  
  // Function Definition?
  if (body) {
    scope->addFuncDefined(name);
    if (!c4type->isFunction()) {
      errorloc(*this, "Definition provided for non-function");
    }
    auto ft = std::static_pointer_cast<C4FunctionType>(c4type);
    if (ft->getReturnType()->isStruct()) {
      errorloc(*this, "Function cannot return a struct");
    }

    auto funcScope = std::make_shared<Scope>(scope, this);

    const FunctionDeclarator* fd = spec_decl->declarator->getInnermostFunDecl();
    const auto& pTypes = ft->getParams();

    // Check if we have a void parameter (which means no parameters)
    bool hasVoidParam = false;
    if (fd->params.size() == 1) {
      auto paramType = fd->params[0]->checkSemantics();
      if (paramType->isVoid() && !fd->params[0]->declarator->getInnermost().getName()) {
        hasVoidParam = true;
      }
    }

    if (hasVoidParam) {
      // void parameter means no actual parameters
      if (pTypes.size() != 0) {
        errorloc(*this, "Function declaration must have the same number of arguments");
      }
      // Don't add the void parameter to the scope
    } else {
      if (fd->params.size() != pTypes.size()) {
        errorloc(*this, "Function declaration must have the same number of arguments");
      }
      // Add actual parameters to the scope
      for (size_t i = 0; i < fd->params.size(); ++i) {
        Symbol pName = fd->params[i]->declarator->getInnermost().getName();
        std::shared_ptr<C4Type> pType = pTypes[i];

        if (pType->isStruct()) errorloc(*(fd->params[i]->specifier), "Cannot pass struct by value");
        
        // Check if parameter has a name (required in C4 function definitions)
        if (!pName) {
          errorloc(*(fd->params[i]->specifier), "Parameter name missing in function definition");
        }
        

        if (funcScope->contains(pName)) errorloc(*(fd->params[i]->specifier), "Duplicate param");
        funcScope->add(pName, fd->params[i]->copy());
        varNames.push_back(pName);
      }
    }
    body->checkSemantics(funcScope);
    funcScope->resolveGoTos();
  }
}

void TranslationUnit::checkSemantics() const {
    const auto globalScope = std::make_shared<Scope>(nullptr);
    for (auto& decl : ext_decls) {
        decl->checkSemantics(globalScope);
    }
}

////////////////////////////////////////////////////////////////////////////////
//                               Declarations                                 //
////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<C4Type> VoidSpecifier::checkSemantics() {
  return std::make_shared<C4VoidType>();
}
std::shared_ptr<C4Type> CharSpecifier::checkSemantics() {
  return std::make_shared<C4CharType>();
}
std::shared_ptr<C4Type> IntSpecifier::checkSemantics() {
  return std::make_shared<C4IntType>();
}

std::shared_ptr<C4Type> StructSpecifier::checkSemantics() {
  // Reject anonymous structs (tag is nullptr)
  if (!tag)
    errorloc(*this, "Anonymous structs are not allowed");

  // Build the list of Fields (Name + Type)
  std::vector<C4StructType::Field> members;
  
  for (auto& c : components) {
    // Check semantics of the member (returns the Type)
    std::shared_ptr<C4Type> memberType = c->checkSemantics();
    
    // Extract the name from the declarator
    // We assume the parser ensures every member has a declarator (e.g., 'int x;')
    Symbol memberName = nullptr;
    if (c->declarator) {
        memberName = c->declarator->getInnermost().getName();
    } else {
        errorloc(*this, "Struct member missing declarator");
    }

    // Add to members list
    members.push_back({memberName, memberType});
  }

  // Construct and return the C4StructType
  return std::make_shared<C4StructType>(tag, members);
}

std::shared_ptr<C4Type> FunctionDeclarator::buildType(const std::shared_ptr<C4Type>& acc) {
    std::vector<std::shared_ptr<C4Type>> paraTypes;
    
    // In C, (void) means "no parameters", not "one void parameter"
    // So if we have exactly one parameter that is void with no declarator, skip it
    bool skipVoidParam = false;
    if (params.size() == 1) {
        auto paramType = params[0]->checkSemantics();
        if (paramType->isVoid()) {
            // Check if the declarator is empty (no name)
            if (params[0]->declarator) {
                const auto& innermost = params[0]->declarator->getInnermost();
                if (!innermost.getName()) {
                    skipVoidParam = true;
                }
            } else {
                skipVoidParam = true;
            }
        }
    }
    
    if (!skipVoidParam) {
        // Check if parameter types are valid
        for (const auto& param : params) {
            paraTypes.push_back(param->checkSemantics());
        }
    }
    return inner->buildType(std::make_shared<C4FunctionType>(acc,paraTypes));
}

std::shared_ptr<C4Type> PointerDeclarator::buildType(const std::shared_ptr<C4Type>& acc) {
  return inner->buildType(std::make_shared<C4PointerType>(acc));
}

std::shared_ptr<C4Type> PrimitiveDeclarator::buildType(const std::shared_ptr<C4Type>& acc) {
  return acc;
}

std::shared_ptr<C4Type> SpecDecl::checkSemantics() {
  return declarator->buildType(specifier->checkSemantics());
}

////////////////////////////////////////////////////////////////////////////////
//                                Expressions                                 //
////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<C4Type> IntLiteral::checkSemantics(std::shared_ptr<Scope> scope) {
  return exprType = std::make_shared<C4IntType>();
}

std::shared_ptr<C4Type> CharLiteral::checkSemantics(std::shared_ptr<Scope> scope) {
  return exprType = std::make_shared<C4CharType>();
}

std::shared_ptr<C4Type> StringLiteral::checkSemantics(std::shared_ptr<Scope> scope) {
  return exprType = std::make_shared<C4PointerType>(std::make_shared<C4CharType>());
}

std::shared_ptr<C4Type> Identifier::checkSemantics(std::shared_ptr<Scope> scope) {
  if (std::shared_ptr<SpecDecl> specDecl = scope->lookup(name))
    return exprType = specDecl->checkSemantics();
  errorloc(*this, "Undeclared identifier '", *name, "'");
}

std::shared_ptr<C4Type> UnaryOp::checkSemantics(std::shared_ptr<Scope> scope) {
  std::shared_ptr<C4Type> t = operand->checkSemantics(scope);

  // unary operator type checking
  switch (op) {
    case TK_PLUS:    // +
    case TK_MINUS:   // -
    case TK_TILDE:   // ~
      if (!t->isArithmetic()) errorloc(*this, "Unary operator requires arithmetic type");
      return exprType = t;
    
    case TK_BANG:    // !
      if (!t->isScalar()) errorloc(*this, "Logical NOT requires scalar type");
      return exprType = std::make_shared<C4IntType>();
    
    case TK_ASTERISK: // * (dereference)
      if (!t->isPointer()) errorloc(*this, "Dereference requires pointer type");
      return exprType = std::static_pointer_cast<C4PointerType>(t)->getBase();
    
    case TK_AND:     // & (address-of)
      if (!operand->isLValue()) errorloc(*this, "Address-of requires lvalue");
      return exprType = std::make_shared<C4PointerType>(t);
    
    case TK_PLUSPLUS:   // ++
    case TK_MINUSMINUS: // --
      if (!operand->isLValue()) errorloc(*this, "Increment/decrement requires lvalue");
      if (!t->isScalar()) errorloc(*this, "Increment/decrement requires scalar type");
      return exprType = t;
    
    default:
      return nullptr;
  }
}

std::shared_ptr<C4Type> BinaryOp::checkSemantics(std::shared_ptr<Scope> scope) {
  std::shared_ptr<C4Type> l = left->checkSemantics(scope);
  std::shared_ptr<C4Type> r = right->checkSemantics(scope);

  // binary operator type checking
  switch (op) {
    case TK_PLUS:  // +
      // pointer + int or int + pointer (returns pointer)
      if (l->isPointer() && r->isArithmetic()) return exprType = l;
      if (l->isArithmetic() && r->isPointer()) return exprType = r;
      // both arithmetic
      if (l->isArithmetic() && r->isArithmetic()) return exprType = l;
      errorloc(*this, "Invalid operands for addition");
    
    case TK_MINUS: // -
      // pointer - int (returns pointer)
      if (l->isPointer() && r->isArithmetic()) return exprType = l;
      // pointer - pointer: allowed if both point to compatible types
      if (l->isPointer() && r->isPointer()) {
        auto lPtr = std::static_pointer_cast<C4PointerType>(l);
        auto rPtr = std::static_pointer_cast<C4PointerType>(r);
        
        // Both pointers must point to the same type
        if (lPtr->equals(*rPtr)) {
          return exprType = std::make_shared<C4IntType>();
        }
        
        // Different pointer types - not allowed in C4
        errorloc(*this, "Pointer subtraction of incompatible pointer types");
      }
      // both arithmetic
      if (l->isArithmetic() && r->isArithmetic()) return exprType = l;
      errorloc(*this, "Invalid operands for subtraction");
    
    case TK_ASTERISK:  // *
    case TK_SLASH:     // /
    case TK_PERCENT:   // %
      if (!l->isArithmetic() || !r->isArithmetic())
        errorloc(*this, "Arithmetic operator requires arithmetic types");
      return exprType = l;
    
    case TK_LANGLELANGLE:  // <<
    case TK_RANGLERANGLE:  // >>
    case TK_AND:           // &
    case TK_PIPE:          // |
    case TK_CARET:         // ^
      if (!l->isArithmetic() || !r->isArithmetic())
        errorloc(*this, "Bitwise operator requires arithmetic types");
      return exprType = l;
    
    case TK_LANGLE:        // <
    case TK_RANGLE:        // >
    case TK_LANGLEEQUALS:  // <=
    case TK_RANGLEEQUALS:  // >=
      // Both arithmetic or both pointers
      if ((l->isArithmetic() && r->isArithmetic()) ||
          (l->isPointer() && r->isPointer())) {
        return exprType = std::make_shared<C4IntType>();
      }
      errorloc(*this, "Comparison requires compatible types");
    
    case TK_EQUALSEQUALS:  // ==
    case TK_BANGEQUALS:    // !=
      // Both arithmetic or both pointers of same type
      if (l->isArithmetic() && r->isArithmetic()) {
        return exprType = std::make_shared<C4IntType>();
      }
      if (l->isPointer() && r->isPointer()) {
        if (l->equals(*r)) return exprType = std::make_shared<C4IntType>();
        errorloc(*this, "Equality comparison of incompatible pointer types");
      }
      // Allow comparison with null pointer constant (literal 0)
      if (l->isPointer() && r->isArithmetic()) {
        // Check if right is a literal 0 (null pointer constant)
        if (auto* intLit = dynamic_cast<IntLiteral*>(right.get())) {
          // allow null pointer constant
          if (intLit->value == 0)
            return exprType = std::make_shared<C4IntType>();
        }
        errorloc(*this, "Equality comparison requires compatible types - cannot compare pointer with non-zero integer");
      }
      if (l->isArithmetic() && r->isPointer()) {
        // Check if left is a literal 0 (null pointer constant)
        if (auto* intLit = dynamic_cast<IntLiteral*>(left.get())) {
          // allow null pointer constant
          if (intLit->value == 0)
            return exprType = std::make_shared<C4IntType>();
        }
        errorloc(*this, "Equality comparison requires compatible types - cannot compare pointer with non-zero integer");
      }
      errorloc(*this, "Equality comparison requires compatible types");
    
    case TK_ANDAND:  // &&
    case TK_PIPEPIPE: // ||
      if (!l->isScalar() || !r->isScalar())
        errorloc(*this, "Logical operator requires scalar types");
      return exprType = std::make_shared<C4IntType>();
    
    default:
      return nullptr;
  }
}

std::shared_ptr<C4Type> TernaryOp::checkSemantics(std::shared_ptr<Scope> scope) {
  std::shared_ptr<C4Type> cond = condition->checkSemantics(scope);
  std::shared_ptr<C4Type> cons = consequent->checkSemantics(scope);
  std::shared_ptr<C4Type> alt = alternative->checkSemantics(scope);

  // Check ternary operator semantics
  if (!cond->isScalar())
    errorloc(*this, "Ternary condition must be scalar");

  auto res = cons;

  // Both branches should have compatible types
  if (!cons->equals(*alt)) {
    bool valid = false;

    if (cons->isArithmetic() && alt->isArithmetic()) {
      valid = true;
    }
    else if (cons->isPointer() && alt->isArithmetic()) {
      if (auto* intLit = dynamic_cast<IntLiteral*>(alternative.get())) {
        if (intLit->value == 0) valid = true;
      }
    }
    else if (cons->isArithmetic() && alt->isPointer()) {
      if (auto* intLit = dynamic_cast<IntLiteral*>(consequent.get())) {
        if (intLit->value == 0) valid = true;
        res = alt;
      }
    }

    if (!valid)
      errorloc(*this, "Ternary branches have incompatible types");
  }
  return exprType = res;
}

std::shared_ptr<C4Type> FunctionCall::checkSemantics(std::shared_ptr<Scope> scope) {
  std::shared_ptr<C4Type> ft = function->checkSemantics(scope);

  // Function pointer
  if (ft->isPointer()) {
    auto ptr = std::static_pointer_cast<C4PointerType>(ft);
    if (ptr->getBase()->isFunction()) {
      ft = ptr->getBase();
    }
  }

  if (!ft->isFunction())
    errorloc(*this, "Called object is not a function");
  
  auto funcType = std::static_pointer_cast<C4FunctionType>(ft);
  const auto& params = funcType->getParams();
  
  // Check argument count
  if (arguments.size() != params.size())
    errorloc(*this, "Function call argument count mismatch");
  
  // Check argument types
  for (size_t i = 0; i < arguments.size(); ++i) {
    std::shared_ptr<C4Type> argType = arguments[i]->checkSemantics(scope);
    if (!argType->equals(*params[i])) {
      // Only allow implicit conversions between arithmetic types (int <-> char)
      if (argType->isArithmetic() && params[i]->isArithmetic()) {
        continue; // Allow int/char conversions
      }
      // Allow null pointer constant to pointer type conversion
      if (params[i]->isPointer()) {
        if (auto* intLit = dynamic_cast<IntLiteral*>(arguments[i].get())) {
          if (intLit->value == 0) continue;
        }
      }
      // No other implicit conversions allowed
      errorloc(*this, "Function call argument type mismatch");
    }
  }
  
  return exprType = funcType->getReturnType();
}

std::shared_ptr<C4Type> Assignment::checkSemantics(std::shared_ptr<Scope> scope) {
  if (!lhs->isLValue()) errorloc(*this, "Assignment to non-Lvalue");
  std::shared_ptr<C4Type> l = lhs->checkSemantics(scope);
  std::shared_ptr<C4Type> r = rhs->checkSemantics(scope);

  // Check assignment type compatibility
  if (l->isStruct() || r->isStruct())
    errorloc(*this, "Cannot assign struct types");
  
  // For simple assignment
  if (op == TK_EQUALS) {
    // Exact type match is always OK
    if (l->equals(*r)) {
      return exprType = l;
    }
    
    // Allow implicit conversions between arithmetic types
    if (l->isArithmetic() && r->isArithmetic()) {
      return exprType = l;
    }
    
    // Allow pointer = pointer of exactly same type
    if (l->isPointer() && r->isPointer()) {
      if (l->equals(*r)) return exprType = l;
      errorloc(*this, "Assignment type mismatch - incompatible pointer types");
    }
    
    // Special case: Allow pointer = 0 (null pointer constant)
    // but reject pointer = non-zero integer
    if (l->isPointer() && r->isArithmetic()) {
      // Check if rhs is literal 0
      if (IntLiteral* intLit = dynamic_cast<IntLiteral*>(rhs.get())) {
        if (intLit->value == 0) {
          return exprType = l; // Allow p = 0
        }
      }
      errorloc(*this, "Assignment type mismatch - cannot assign integer to pointer");
    }
    
    // Disallow int = pointer
    if (l->isArithmetic() && r->isPointer()) {
      errorloc(*this, "Assignment type mismatch - cannot assign pointer to integer");
    }
    
    errorloc(*this, "Assignment type mismatch");
  } else {
    // Compound assignments (+=, -=, etc.) require compatible types
    if (!l->isScalar() || !r->isScalar())
      errorloc(*this, "Compound assignment requires scalar types");
  }
  return exprType = l;
}

std::shared_ptr<C4Type> SizeOf::checkSemantics(std::shared_ptr<Scope> scope) {
  // sizeof returns int, check operand
  if (expr_operand) {
    operandType = expr_operand->checkSemantics(scope);
  } else if (type_operand) {
    operandType = type_operand->checkSemantics();
  }
  return exprType = std::make_shared<C4IntType>();
}

std::shared_ptr<C4Type> Cast::checkSemantics(std::shared_ptr<Scope> scope) {
  std::shared_ptr<C4Type> target = type->checkSemantics();
  std::shared_ptr<C4Type> src = operand->checkSemantics(scope);

  if (!target->isScalar() || !src->isScalar()) {
    if (!target->isVoid()) // (void) cast is allowed
      errorloc(*this, "Cast involves non-scalar");
  }
  return exprType = target;
}

std::shared_ptr<C4Type> Index::checkSemantics(std::shared_ptr<Scope> scope) {
  //Check array subscript - a[i] is equivalent to *(a+i)
  std::shared_ptr<C4Type> arrayType = array->checkSemantics(scope);
  std::shared_ptr<C4Type> indexType = index->checkSemantics(scope);
  
  // One must be pointer, other must be integer (or vice versa)
  if (arrayType->isPointer() && indexType->isArithmetic()) {
    return exprType = std::static_pointer_cast<C4PointerType>(arrayType)->getBase();
  }
  if (arrayType->isArithmetic() && indexType->isPointer()) {
    // Don't swap here - preserve original AST, handle in codegen
    is_swapped = true;
    return exprType = std::static_pointer_cast<C4PointerType>(indexType)->getBase();
  }
  
  errorloc(*this, "Array subscript requires pointer and integer");
}

std::shared_ptr<C4Type> Member::checkSemantics(std::shared_ptr<Scope> scope) {
  std::shared_ptr<C4Type> objType = object->checkSemantics(scope);
  
  if (is_arrow) {
    // obj->member: obj must be pointer to struct
    if (!objType->isPointer())
      errorloc(*this, "Arrow operator requires pointer type");
    
    objType = std::static_pointer_cast<C4PointerType>(objType)->getBase();
  }
  
  // Now objType should be struct
  if (!objType->isStruct())
    errorloc(*this, "Member access requires struct type");
  
  // Look up the struct definition to validate member exists
  auto structType = std::static_pointer_cast<C4StructType>(objType);
  Symbol structTag = structType->getTag();
  
  // Look up struct definition in scope
  auto structDecl = scope->lookup(structTag);
  if (!structDecl)
    errorloc(*this, "Struct type '", *structTag, "' not found");
  
  // Get the struct specifier to check components
  // The struct declaration's specifier should be a StructSpecifier
  auto structSpec = dynamic_cast<StructSpecifier*>(structDecl->specifier.get());
  if (!structSpec)
    errorloc(*this, "Expected struct specifier");
  
  // Check if member exists in struct components
  bool found = false;
  std::shared_ptr<C4Type> memberType;
  for (const auto& component : structSpec->components) {
    Symbol compName = component->declarator->getInnermost().getName();
    if (compName == member) {
      found = true;
      memberType = component->checkSemantics();
      break;
    }
  }
  
  if (!found)
    errorloc(*this, "Struct '", *structTag, "' has no member named '", *member, "'");
  
  return exprType = memberType;
}

////////////////////////////////////////////////////////////////////////////////
//                                Statements                                  //
////////////////////////////////////////////////////////////////////////////////

void CompoundStmt::checkSemantics(std::shared_ptr<Scope> scope) {
  // Create a new scope for the compound statement (C block scoping)
  std::shared_ptr<Scope> blockScope;
  if (is_fun_body) {
    blockScope = scope;  // Function body shares scope with parameters
  } else {
    blockScope = std::make_shared<Scope>(scope);  // Nested blocks get new scope
  }
  for (auto& statement : components) statement->checkSemantics(blockScope);
}

void LabelStmt::checkSemantics(std::shared_ptr<Scope> scope) {
  if (!scope->getRelatedFunc())
    errorloc(*this, "Labels can only be declared in a function");

  if (scope->lookupLabel(label))
    errorloc(*this, "label already exists");
  
  scope->addDeclaredLabel(this);
  stmt->checkSemantics(scope);
}

void ExpressionStmt::checkSemantics(std::shared_ptr<Scope> scope) {
  expr->checkSemantics(scope);
}

void NullStmt::checkSemantics(std::shared_ptr<Scope> scope) {}

void IfStmt::checkSemantics(std::shared_ptr<Scope> scope) {
  std::shared_ptr<C4Type> t = condition->checkSemantics(scope);
  if (!t->isScalar()) errorloc(*this, "If condition must be scalar");
  consequence->checkSemantics(scope);
  if (alternative) alternative->checkSemantics(scope);
}

void WhileStmt::checkSemantics(std::shared_ptr<Scope> scope) {
  std::shared_ptr<C4Type> t = condition->checkSemantics(scope);
  if (!t->isScalar()) errorloc(*this, "While condition must be scalar");
  auto whileScope = std::make_shared<Scope>(scope, this);
  body->checkSemantics(whileScope);
}

void ReturnStmt::checkSemantics(std::shared_ptr<Scope> scope) {
  if (!scope->getRelatedFunc())
    errorloc(*this, "Return outside function");

  auto funcSpecDecl = scope->getRelatedFunc()->spec_decl->copy();
  
  // Get the function's return type from the SpecDecl
  auto funcType = funcSpecDecl->checkSemantics();
  if (!funcType->isFunction())
    errorloc(*this, "Related function has non-function type");
  
  auto ft = std::static_pointer_cast<C4FunctionType>(funcType);
  auto expectedReturnType = ft->getReturnType();
  
  if(expr)
  {
    auto exprType = expr->checkSemantics(scope);
    // Check return type matches function return type
    
    // Check if returning from void function
    if (expectedReturnType->isVoid()) {
      if (!exprType->isVoid()) {
        errorloc(*this, "Cannot return value from void function");
      }
    } else {
      if (exprType->isStruct())
        errorloc(*this, "Cannot return struct by value");
        
      // Check type compatibility
      if (!exprType->equals(*expectedReturnType)) {
        // Allow some implicit conversions between arithmetic types
        if (exprType->isArithmetic() && expectedReturnType->isPointer()) {
          if (auto* intLit = dynamic_cast<IntLiteral*>(expr.get())) {
            // allow null pointer constant
            if (intLit->value != 0)
              errorloc(*this, "non zero int returned for pointer type");
          }
        } else if (!(exprType->isArithmetic() && expectedReturnType->isArithmetic()))
          errorloc(*this, "Return type mismatch");
      }
    }
  } else {
    // Returning void - should only be in void function
    if (!expectedReturnType->isVoid())
      errorloc(*this, "Non-void function must return a value");
  }
}

void BreakContinueStmt::checkSemantics(std::shared_ptr<Scope> scope) {
  auto relatedWhile = scope->getRelatedWhile();
  if (!relatedWhile)
    errorloc(*this, "Break/Continue outside While");
  destination = relatedWhile;
}

void GotoStmt::checkSemantics(std::shared_ptr<Scope> scope) {
  scope->addGoTo(this);
}

void Declaration::checkSemantics(std::shared_ptr<Scope> scope) {
  // Check if this declares a struct tag BEFORE checking semantics
  if (auto* structSpec = dynamic_cast<StructSpecifier*>(spec_decl->specifier.get())) {
    if (structSpec->tag && !structSpec->components.empty()) {
      // This is a struct definition - register the tag
      if (scope->contains(structSpec->tag))
        errorloc(*this, "Redefinition of struct tag '", *structSpec->tag, "'");
      
      // Check for duplicate fields in struct
      std::unordered_set<Symbol> fieldNames;
      for (const auto& component : structSpec->components) {
        Symbol fieldName = component->declarator->getInnermost().getName();
        if (fieldName) {
          if (fieldNames.count(fieldName)) {
            errorloc(*this, "Duplicate field '", *fieldName, "' in struct");
          }
          fieldNames.insert(fieldName);
        }
      }
      
      // Create a copy of the spec_decl to store for the struct tag
      scope->add(structSpec->tag, spec_decl->copy());
    }
  }
  
  c4Type = spec_decl->checkSemantics();
  Symbol name = spec_decl->declarator->getInnermost().getName();
  
  // Reject declarations without declarator (e.g., "int;")
  // BUT allow forward declarations and struct definitions without declarators (e.g. "struct S;", "struct S { int a; };")
  if (!name) {
    bool isStructDef = false;
    if (auto* structSpec = dynamic_cast<StructSpecifier*>(spec_decl->specifier.get())) {
      if (structSpec->tag) {
        isStructDef = true;  // This is a struct definition, which is allowed without declarator
      }
    }
    if (!isStructDef) {
      errorloc(*(spec_decl->specifier), "Declaration without declarator");
    }
  }
  
  if (name && scope->contains(name)) errorloc(*(spec_decl->specifier), "Redeclaration");
  if (name) scope->add(name, spec_decl->copy());
}
