#include "sccp.h"
#include <llvm/IR/CFG.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/PatternMatch.h>
#include <llvm/Support/Casting.h>
#include <llvm/Analysis/ConstantFolding.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <queue>

using namespace llvm;

// Lattice meet operation
void LatticeCell::meet(const LatticeCell& other) {
    if (isBottom() || other.isTop()) {
        // BOTTOM meet anything = BOTTOM
        // anything meet TOP = anything
        return;
    }
    
    if (other.isBottom()) {
        value = LatticeValue::BOTTOM;
        constant = nullptr;
        return;
    }
    
    if (isTop()) {
        // TOP meet anything = anything
        value = other.value;
        constant = other.constant;
        return;
    }
    
    // Both are constants
    if (isConstant() && other.isConstant()) {
        if (constant != other.constant) {
            // Different constants = BOTTOM
            value = LatticeValue::BOTTOM;
            constant = nullptr;
        }
        // Same constant remains constant
    }
}

bool SCCPPass::runOnModule(Module& M) {
    bool modified = false;
    
    for (Function& F : M) {
        if (!F.isDeclaration()) {
            modified |= runOnFunction(F);
        }
    }
    
    return modified;
}

bool SCCPPass::runOnFunction(Function& F) {
    // Initialize the analysis
    initialize(F);
    
    // Mark entry block as executable with a virtual edge from nullptr
    markEdgeExecutable(nullptr, &F.getEntryBlock());
    
    // Process worklists until fixed point
    while (!ssaWorkList.empty() || !bbWorkList.empty()) {
        // Process basic block worklist
        while (!bbWorkList.empty()) {
            BasicBlock* BB = bbWorkList.back();
            bbWorkList.pop_back();
            
            // Visit all instructions in the block
            for (Instruction& I : *BB) {
                visitInst(&I);
            }
        }
        
        // Process SSA worklist
        while (!ssaWorkList.empty() && bbWorkList.empty()) {
            Instruction* I = ssaWorkList.back();
            ssaWorkList.pop_back();
            
            visitInst(I);
        }
    }
    
    // Rewrite the code based on analysis results
    bool modified = rewriteCode(F);
    
    // Remove unreachable blocks
    modified |= removeUnreachableBlocks(F);
    
    // Clear state for next function
    valueState.clear();
    executableEdges.clear();
    executableBlocks.clear();
    
    return modified;
}

void SCCPPass::initialize(Function& F) {
    valueState.clear();
    executableEdges.clear();
    executableBlocks.clear();
    ssaWorkList.clear();
    bbWorkList.clear();
    
    // Initialize all instruction values to TOP
    for (Instruction& I : instructions(F)) {
        if (I.getType()->isVoidTy()) continue;
        valueState[&I] = LatticeCell(LatticeValue::TOP);
    }
    
    // Initialize function arguments to BOTTOM (unknown)
    for (Argument& Arg : F.args()) {
        valueState[&Arg] = LatticeCell(LatticeValue::BOTTOM);
    }
}

void SCCPPass::markEdgeExecutable(BasicBlock* from, BasicBlock* to) {
    auto edge = std::make_pair(from, to);
    if (executableEdges.count(edge)) {
        return; // Already marked
    }
    
    executableEdges.insert(edge);
    
    if (isBlockExecutable(to)) {
        // Block already executable, but new edge found.
        // Re-evaluate PHI nodes since they depend on which edges are executable.
        for (auto &I : *to) {
            if (PHINode *PHI = dyn_cast<PHINode>(&I)) {
                ssaWorkList.push_back(PHI);
            } else {
                break; // PHI nodes are always at the beginning of the block
            }
        }
    } else {
        markBlockExecutable(to);
    }
}

void SCCPPass::markBlockExecutable(BasicBlock* BB) {
    if (executableBlocks.count(BB)) {
        return; // Already executable
    }
    
    executableBlocks.insert(BB);
    bbWorkList.push_back(BB);
}

LatticeCell SCCPPass::getLatticeValue(Value* V) {
    // Constants are always constant
    if (Constant* C = dyn_cast<Constant>(V)) {
        if (!isa<UndefValue>(C)) {
            return LatticeCell(C);
        }
        return LatticeCell(LatticeValue::TOP);
    }
    
    auto it = valueState.find(V);
    if (it != valueState.end()) {
        return it->second;
    }
    
    // Default to TOP for unknowns
    return LatticeCell(LatticeValue::TOP);
}

void SCCPPass::updateValue(Value* V, const LatticeCell& cell) {
    auto it = valueState.find(V);
    if (it == valueState.end()) {
        // First time setting the value
        valueState[V] = cell;
        
        // Add users to worklist if value is not TOP anymore
        if (!cell.isTop()) {
            if (Instruction* I = dyn_cast<Instruction>(V)) {
                for (User* U : I->users()) {
                    if (Instruction* UserInst = dyn_cast<Instruction>(U)) {
                        if (isBlockExecutable(UserInst->getParent())) {
                            ssaWorkList.push_back(UserInst);
                        }
                    }
                }
            }
        }
        return;
    }
    
    LatticeCell oldValue = it->second;
    it->second.meet(cell);
    
    // If value changed (moved down in lattice), add users to worklist
    if (oldValue.value != it->second.value || 
        (oldValue.value == LatticeValue::CONSTANT && it->second.value == LatticeValue::CONSTANT &&
         oldValue.constant != it->second.constant)) {
        if (Instruction* I = dyn_cast<Instruction>(V)) {
            for (User* U : I->users()) {
                if (Instruction* UserInst = dyn_cast<Instruction>(U)) {
                    if (isBlockExecutable(UserInst->getParent())) {
                        ssaWorkList.push_back(UserInst);
                    }
                }
            }
        }
    }
}

bool SCCPPass::isEdgeExecutable(BasicBlock* from, BasicBlock* to) {
    return executableEdges.count(std::make_pair(from, to)) > 0;
}

bool SCCPPass::isBlockExecutable(BasicBlock* BB) {
    return executableBlocks.count(BB) > 0;
}

void SCCPPass::visitInst(Instruction* I) {
    // Skip instructions in non-executable blocks (e.g. from SSA worklist)
    if (!isBlockExecutable(I->getParent())) return;
    
    if (PHINode* PHI = dyn_cast<PHINode>(I)) {
        visitPHINode(PHI);
    } else if (BinaryOperator* BO = dyn_cast<BinaryOperator>(I)) {
        visitBinaryOperator(BO);
    } else if (CastInst* CI = dyn_cast<CastInst>(I)) {
        visitCastInst(CI);
    } else if (SelectInst* SI = dyn_cast<SelectInst>(I)) {
        visitSelectInst(SI);
    } else if (CmpInst* CI = dyn_cast<CmpInst>(I)) {
        visitCmpInst(CI);
    } else if (LoadInst* LI = dyn_cast<LoadInst>(I)) {
        visitLoadInst(LI);
    } else if (StoreInst* SI = dyn_cast<StoreInst>(I)) {
        visitStoreInst(SI);
    } else if (CallInst* CI = dyn_cast<CallInst>(I)) {
        visitCallInst(CI);
    } else if (BranchInst* BI = dyn_cast<BranchInst>(I)) {
        visitBranchInst(BI);
    } else if (SwitchInst* SI = dyn_cast<SwitchInst>(I)) {
        visitSwitchInst(SI);
    } else if (ReturnInst* RI = dyn_cast<ReturnInst>(I)) {
        visitReturnInst(RI);
    } else if (isa<AllocaInst>(I) || isa<GetElementPtrInst>(I)) {
        // Alloca and GEP produce pointers, mark as BOTTOM
        updateValue(I, LatticeCell(LatticeValue::BOTTOM));
    } else {
        // Unknown instruction, mark as BOTTOM
        if (!I->getType()->isVoidTy()) {
            updateValue(I, LatticeCell(LatticeValue::BOTTOM));
        }
    }
}

void SCCPPass::visitPHINode(PHINode* PHI) {
    LatticeCell result(LatticeValue::TOP);
    
    // Meet values from executable predecessors only
    for (unsigned i = 0; i < PHI->getNumIncomingValues(); ++i) {
        BasicBlock* predBB = PHI->getIncomingBlock(i);
        if (!isEdgeExecutable(predBB, PHI->getParent())) {
            continue; // Skip non-executable edge
        }
        
        Value* incomingValue = PHI->getIncomingValue(i);
        LatticeCell incomingCell = getLatticeValue(incomingValue);
        result.meet(incomingCell);
    }
    
    updateValue(PHI, result);
}

void SCCPPass::visitBinaryOperator(BinaryOperator* BO) {
    LatticeCell lhs = getLatticeValue(BO->getOperand(0));
    LatticeCell rhs = getLatticeValue(BO->getOperand(1));
    
    // Absorbing elements that work even when the other operand is TOP/BOTTOM/CONSTANT
    // x * 0 = 0, x & 0 = 0 (regardless of x)
    if (BO->getOpcode() == Instruction::Mul) {
        if (rhs.isConstant() && isa<ConstantInt>(rhs.constant) &&
            cast<ConstantInt>(rhs.constant)->isZero()) {
            updateValue(BO, rhs);
            return;
        }
        if (lhs.isConstant() && isa<ConstantInt>(lhs.constant) &&
            cast<ConstantInt>(lhs.constant)->isZero()) {
            updateValue(BO, lhs);
            return;
        }
    }
    if (BO->getOpcode() == Instruction::And) {
        if (rhs.isConstant() && isa<ConstantInt>(rhs.constant) &&
            cast<ConstantInt>(rhs.constant)->isZero()) {
            updateValue(BO, rhs);
            return;
        }
        if (lhs.isConstant() && isa<ConstantInt>(lhs.constant) &&
            cast<ConstantInt>(lhs.constant)->isZero()) {
            updateValue(BO, lhs);
            return;
        }
    }
    if (BO->getOpcode() == Instruction::Or) {
        if (rhs.isConstant() && isa<ConstantInt>(rhs.constant) &&
            cast<ConstantInt>(rhs.constant)->isMinusOne()) {
            updateValue(BO, rhs);
            return;
        }
        if (lhs.isConstant() && isa<ConstantInt>(lhs.constant) &&
            cast<ConstantInt>(lhs.constant)->isMinusOne()) {
            updateValue(BO, lhs);
            return;
        }
    }
    
    // If either operand is TOP, result stays TOP (don't update yet)
    if (lhs.isTop() || rhs.isTop()) {
        return;
    }
    
    // Same-value optimizations: x - x = 0, x ^ x = 0 (when x is not TOP)
    if (BO->getOperand(0) == BO->getOperand(1) && !lhs.isTop()) {
        Type *Ty = BO->getType();
        switch (BO->getOpcode()) {
            case Instruction::Sub:
            case Instruction::Xor:
                updateValue(BO, LatticeCell(ConstantInt::get(Ty, 0)));
                return;
            case Instruction::URem:
            case Instruction::SRem:
                updateValue(BO, LatticeCell(ConstantInt::get(Ty, 0)));
                return;
            case Instruction::UDiv:
            case Instruction::SDiv:
                updateValue(BO, LatticeCell(ConstantInt::get(Ty, 1)));
                return;
            case Instruction::And:
            case Instruction::Or:
                // x & x = x, x | x = x
                updateValue(BO, lhs);
                return;
            default:
                break;
        }
    }
    
    // Identity operations: x + 0 = x, x * 1 = x, etc.
    if (lhs.isConstant() && rhs.isConstant()) {
        // Both are constants - fold it using ConstantFoldBinaryOpOperands
        // (ConstantExpr::get only supports a limited set of opcodes in newer LLVM)
        const DataLayout &DL = BO->getModule()->getDataLayout();
        Constant* result = ConstantFoldBinaryOpOperands(
            BO->getOpcode(), 
            lhs.constant, 
            rhs.constant,
            DL
        );
        if (result) {
            updateValue(BO, LatticeCell(result));
        } else {
            updateValue(BO, LatticeCell(LatticeValue::BOTTOM));
        }
        return;
    }
    
    // If either operand is BOTTOM, result is BOTTOM
    if (lhs.isBottom() || rhs.isBottom()) {
        updateValue(BO, LatticeCell(LatticeValue::BOTTOM));
        return;
    }
}

void SCCPPass::visitCastInst(CastInst* CI) {
    LatticeCell operand = getLatticeValue(CI->getOperand(0));
    
    if (operand.isTop()) {
        return; // Result stays TOP
    }
    
    if (operand.isBottom()) {
        updateValue(CI, LatticeCell(LatticeValue::BOTTOM));
        return;
    }
    
    // Operand is constant, fold the cast
    if (operand.isConstant()) {
        const DataLayout &DL = CI->getModule()->getDataLayout();
        Constant* result = ConstantFoldCastOperand(
            CI->getOpcode(),
            operand.constant,
            CI->getType(),
            DL
        );
        if (result) {
            updateValue(CI, LatticeCell(result));
        } else {
            updateValue(CI, LatticeCell(LatticeValue::BOTTOM));
        }
    }
}

void SCCPPass::visitSelectInst(SelectInst* SI) {
    LatticeCell cond = getLatticeValue(SI->getCondition());
    
    if (cond.isTop()) {
        // Condition not yet determined. Don't evaluate eagerly to avoid
        // premature BOTTOM.  However, if both arms are the same constant the
        // result is known regardless of the condition.
        LatticeCell trueVal  = getLatticeValue(SI->getTrueValue());
        LatticeCell falseVal = getLatticeValue(SI->getFalseValue());
        if (trueVal.isConstant() && falseVal.isConstant() &&
            trueVal.constant == falseVal.constant) {
            updateValue(SI, trueVal);
        }
        return;
    }
    
    if (cond.isConstant()) {
        // Condition is constant, select one branch
        if (ConstantInt* condInt = dyn_cast<ConstantInt>(cond.constant)) {
            Value* selected = condInt->isOne() ? SI->getTrueValue() : SI->getFalseValue();
            LatticeCell result = getLatticeValue(selected);
            updateValue(SI, result);
            return;
        }
    }
    
    // cond is BOTTOM: condition is non-constant, both branches are possible
    LatticeCell trueVal = getLatticeValue(SI->getTrueValue());
    LatticeCell falseVal = getLatticeValue(SI->getFalseValue());
    
    LatticeCell result = trueVal;
    result.meet(falseVal);
    updateValue(SI, result);
}

void SCCPPass::visitCmpInst(CmpInst* CI) {
    LatticeCell lhs = getLatticeValue(CI->getOperand(0));
    LatticeCell rhs = getLatticeValue(CI->getOperand(1));
    
    if (lhs.isTop() || rhs.isTop()) {
        return; // Result stays TOP
    }
    
    if (lhs.isBottom() || rhs.isBottom()) {
        updateValue(CI, LatticeCell(LatticeValue::BOTTOM));
        return;
    }
    
    // Both operands are constants, fold the comparison
    if (lhs.isConstant() && rhs.isConstant()) {
        Constant* result = ConstantFoldCompareInstOperands(
            CI->getPredicate(),
            lhs.constant,
            rhs.constant,
            CI->getModule()->getDataLayout()
        );
        if (result) {
            updateValue(CI, LatticeCell(result));
        } else {
            updateValue(CI, LatticeCell(LatticeValue::BOTTOM));
        }
    }
}

void SCCPPass::visitLoadInst(LoadInst* LI) {
    // We can't track memory in simple SCCP, mark as BOTTOM
    updateValue(LI, LatticeCell(LatticeValue::BOTTOM));
}

void SCCPPass::visitStoreInst(StoreInst* SI) {
    // Store doesn't produce a value, nothing to do
}

void SCCPPass::visitCallInst(CallInst* CI) {
    // Function calls are generally unknown, mark as BOTTOM
    if (!CI->getType()->isVoidTy()) {
        updateValue(CI, LatticeCell(LatticeValue::BOTTOM));
    }
}

void SCCPPass::visitBranchInst(BranchInst* BI) {
    if (BI->isUnconditional()) {
        // Mark the edge as executable
        markEdgeExecutable(BI->getParent(), BI->getSuccessor(0));
        return;
    }
    
    // Conditional branch
    LatticeCell cond = getLatticeValue(BI->getCondition());
    
    if (cond.isTop()) {
        // Condition unknown, don't mark any edges yet
        return;
    }
    
    if (cond.isBottom()) {
        // Condition is not constant, both edges are possible
        markEdgeExecutable(BI->getParent(), BI->getSuccessor(0));
        markEdgeExecutable(BI->getParent(), BI->getSuccessor(1));
        return;
    }
    
    // Condition is constant
    if (cond.isConstant()) {
        if (ConstantInt* condInt = dyn_cast<ConstantInt>(cond.constant)) {
            if (condInt->isOne()) {
                markEdgeExecutable(BI->getParent(), BI->getSuccessor(0));
            } else {
                markEdgeExecutable(BI->getParent(), BI->getSuccessor(1));
            }
        }
    }
}

void SCCPPass::visitSwitchInst(SwitchInst* SI) {
    LatticeCell cond = getLatticeValue(SI->getCondition());
    
    if (cond.isTop()) {
        // Condition unknown
        return;
    }
    
    if (cond.isBottom()) {
        // Condition is not constant, all edges are possible
        for (auto& CaseIt : SI->cases()) {
            markEdgeExecutable(SI->getParent(), CaseIt.getCaseSuccessor());
        }
        markEdgeExecutable(SI->getParent(), SI->getDefaultDest());
        return;
    }
    
    // Condition is constant
    if (cond.isConstant()) {
        if (ConstantInt* condInt = dyn_cast<ConstantInt>(cond.constant)) {
            for (auto& CaseIt : SI->cases()) {
                if (CaseIt.getCaseValue() == condInt) {
                    markEdgeExecutable(SI->getParent(), CaseIt.getCaseSuccessor());
                    return;
                }
            }
            // No case matched, take default
            markEdgeExecutable(SI->getParent(), SI->getDefaultDest());
        }
    }
}

void SCCPPass::visitReturnInst(ReturnInst* RI) {
    // Return doesn't produce a value, nothing to do
}

bool SCCPPass::rewriteCode(Function& F) {
    bool modified = false;
    std::vector<Instruction*> toDelete;
    std::vector<std::pair<BranchInst*, BasicBlock*>> branchesToReplace;
    std::vector<std::pair<SwitchInst*, BasicBlock*>> switchesToReplace;
    
    for (Instruction& I : instructions(F)) {
        // Skip if block is not executable
        if (!isBlockExecutable(I.getParent())) {
            continue;
        }
        
        LatticeCell cell = getLatticeValue(&I);
        
        // Replace instructions with constants
        if (cell.isConstant() && cell.constant) {
            // Don't replace terminators
            if (I.isTerminator()) continue;
            
            I.replaceAllUsesWith(cell.constant);
            toDelete.push_back(&I);
            modified = true;
            continue;
        }
        
        // Collect conditional branches to replace
        if (BranchInst* BI = dyn_cast<BranchInst>(&I)) {
            if (BI->isConditional()) {
                LatticeCell cond = getLatticeValue(BI->getCondition());
                if (cond.isConstant()) {
                    if (ConstantInt* condInt = dyn_cast<ConstantInt>(cond.constant)) {
                        BasicBlock* takenSucc = condInt->isOne() ? 
                            BI->getSuccessor(0) : BI->getSuccessor(1);
                        
                        branchesToReplace.push_back({BI, takenSucc});
                        modified = true;
                    }
                }
            }
        }
        
        // Collect switches to replace
        if (SwitchInst* SI = dyn_cast<SwitchInst>(&I)) {
            LatticeCell cond = getLatticeValue(SI->getCondition());
            if (cond.isConstant()) {
                if (ConstantInt* condInt = dyn_cast<ConstantInt>(cond.constant)) {
                    BasicBlock* takenSucc = SI->getDefaultDest();
                    
                    for (auto& CaseIt : SI->cases()) {
                        if (CaseIt.getCaseValue() == condInt) {
                            takenSucc = CaseIt.getCaseSuccessor();
                            break;
                        }
                    }
                    
                    switchesToReplace.push_back({SI, takenSucc});
                    modified = true;
                }
            }
        }
    }
    
    // Replace branches with unconditional branches  
    for (auto& pair : branchesToReplace) {
        BranchInst* BI = pair.first;
        BasicBlock* takenSucc = pair.second;
        
        // Use ReplaceInstWithInst which handles everything atomically
        BranchInst* NewBr = BranchInst::Create(takenSucc);
        ReplaceInstWithInst(BI, NewBr);
    }
    
    // Replace switches with unconditional branches
    for (auto& pair : switchesToReplace) {
        SwitchInst* SI = pair.first;
        BasicBlock* takenSucc = pair.second;
        
        // Use ReplaceInstWithInst which handles everything atomically
        BranchInst* NewBr = BranchInst::Create(takenSucc);
        ReplaceInstWithInst(SI, NewBr);
    }
    
    // Delete dead instructions
    for (Instruction* I : toDelete) {
        I->eraseFromParent();
    }
    
    return modified;
}

bool SCCPPass::removeUnreachableBlocks(Function& F) {
    std::vector<BasicBlock*> toDelete;
    
    for (BasicBlock& BB : F) {
        if (&BB == &F.getEntryBlock()) continue; // Never remove entry
        if (!isBlockExecutable(&BB)) {
            toDelete.push_back(&BB);
        }
    }
    
    if (toDelete.empty()) {
        return false;
    }
    
    // Phase 1: Remove unreachable blocks from successors' PHI nodes
    // and replace all uses of their instructions with undef.
    for (BasicBlock* BB : toDelete) {
        for (BasicBlock* Succ : successors(BB)) {
            Succ->removePredecessor(BB);
        }
        for (Instruction& I : *BB) {
            if (!I.getType()->isVoidTy() && !I.use_empty()) {
                I.replaceAllUsesWith(UndefValue::get(I.getType()));
            }
        }
    }
    
    // Phase 2: Drop all references so cross-block operand uses are cleared.
    for (BasicBlock* BB : toDelete) {
        BB->dropAllReferences();
    }
    
    // Phase 3: Erase the blocks.
    for (BasicBlock* BB : toDelete) {
        BB->eraseFromParent();
    }
    
    return true;
}
