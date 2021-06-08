# lbc library
add_library(
    lbc_lib
    pch.hpp
    Ast/Ast.cpp
    Ast/Ast.def.hpp
    Ast/Ast.hpp
    Ast/AstPrinter.cpp
    Ast/AstPrinter.hpp
    Ast/AstVisitor.h
    Ast/CodePrinter.cpp
    Ast/CodePrinter.hpp
    Ast/ControlFlowStack.hpp
    Ast/ValueFlags.hpp
    Diag/DiagnosticEngine.cpp
    Diag/DiagnosticEngine.hpp
    Driver/CmdLineParser.cpp
    Driver/CmdLineParser.hpp
    Driver/Context.cpp
    Driver/Context.hpp
    Driver/Driver.cpp
    Driver/Driver.hpp
    Driver/Source.hpp
    Driver/TempFileCache.cpp
    Driver/TempFileCache.hpp
    Driver/Toolchain/ToolTask.cpp
    Driver/Toolchain/ToolTask.hpp
    Driver/Toolchain/Toolchain.cpp
    Driver/Toolchain/Toolchain.hpp
    Driver/TranslationUnit.hpp
    Gen/Builders/BinaryExprBuilder.cpp
    Gen/Builders/BinaryExprBuilder.hpp
    Gen/Builders/Builder.hpp
    Gen/Builders/DoLoopBuilder.cpp
    Gen/Builders/DoLoopBuilder.hpp
    Gen/Builders/ForStmtBuilder.cpp
    Gen/Builders/ForStmtBuilder.hpp
    Gen/Builders/IfStmtBuilder.cpp
    Gen/Builders/IfStmtBuilder.hpp
    Gen/CodeGen.cpp
    Gen/CodeGen.hpp
    Gen/Helpers.cpp
    Gen/Helpers.hpp
    Gen/ValueHandler.cpp
    Gen/ValueHandler.hpp
    Lexer/Lexer.cpp
    Lexer/Lexer.hpp
    Lexer/Token.cpp
    Lexer/Token.def.hpp
    Lexer/Token.hpp
    Parser/Parser.cpp
    Parser/Parser.hpp
    Sem/Passes/ConstantFoldingPass.cpp
    Sem/Passes/ConstantFoldingPass.hpp
    Sem/Passes/ForStmtPass.cpp
    Sem/Passes/ForStmtPass.hpp
    Sem/Passes/FuncDeclarerPass.cpp
    Sem/Passes/FuncDeclarerPass.hpp
    Sem/Passes/TypeDeclPass.cpp
    Sem/Passes/TypeDeclPass.hpp
    Sem/Passes/TypePass.cpp
    Sem/Passes/TypePass.hpp
    Sem/SemanticAnalyzer.cpp
    Sem/SemanticAnalyzer.hpp
    Symbol/Symbol.hpp
    Symbol/SymbolTable.cpp
    Symbol/SymbolTable.hpp
    Type/Type.cpp
    Type/Type.def.hpp
    Type/Type.hpp
    Type/TypeUdt.cpp
    Type/TypeUdt.hpp
    Utils/Utils.cpp
    Utils/Utils.hpp
    Utils/Version.hpp
)

target_precompile_headers(project_options INTERFACE pch.hpp)
target_include_directories(lbc_lib PUBLIC ${lbc_SOURCE_DIR}/src)
configure_llvm(lbc_lib)
target_link_libraries(lbc_lib PUBLIC project_options project_warnings)

# lbc executable
add_executable(lbc main.cpp)
target_link_libraries(lbc PRIVATE lbc_lib)
set_target_properties(lbc PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin)
