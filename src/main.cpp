//
// Created by Albert on 03/07/2020.
//
#include "pch.h"
#include "Ast/Ast.h"
#include "Ast/AstPrinter.h"
#include "Ast/AstVisitor.h"
#include "Gen/CodeGen.h"
#include "Parser/Parser.h"
#include "Driver/Driver.h"
#include "Sem/SemanticAnalyzer.h"
#include "llvm/Support/CommandLine.h"

namespace cl = llvm::cl;
using namespace lbc;

int main(int argc, char* argv[]) {
    // command line options
    cl::opt<string> inputFile(cl::Positional, cl::Required, cl::desc("<input file>"));
    if (!cl::ParseCommandLineOptions(argc, argv)) {
        return EXIT_FAILURE;
    }

    // passed file, check it exists & is a valid file
    auto inputPath = fs::path(inputFile.getValue());
    if (!fs::exists(inputPath)) {
        auto message = "File '"s + inputPath.string() + "' not found";
        std::cerr << message << std::endl;
        return EXIT_FAILURE;
    }

    if (!fs::is_regular_file(inputPath)) {
        auto message = "File '"s + inputPath.string() + "' is not a regular file";
        std::cerr << message << std::endl;
        return EXIT_FAILURE;
    }

    Driver driver;

    // Load
    auto& srcMgr = driver.getSourceMrg();
    std::vector include_paths{ fs::current_path().string() };
    srcMgr.setIncludeDirs(include_paths);
    string included;
    auto ID = srcMgr.AddIncludeFile(inputPath.string(), {}, included);
    if (ID == ~0U) {
        auto message = "Failed to load '"s + inputPath.string() + "'";
        std::cerr << message << std::endl;
        return EXIT_FAILURE;
    }

    // Lex the input
    Parser parser{ srcMgr, ID };
    if (auto ast = parser.parse()) {
        auto& context = driver.getLlvmContext();
        
        // Analyze
        SemanticAnalyzer sem(context, srcMgr, ID);
        ast->accept(&sem);

        // generate IR
        CodeGen gen(context, srcMgr, ID);
        ast->accept(&gen);

        // write .bc
        fs::path bitcodePath{ inputPath };
        bitcodePath.replace_extension(".bc");
        std::error_code errors{};
        llvm::raw_fd_ostream stream{ bitcodePath.string(), errors };
        llvm::WriteBitcodeToFile(*gen.module(), stream);
        stream.flush();
        stream.close();

        // turn into .o
        fs::path objectPath{ inputPath };
        objectPath.replace_extension(".o");
        std::vector<llvm::StringRef> llcArgs{
            "llc",
            "-filetype=obj",
            "-o",
            objectPath.string(),
            bitcodePath.string()
        };
        llvm::sys::ExecuteAndWait("/usr/local/bin/llc", llcArgs);

        // generate executable
        fs::path binaryPath{ inputPath };
        binaryPath.replace_extension(".a");
        std::vector<llvm::StringRef> ldArgs{
            "ld",
            "-lSystem",
            "-o",
            binaryPath.string(),
            objectPath.string()
        };
        llvm::sys::ExecuteAndWait("/usr/bin/ld", ldArgs);

        // clean up
        fs::remove(objectPath);
        fs::remove(bitcodePath);
    } else {
        std::cerr << "Failed to parse the input" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
