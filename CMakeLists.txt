cmake_minimum_required(VERSION 3.16)
project(lbc)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# Boost
find_package(Boost 1.73.0 REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

# LLVM
find_package(LLVM REQUIRED CONFIG)
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs core support bitwriter)

# lbc
add_executable(lbc
        src/main.cpp
        src/pch.h
        src/Lexer/Lexer.cpp
        src/Lexer/Lexer.h
        src/Lexer/Token.cpp
        src/Lexer/Token.h
        src/Utils/Utils.h
        src/Parser/Parser.cpp
        src/Parser/Parser.h
        src/Lexer/Token.def.h
        src/Ast/Ast.cpp
        src/Ast/Ast.h
        src/Ast/AstVisitor.h
        src/Ast/Ast.def.h
        src/Ast/AstPrinter.cpp
        src/Ast/AstPrinter.h
        src/Gen/CodeGen.cpp
        src/Gen/CodeGen.h
        src/Symbol/SymbolTable.cpp
        src/Symbol/SymbolTable.h
        src/Symbol/Symbol.cpp
        src/Symbol/Symbol.h
        src/Type/Type.cpp
        src/Type/Type.h src/Type/Type.def.h)
include_directories(${lbc_SOURCE_DIR}/src)
target_precompile_headers(lbc PRIVATE src/pch.h)
target_link_libraries(lbc ${llvm_libs})
target_compile_options(lbc PRIVATE
        -fno-exceptions -fno-rtti
        -Werror -Wall -Wextra -pedantic-errors -Wconversion -Wsign-conversion
        -Wno-unused-private-field
        -Wno-gnu-zero-variadic-macro-arguments
        -Wno-vla-extension
        -Wno-unused-variable
        -Wno-unused-parameter
        -Wno-unused-function
        )
