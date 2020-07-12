add_executable(
    lbc
    main.cpp
    pch.h
    Lexer/Lexer.cpp
    Lexer/Lexer.h
    Lexer/Token.cpp
    Lexer/Token.h
    Utils/Utils.h
    Parser/Parser.cpp
    Parser/Parser.h
    Lexer/Token.def.h
    Ast/Ast.cpp
    Ast/Ast.h
    Ast/AstVisitor.cpp
    Ast/AstVisitor.h
    Ast/Ast.def.h
    Ast/AstPrinter.cpp
    Ast/AstPrinter.h
    Gen/CodeGen.cpp
    Gen/CodeGen.h
    Symbol/SymbolTable.cpp
    Symbol/SymbolTable.h
    Symbol/Symbol.cpp
    Symbol/Symbol.h
    Type/Type.cpp
    Type/Type.h
    Type/Type.def.h
    Sem/SemanticAnalyzer.cpp
    Sem/SemanticAnalyzer.h)

target_include_directories(lbc SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
target_include_directories(lbc PRIVATE ${lbc_SOURCE_DIR}/src)
target_link_libraries(lbc PRIVATE ${llvm_libs} project_options project_warnings)
add_definitions(${LLVM_DEFINITIONS})
