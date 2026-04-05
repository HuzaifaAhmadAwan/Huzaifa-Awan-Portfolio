#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <filesystem>

#include "diagnostic.h"
#include "lexer.h"
#include "parser.h"
#include "alias_analysis.h"
#include "sccp.h"

// LLVM headers for loading IR
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"

// Include the alias printing utility
#include "../Alias_Analysis_Helper_Example/alias_print.h"

using namespace std::literals::string_literals;
namespace fs = std::filesystem;

enum class Mode {
    TOKENIZE,
    PARSE,
    PRETTYPRINT,
    COMPILE,
    OPTIMIZE,
    ANALYZE_ALIAS,
};

int main(int argc, const char *argv[]) {
    Mode mode = Mode::COMPILE;  // Default mode is now compile
    std::string FileName;
    bool analyzeAlias = false;

    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (argv[i] == "--tokenize"s) {
                mode = Mode::TOKENIZE;
            } else if (argv[i] == "--parse"s) {
                mode = Mode::PARSE;
            } else if (argv[i] == "--print-ast"s) {
                mode = Mode::PRETTYPRINT;
            } else if (argv[i] == "--compile"s) {
                mode = Mode::COMPILE;
            } else if (argv[i] == "--optimize"s) {
                mode = Mode::OPTIMIZE;
            } else if (argv[i] == "--analyze-alias"s) {
                analyzeAlias = true;
                mode = Mode::ANALYZE_ALIAS;
            } else {
                error("Unknown option: ", argv[i]);
            }
        } else if (FileName.empty()) {
            FileName = argv[i];
        } else {
            error("Too many positional arguments!");
        }
    }

    if (FileName.empty()) {
        error("No file name given!");
    }

    // Check if this is an LLVM IR file
    fs::path filePath(FileName);
    bool isLLVMIR = (filePath.extension() == ".ll");

    // If --analyze-alias is given, we need an LLVM IR file
    if (analyzeAlias && !isLLVMIR) {
        error("--analyze-alias requires an LLVM IR (.ll) file!");
    }

    // Handle LLVM IR file with alias analysis
    if (isLLVMIR && analyzeAlias) {
        llvm::LLVMContext Ctx; // LLVM context for parsing
        llvm::SMDiagnostic Err{}; // For capturing parsing errors
        std::unique_ptr<llvm::Module> M = llvm::parseIRFile(FileName, Err, Ctx); // Parse the IR file
        
        if (!M) {
            Err.print(argv[0], llvm::errs()); 
            error("Failed to parse LLVM IR file!");
        }

        // Run points-to alias analysis
        PointsToAliasAnalysis AA; // Create analysis instance
        AA.analyze(*M); // Analyze the module

        // Print alias sets using the provided utility
        aliasPrint::dumpAliasSets(*M, AA, llvm::outs()); // Use LLVM's outs() for output

        return EXIT_SUCCESS;
    }

    // Handle LLVM IR file with optimization
    if (isLLVMIR && mode == Mode::OPTIMIZE) {
        llvm::LLVMContext Ctx;
        llvm::SMDiagnostic Err{};
        std::unique_ptr<llvm::Module> M = llvm::parseIRFile(FileName, Err, Ctx);
        
        if (!M) {
            Err.print(argv[0], llvm::errs());
            error("Failed to parse LLVM IR file!");
        }

        // Apply mem2reg pass to promote allocas to registers (SSA construction)
        llvm::legacy::FunctionPassManager FPM(M.get());
        FPM.add(llvm::createPromoteMemoryToRegisterPass());
        FPM.doInitialization();
        
        for (llvm::Function& F : *M) {
            if (!F.isDeclaration()) {
                FPM.run(F);
            }
        }
        FPM.doFinalization();

        // Run SCCP optimization pass
        SCCPPass sccp;
        sccp.runOnModule(*M);

        // Verify module after optimization
        llvm::verifyModule(*M);

        // Generate output filename: input_opt.ll
        fs::path inputPath(FileName);
        std::string baseName = inputPath.stem().string();
        std::string outFileName = baseName + "_opt.ll";
        
        std::error_code EC;
        llvm::raw_fd_ostream stream(outFileName, EC, llvm::sys::fs::OF_Text);
        if (EC) {
            error("Failed to open output file: ", outFileName);
        }

        M->print(stream, nullptr);
        
        std::cout << "Generated optimized IR: " << outFileName << std::endl;
        return EXIT_SUCCESS;
    }

    // Handle regular C source files
    std::ifstream File(FileName, std::ios::in | std::ios::binary);
    if (!File) {
        error("Cannot open file `", FileName, "'!");
    }

    Lexer lexer(File, FileName);

    if (mode == Mode::TOKENIZE) {
        while (lexer.peek().Kind != TK_EOI) {
            std::cout << lexer.peek() << '\n';
            lexer.pop();
        }
        return EXIT_SUCCESS;
    }

    Parser parser(lexer);

    std::unique_ptr<TranslationUnit> tunit = parser.parse();

    //Perform semantic analysis for --parse, --print-ast, --compile, and --optimize modes
    if (mode == Mode::PARSE || mode == Mode::PRETTYPRINT || mode == Mode::COMPILE || mode == Mode::OPTIMIZE) {
        tunit->checkSemantics();
    }

    if (mode == Mode::PARSE) {
        return EXIT_SUCCESS;
    }

    if (mode == Mode::PRETTYPRINT) {
        tunit->print(std::cout);
        return EXIT_SUCCESS;
    }

    if (mode == Mode::COMPILE) {
        // generate code without optimization
        tunit->codeGen(FileName, false);
        return EXIT_SUCCESS;
    }

    if (mode == Mode::OPTIMIZE) {
        // generate code with optimization
        tunit->codeGen(FileName, true);
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}