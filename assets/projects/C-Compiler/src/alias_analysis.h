#pragma once

#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Analysis/AliasAnalysis.h>
#include <map>
#include <set>

// Points-to graph: maps each pointer variable to the set of locations it may point to
using PointsToSet = std::set<const llvm::Value*>; 
using PointsToGraph = std::map<const llvm::Value*, PointsToSet>; 

class PointsToAliasAnalysis {
public:
    // Constructor
    PointsToAliasAnalysis() = default;

    // Run the flow-sensitive points-to analysis on the module
    void analyze(llvm::Module &M);

    // Query aliasing between two values at a specific instruction
    llvm::AliasResult alias(const llvm::Value* V1, const llvm::Value* V2,
                           const llvm::Instruction* CtxI)const;

private:
    // Points-to information at each program point (instruction)
    std::map<const llvm::Instruction*, PointsToGraph> pointsToInfo;

    // Process a single function
    void analyzeFunction(llvm::Function &F);

    // Transfer functions for different instruction types
    PointsToGraph transfer(const llvm::Instruction* I, const PointsToGraph& InPTG);

    // Helper: Get points-to set for a value
    PointsToSet getPointsToSet(const llvm::Value* V, const PointsToGraph& PTG) const;

    // Helper: Resolve points-to through dereferences
    PointsToSet resolvePointsTo(const llvm::Value* V, const PointsToGraph& PTG) const;
};
