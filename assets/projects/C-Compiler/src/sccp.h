#pragma once

#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>
#include <map>
#include <set>

// Lattice values for SCCP
enum class LatticeValue {
    TOP,        // Unknown/uninitialized
    CONSTANT,   // Known constant value
    BOTTOM      // Not a constant (overdefined)
};

// Lattice cell holding a value and its constant (if applicable)
struct LatticeCell {
    LatticeValue value;
    llvm::Constant* constant; // Only valid if value == CONSTANT

    LatticeCell() : value(LatticeValue::TOP), constant(nullptr) {}
    LatticeCell(LatticeValue v) : value(v), constant(nullptr) {}
    LatticeCell(llvm::Constant* c) : value(LatticeValue::CONSTANT), constant(c) {}

    bool isTop() const { return value == LatticeValue::TOP; }
    bool isConstant() const { return value == LatticeValue::CONSTANT; }
    bool isBottom() const { return value == LatticeValue::BOTTOM; }

    // Meet operation for lattice join
    void meet(const LatticeCell& other);
};

class SCCPPass {
public:
    // Run SCCP optimization on a module
    bool runOnModule(llvm::Module& M);

private:
    // Lattice value map for SSA values
    std::map<llvm::Value*, LatticeCell> valueState;
    
    // Executable edge tracking
    std::set<std::pair<llvm::BasicBlock*, llvm::BasicBlock*>> executableEdges;
    
    // Executable blocks
    std::set<llvm::BasicBlock*> executableBlocks;
    
    // Worklists
    std::vector<llvm::Instruction*> ssaWorkList;
    std::vector<llvm::BasicBlock*> bbWorkList;
    
    // Run SCCP on a single function
    bool runOnFunction(llvm::Function& F);
    
    // Initialize the algorithm
    void initialize(llvm::Function& F);
    
    // Mark edge as executable 
    void markEdgeExecutable(llvm::BasicBlock* from, llvm::BasicBlock* to);
    
    // Mark block as executable
    void markBlockExecutable(llvm::BasicBlock* BB);
    
    // Get lattice value for an LLVM value
    LatticeCell getLatticeValue(llvm::Value* V);
    
    // Update lattice value for an SSA value
    void updateValue(llvm::Value* V, const LatticeCell& cell);
    
    // Visit an instruction
    void visitInst(llvm::Instruction* I);
    
    // Visit specific instruction types
    void visitPHINode(llvm::PHINode* PHI);
    void visitBinaryOperator(llvm::BinaryOperator* BO);
    void visitCastInst(llvm::CastInst* CI);
    void visitSelectInst(llvm::SelectInst* SI);
    void visitCmpInst(llvm::CmpInst* CI);
    void visitLoadInst(llvm::LoadInst* LI);
    void visitStoreInst(llvm::StoreInst* SI);
    void visitCallInst(llvm::CallInst* CI);
    void visitBranchInst(llvm::BranchInst* BI);
    void visitSwitchInst(llvm::SwitchInst* SI);
    void visitReturnInst(llvm::ReturnInst* RI);
    
    // Check if an edge is executable
    bool isEdgeExecutable(llvm::BasicBlock* from, llvm::BasicBlock* to);
    
    // Check if a block is executable
    bool isBlockExecutable(llvm::BasicBlock* BB);
    
    // Replace uses and delete dead code
    bool rewriteCode(llvm::Function& F);
    
    // Remove unreachable blocks
    bool removeUnreachableBlocks(llvm::Function& F);
};
