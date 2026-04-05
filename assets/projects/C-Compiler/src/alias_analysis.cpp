#include "alias_analysis.h"
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/CFG.h>
#include <llvm/Support/Casting.h>
#include <queue>

// Special unknown location for TOP (may point to anything)
static const llvm::Value* UnknownLoc = reinterpret_cast<const llvm::Value*>(0x1);

// Merge two points-to graphs (union of points-to sets)
static PointsToGraph mergeGraphs(const PointsToGraph& G1, const PointsToGraph& G2) {
    PointsToGraph result = G1;
    for (const auto& [ptr, targets] : G2) {
        if (result.count(ptr)) { // If pointer already has targets, merge sets
            result[ptr].insert(targets.begin(), targets.end());
        } else {
            result[ptr] = targets;
        }
    }
    return result;
}

// Check if two graphs are equal
static bool graphsEqual(const PointsToGraph& G1, const PointsToGraph& G2) {
    if (G1.size() != G2.size()) return false;
    for (const auto& [ptr, targets] : G1) {
        auto it = G2.find(ptr);
        if (it == G2.end()) return false;
        if (targets != it->second) return false;
    }
    return true;
}

void PointsToAliasAnalysis::analyze(llvm::Module &M) {
    // Analyze each function in the module
    for (auto &F : M) { // Iterate over functions
        if (!F.isDeclaration()) { // Skip function declarations
            analyzeFunction(F); // Analyze function body
        }
    }
}

void PointsToAliasAnalysis::analyzeFunction(llvm::Function &F) {
    std::map<const llvm::BasicBlock*, PointsToGraph> blockIN; //points-to graph at block entry
    std::map<const llvm::BasicBlock*, PointsToGraph> blockOUT; //points-to graph at block exit
    
    // Initialize with function arguments: each argument points to itself
    PointsToGraph initialPTG;
    for (auto &Arg : F.args()) {
        if (Arg.getType()->isPointerTy()) {
            initialPTG[&Arg] = {&Arg};
        }
    }
    
    // Initialize all blocks
    for (auto &BB : F) {
        blockIN[&BB] = PointsToGraph();
        blockOUT[&BB] = PointsToGraph();
    }
    
    blockIN[&F.getEntryBlock()] = initialPTG; // Entry block starts with initial PTG i.e. arguments
    
    // Worklist algorithm
    std::queue<const llvm::BasicBlock*> worklist;
    std::set<const llvm::BasicBlock*> inWorklist;
    
    // All blocks initially need processing
    for (auto &BB : F) {
        worklist.push(&BB);
        inWorklist.insert(&BB);
    }
    
    while (!worklist.empty()) {
        const llvm::BasicBlock* BB = worklist.front(); // Get next block to process
        worklist.pop();
        inWorklist.erase(BB);
        
        // Compute IN[BB] = merge of OUT of predecessors
        PointsToGraph inPTG;
        bool firstPred = true;
         
        for (const llvm::BasicBlock* Pred : llvm::predecessors(BB)) {
            if (blockOUT.count(Pred)) {
                if (firstPred) {
                    inPTG = blockOUT[Pred];
                    firstPred = false;
                } else {
                    inPTG = mergeGraphs(inPTG, blockOUT[Pred]);
                }
            }
        }
        
        if (firstPred && BB == &F.getEntryBlock()) {
            inPTG = initialPTG;
        }
        
        blockIN[BB] = inPTG;
        
        // Apply transfer functions
        PointsToGraph currentPTG = inPTG;
        for (const auto &I : *BB) {
            currentPTG = transfer(&I, currentPTG);
            pointsToInfo[&I] = currentPTG;
        }
        
        // Check if changed
        bool changed = !blockOUT.count(BB) || !graphsEqual(blockOUT[BB], currentPTG); // If blockOUT[BB] doesn't exist or is different from currentPTG, we have a change
        blockOUT[BB] = currentPTG;
        
        if (changed) { // If OUT changed, add successors to worklist
            for (const llvm::BasicBlock* Succ : llvm::successors(BB)) {
                if (inWorklist.find(Succ) == inWorklist.end()) {  // If successor not already in worklist
                    worklist.push(Succ);
                    inWorklist.insert(Succ);
                }
            }
        }
    }
}

PointsToGraph PointsToAliasAnalysis::transfer(const llvm::Instruction* I, 
                                               const PointsToGraph& InPTG) {
    PointsToGraph OutPTG = InPTG;

    // x <- alloc_p: [[x <- alloc_p]]# P = (P \ x) ∪ {(x, alloc_p)}
    // Alloca: don't add to PTG initially
    // Allocas only get PTG entries when storing pointer values to them
    if (llvm::isa<llvm::AllocaInst>(I)) {
        // No-op: allocas start with no points-to info
    }
    //store: *x <- y: [[*x<-y]]# P = P U {(s,t)|(x,s)∈ P, (y,t)∈ P}
    else if (auto *SI = llvm::dyn_cast<llvm::StoreInst>(I)) {
        const llvm::Value* valueStored = SI->getValueOperand();
        const llvm::Value* pointerDest = SI->getPointerOperand();
        
        if (!valueStored->getType()->isPointerTy()) {
            return OutPTG;
        }
        
        // Get pts(y)
        PointsToSet valueTargets = resolvePointsTo(valueStored, InPTG);
        
        if (llvm::isa<llvm::ConstantPointerNull>(valueStored)) {
            valueTargets = {valueStored};
        }
        else if (llvm::isa<llvm::PtrToIntInst>(valueStored) || 
                 llvm::isa<llvm::IntToPtrInst>(valueStored)) {
            valueTargets = {UnknownLoc};
        }
        
        // Strong update for allocas
        if (llvm::isa<llvm::AllocaInst>(pointerDest)) {
            if (!valueTargets.empty()) {
                OutPTG[pointerDest] = valueTargets;
            }
        }
        // Weak update otherwise
        else {
            PointsToSet destTargets = resolvePointsTo(pointerDest, InPTG);
            for (const auto* destTarget : destTargets) {
                if (!valueTargets.empty()) { // If value has targets
                    if (OutPTG.count(destTarget)) { // If dest already has targets, merge sets
                        OutPTG[destTarget].insert(valueTargets.begin(), valueTargets.end());
                    } else {
                        OutPTG[destTarget] = valueTargets;
                    }
                }
            }
        }
    }
    // x <- y: [[x <- y]]# P = (P \ x) ∪ {(x, t) | (y, t) ∈ P}
    // x <- *y: [[x <- *y]]# P = (P \ x) ∪ {(x, t) | (s, t) ∈ P, (y, s) ∈ P}
    else if (auto *LI = llvm::dyn_cast<llvm::LoadInst>(I)) { // Load instruction
        if (LI->getType()->isPointerTy()) { // Only if loading a pointer
            const llvm::Value* pointerSrc = LI->getPointerOperand(); // y or *y
            
            OutPTG.erase(LI); // Remove old points-to info
            
            if (llvm::isa<llvm::PtrToIntInst>(pointerSrc) || 
                llvm::isa<llvm::IntToPtrInst>(pointerSrc)) { // Casts to/from int
                OutPTG[LI] = {UnknownLoc}; // Lose all points-to info
            }
            else { 
                PointsToSet ptrTargets = resolvePointsTo(pointerSrc, InPTG); // s in *y or y
                PointsToSet resultSet; 
                
                for (const auto* target : ptrTargets) { // For each s
                    PointsToSet innerTargets = resolvePointsTo(target, InPTG); // t in (s, t)
                    resultSet.insert(innerTargets.begin(), innerTargets.end()); // Collect t's
                }
                
                if (!resultSet.empty()) { // If we found targets
                    OutPTG[LI] = resultSet; // x points to collected targets
                }
            }
        }
    }
    // x <- gep y, ...: [[x <- gep y, ...]]# P = (P \ x) ∪ {(x, t) | (y, t) ∈ P}
    else if (auto *GEP = llvm::dyn_cast<llvm::GetElementPtrInst>(I)) { // GEP instruction
        const llvm::Value* ptrOp = GEP->getPointerOperand(); // y in gep y, ...
        PointsToSet pts = resolvePointsTo(ptrOp, InPTG); // y's points-to set
        OutPTG.erase(GEP); // Remove old points-to info (P \ x)
        if (!pts.empty()) { // If non-empty
            OutPTG[GEP] = pts; // x points to what y points to
        }
    }
    // Handle casts
    else if (auto *Cast = llvm::dyn_cast<llvm::CastInst>(I)) { // Cast instruction
        OutPTG.erase(Cast); // Remove old points-to info (P \ x)
        if (llvm::isa<llvm::PtrToIntInst>(Cast) || llvm::isa<llvm::IntToPtrInst>(Cast)) { // Casts to/from int
            OutPTG[Cast] = {UnknownLoc}; // Lose all points-to info
        }
        else if (Cast->getType()->isPointerTy()) { // Cast to pointer
            const llvm::Value* srcPtr = Cast->getOperand(0); // Source pointer
            if (srcPtr->getType()->isPointerTy()) { // Source is pointer
                PointsToSet pts = resolvePointsTo(srcPtr, InPTG); // Source's points-to set
                if (!pts.empty()) { // If non-empty
                    OutPTG[Cast] = pts; // x points to what source points to
                }
            }
        }
    }    
    // PHI nodes
    // x <- phi(...): [[x <- phi(...)]], for each incoming value v: P = P U {(x, t) | (v, t) ∈ P}
    else if (auto *PHI = llvm::dyn_cast<llvm::PHINode>(I)) {
        if (PHI->getType()->isPointerTy()) {
            OutPTG.erase(PHI); // Remove old points-to info (P \ x)
            PointsToSet resultSet; 
            for (unsigned i = 0; i < PHI->getNumIncomingValues(); ++i) { // For each incoming value
                const llvm::Value* incomingVal = PHI->getIncomingValue(i);
                PointsToSet pts = resolvePointsTo(incomingVal, InPTG); // Incoming value's points-to set
                resultSet.insert(pts.begin(), pts.end()); // Collect targets
            }
            if (!resultSet.empty()) {
                OutPTG[PHI] = resultSet; // x points to collected targets
            }
        }
    }

    return OutPTG; // Return updated graph
} 

PointsToSet PointsToAliasAnalysis::getPointsToSet(const llvm::Value* V, 
                                                   const PointsToGraph& PTG) const {
    auto it = PTG.find(V); // Find value in graph 
    if (it != PTG.end()) { // If found
        return it->second; // Return points-to set
    }
    return {};
}

PointsToSet PointsToAliasAnalysis::resolvePointsTo(const llvm::Value* V, 
                                                    const PointsToGraph& PTG) const {
    // Check graph first                                                    
    if (PTG.count(V)) { 
        return PTG.at(V);
    }
    
    // Allocas and arguments point to themselves if not in graph
    if (llvm::isa<llvm::AllocaInst>(V) || llvm::isa<llvm::Argument>(V)) {
        return {V};
    }
    
    return {};
}

llvm::AliasResult PointsToAliasAnalysis::alias(const llvm::Value* V1, 
                                               const llvm::Value* V2,
                                               const llvm::Instruction* CtxI) const {
    if (V1 == V2) { // Same value must alias
        return llvm::AliasResult::MustAlias;
    }
    
    // Two different allocas never alias
    if (llvm::isa<llvm::AllocaInst>(V1) && llvm::isa<llvm::AllocaInst>(V2)) {
        return llvm::AliasResult::NoAlias;
    }
    
    // Two different arguments never alias
    if (llvm::isa<llvm::Argument>(V1) && llvm::isa<llvm::Argument>(V2)) {
        return llvm::AliasResult::NoAlias;
    }
    
    // Alloca and argument never alias
    if ((llvm::isa<llvm::AllocaInst>(V1) && llvm::isa<llvm::Argument>(V2)) ||
        (llvm::isa<llvm::Argument>(V1) && llvm::isa<llvm::AllocaInst>(V2))) {
        return llvm::AliasResult::NoAlias;
    }
    
    auto it = pointsToInfo.find(CtxI); // Get points-to info at context instruction
    if (it == pointsToInfo.end()) { // No points-to info available
        return llvm::AliasResult::MayAlias;
    }
    
    const PointsToGraph& PTG = it->second; // Points-to graph at context
    
    // For allocas/arguments, treat them as pointing to themselves
    // For other values, look up their points-to set in the PTG
    PointsToSet pts1, pts2;
    
    if (llvm::isa<llvm::AllocaInst>(V1) || llvm::isa<llvm::Argument>(V1)) {
        pts1 = {V1};
    } else {
        pts1 = resolvePointsTo(V1, PTG);
    }
    
    if (llvm::isa<llvm::AllocaInst>(V2) || llvm::isa<llvm::Argument>(V2)) {
        pts2 = {V2};
    } else {
        pts2 = resolvePointsTo(V2, PTG);
    }
    
    if (pts1.count(UnknownLoc) || pts2.count(UnknownLoc)) {
        return llvm::AliasResult::MayAlias;
    }
    
    if (pts1.empty() || pts2.empty()) {
        return llvm::AliasResult::NoAlias;
    }
    
    // Check for intersection
    bool hasIntersection = false;
    for (const auto* loc : pts1) {
        if (pts2.count(loc)) {
            hasIntersection = true;
            break;
        }
    }
    
    if (!hasIntersection) {
        return llvm::AliasResult::NoAlias;
    }
    
    // MustAlias if both point to exactly the same single location
    if (pts1.size() == 1 && pts2.size() == 1 && pts1 == pts2) {
        return llvm::AliasResult::MustAlias;
    }
    
    return llvm::AliasResult::MayAlias;
}