//
// Created by Albert on 03/07/2020.
//
#include "pch.h"
#include "llvm/Support/CommandLine.h"
#include "Parser/Parser.h"
#include "Ast/Ast.h"
#include "Ast/AstVisitor.h"
#include "Ast/AstPrinter.h"
#include "Gen/CodeGen.h"

namespace cl = llvm::cl;
namespace fs = std::filesystem;
using namespace lbc;

int main(int argc, char *argv[]) {
    // command line options
    cl::opt<string> inputFile(cl::Positional, cl::Required, cl::desc("<input file>"));
    if (!cl::ParseCommandLineOptions(argc, argv)) {
        return EXIT_FAILURE;
    }

    // passed file, check it exists & is a valid file
    auto path = fs::path(inputFile.getValue());
    if (!fs::exists(path)) {
        auto message = "File '"s + path.string() + "' not found";
        std::cerr << message << std::endl;
        return EXIT_FAILURE;
    }

    if (!fs::is_regular_file(path)) {
        auto message = "File '"s + path.string() + "' is not a regular file";
        std::cerr << message << std::endl;
        return EXIT_FAILURE;
    }

    // Load
    llvm::SourceMgr srcMgr{};
    srcMgr.setIncludeDirs({fs::current_path()});
    string included;
    auto ID = srcMgr.AddIncludeFile(path, {}, included);
    if (ID == ~0U) {
        auto message = "Failed to load '"s + path.string() + "'";
        std::cerr << message << std::endl;
        return EXIT_FAILURE;
    }

    // Lex the input
    Parser parser{srcMgr, ID};
    if (auto ast = parser.parse()) {
//        CodeGen gen;
//        ast->accept(&gen);
        AstPrinter printer;
        ast->accept(&printer);
    } else {
        std::cerr << "Failed to parse the input" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

