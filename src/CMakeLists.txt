# lbc library
add_library(
    lbc_lib
    pch.h
    Ast/Ast.cpp
    Ast/Ast.def.h
    Ast/Ast.h
    Ast/AstPrinter.cpp
    Ast/AstPrinter.h
    Ast/AstVisitor.h
    Ast/CodePrinter.cpp
    Ast/CodePrinter.h
    Ast/ControlFlowStack.h
    Driver/CmdLineParser.cpp
    Driver/CmdLineParser.h
    Driver/Context.cpp
    Driver/Context.h
    Driver/Driver.cpp
    Driver/Driver.h
    Driver/Source.h
    Driver/TempFileCache.cpp
    Driver/TempFileCache.h
    Driver/Toolchain/Toolchain.cpp
    Driver/Toolchain/Toolchain.h
    Driver/Toolchain/ToolTask.cpp
    Driver/Toolchain/ToolTask.h
    Driver/TranslationUnit.h
    Gen/Builders/ForStmtBuilder.cpp
    Gen/Builders/ForStmtBuilder.hpp
    Gen/CodeGen.cpp
    Gen/CodeGen.h
    Gen/Helpers.cpp
    Gen/Helpers.hpp
    Gen/ValueHandler.h
    Lexer/Lexer.cpp
    Lexer/Lexer.h
    Lexer/Token.cpp
    Lexer/Token.def.h
    Lexer/Token.h
    Parser/Parser.cpp
    Parser/Parser.h
    Sem/Passes/ConstantFoldingPass.cpp
    Sem/Passes/ConstantFoldingPass.h
    Sem/Passes/FuncDeclarerPass.cpp
    Sem/Passes/FuncDeclarerPass.h
    Sem/Passes/TypePass.cpp
    Sem/Passes/TypePass.h
    Sem/SemanticAnalyzer.cpp
    Sem/SemanticAnalyzer.h
    Symbol/Symbol.h
    Symbol/SymbolTable.cpp
    Symbol/SymbolTable.h
    Type/Type.cpp
    Type/Type.def.h
    Type/Type.h
    Utils/EnumFlags.h
    Utils/Utils.cpp
    Utils/Utils.h
    Utils/Version.h
)

target_precompile_headers(project_options INTERFACE pch.h)
target_include_directories(lbc_lib PUBLIC ${lbc_SOURCE_DIR}/src)
configure_llvm(lbc_lib)
target_link_libraries(lbc_lib PUBLIC project_options project_warnings)

# lbc executable
add_executable(lbc main.cpp)
target_link_libraries(lbc PRIVATE lbc_lib)
set_target_properties(lbc PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin)
