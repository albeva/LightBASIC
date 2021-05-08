//
// Created by Albert Varaksin on 03/07/2020.
//
#pragma once
#include "Type/Type.def.h"

// clang-format off

#define TOKEN_GENERAL(_) \
    _( Invalid,         "Invalid"     ) \
    _( EndOfFile,       "End-Of-File" ) \
    _( EndOfStmt,       "End-Of-StmtFirst" ) \
    _( Identifier,      "Identifier"  )

#define TOKEN_LITERALS(_) \
    _( StringLiteral,        "String-Literal"         ) \
    _( IntegerLiteral ,      "Number-Literal"         ) \
    _( FloatingPointLiteral, "Floating-point-literal" ) \
    _( BooleanLiteral,       "Boolean-Literal"        )

#define TOKEN_SYMBOLS(_) \
    _( Assign,          "="   ) \
    _( Comma,           ","   ) \
    _( Period,          "."   ) \
    _( ParenOpen,       "("   ) \
    _( ParenClose,      ")"   ) \
    _( BracketOpen,     "["   ) \
    _( BracketClose,    "]"   ) \
    _( Ellipsis,        "..." )

#define TOKEN_OPERATORS(_) \
    _( Plus,            "+",   1, Binary, Left  ) \
    _( Minus,           "-",   1, Binary, Left  ) \
    _( Modulus,         "MOD", 2, Binary, Left  ) \
    _( Multiply,        "*",   3, Binary, Left  ) \
    _( Divide,          "/",   3, Binary, Left  ) \
    _( Exponent,        "^",   4, Binary, Right ) \
    _( Negate,          "-",   5, Unary,  Left  ) \
    _( Factorial,       "!",   5, Unary,  Right )

#define TOKEN_KEYWORDS(_)   \
    _( Dim,             "DIM"      ) \
    _( Var,             "VAR"      ) \
    _( As,              "AS"       ) \
    _( Declare,         "DECLARE"  ) \
    _( Function,        "FUNCTION" ) \
    _( Sub,             "SUB"      ) \
    _( End,             "END"      ) \
    _( Return,          "RETURN"   ) \
    _( If,              "IF"       ) \
    _( Then,            "THEN"     ) \
    _( Else,            "ELSE"     ) \
    _( For,             "FOR"      ) \
    _( To,              "TO"       ) \
    _( Step,            "STEP"     ) \
    _( Next,            "NEXT"     ) \
    _( Continue,        "CONTINUE" ) \
    _( Exit,            "EXIT"     ) \
    _( True,            "TRUE",    ) \
    _( False,           "FALSE",   ) \
    _( Null,            "NULL",    )

// All tokens combined
#define ALL_TOKENS(_)  \
    TOKEN_GENERAL(_)   \
    TOKEN_LITERALS(_)  \
    TOKEN_SYMBOLS(_)   \
    TOKEN_OPERATORS(_) \
    TOKEN_KEYWORDS(_)  \
    ALL_TYPES(_)
