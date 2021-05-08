# Sources
add_executable(
    lbc
    main.cpp
    pch.h
    Ast/Ast.cpp
    Ast/Ast.def.h
    Ast/Ast.h
    Ast/AstPrinter.cpp
    Ast/AstPrinter.h
    Ast/AstVisitor.h
    Driver/CmdLineParser.cpp
    Driver/CmdLineParser.h
    Driver/Context.cpp
    Driver/Context.h
    Driver/Driver.cpp
    Driver/Driver.h
    Driver/Source.h
    Driver/TempFileCache.cpp
    Driver/TempFileCache.h
    Driver/Toolchain/ToolTask.cpp
    Driver/Toolchain/ToolTask.h
    Driver/Toolchain/Toolchain.cpp
    Driver/Toolchain/Toolchain.h
    Gen/CodeGen.cpp
    Gen/CodeGen.h
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
    Sem/SemanticAnalyzer.cpp
    Sem/SemanticAnalyzer.h
    Symbol/Symbol.cpp
    Symbol/Symbol.h
    Symbol/SymbolTable.cpp
    Symbol/SymbolTable.h
    Type/Type.cpp
    Type/Type.def.h
    Type/Type.h
    Utils/Utils.cpp
    Utils/Utils.h
    Utils/Version.h
    Driver/TranslationUnit.h)

# LLVM
find_package(LLVM REQUIRED CONFIG)
llvm_map_components_to_libnames(
    llvm_libs
    core
    support
    bitwriter
    transformUtils
)

target_include_directories(lbc SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# PCH header
target_precompile_headers(project_options INTERFACE pch.h)

# Source includes
target_include_directories(lbc PRIVATE ${lbc_SOURCE_DIR}/src)

# build lbc
target_link_libraries(lbc PRIVATE ${llvm_libs} project_options project_warnings)
