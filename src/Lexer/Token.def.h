//
// Created by Albert Varaksin on 03/07/2020.
//
#pragma once
#include "Type/Type.def.h"

// clang-format off

#define TOKEN_GENERAL(_) \
    _( Invalid,         "Invalid"          ) \
    _( EndOfFile,       "End-Of-File"      ) \
    _( EndOfStmt,       "End-Of-StmtFirst" ) \
    _( Identifier,      "Identifier"       )

#define TOKEN_LITERALS(_) \
    _( StringLiteral,        "String-Literal"         ) \
    _( IntegerLiteral ,      "Number-Literal"         ) \
    _( FloatingPointLiteral, "Floating-point-literal" ) \
    _( BooleanLiteral,       "Boolean-Literal"        ) \
    _( NullLiteral,          "Null-Literal"           )

#define TOKEN_SYMBOLS(_) \
    _( Comma,           ","   ) \
    _( Period,          "."   ) \
    _( ParenOpen,       "("   ) \
    _( ParenClose,      ")"   ) \
    _( BracketOpen,     "["   ) \
    _( BracketClose,    "]"   ) \
    _( Ellipsis,        "..." )

#define TOKEN_OPERATORS(_) \
    _( Exponent,        "^",   11, Binary, Right ) \
                                                   \
    _( Factorial,       "!",   10, Unary,  Right ) \
                                                   \
    _( Negate,          "-",   9,  Unary,  Left  ) \
    _( LogicalNot,      "NOT", 9,  Unary,  Left  ) \
                                                   \
    _( Multiply,        "*",   8,  Binary, Left  ) \
    _( Divide,          "/",   8,  Binary, Left  ) \
                                                   \
    _( Modulus,         "MOD", 7,  Binary, Left  ) \
                                                   \
    _( Plus,            "+",   6,  Binary, Left  ) \
    _( Minus,           "-",   6,  Binary, Left  ) \
                                                   \
    _( Equal,           "=",   5,  Binary, Left  ) \
    _( NotEqual,        "<>",  5,  Binary, Left  ) \
                                                   \
    _( LessThan,        "<",   4,  Binary, Left  ) \
    _( LessOrEqual,     "<=",  4,  Binary, Left  ) \
    _( GreaterThan,     ">",   4,  Binary, Left  ) \
    _( GreaterOrEqual,  ">=",  4,  Binary, Left  ) \
                                                   \
    _( LogicalAnd,      "AND", 3,  Binary, Left  ) \
                                                   \
    _( LogicalOr,       "OR",  2,  Binary, Left  ) \
                                                   \
    _( Assign,          "OR",  1,  Binary, Left  )

#define TOKEN_KEYWORDS(_)   \
    _( Var,             "VAR"      ) \
    _( As,              "AS"       ) \
    _( Declare,         "DECLARE"  ) \
    _( Function,        "FUNCTION" ) \
    _( Sub,             "SUB"      ) \
    _( End,             "END"      ) \
    _( Return,          "RETURN"   ) \
    _( True,            "TRUE"     ) \
    _( False,           "FALSE"    ) \
    _( Null,            "NULL"     )

#define TOKEN_OPERAOTR_KEYWORD_MAP(_) \
    _( LogicalNot ) \
    _( Modulus )    \
    _( LogicalAnd ) \
    _( LogicalOr )

// All tokens combined
#define ALL_TOKENS(_)  \
    TOKEN_GENERAL(_)   \
    TOKEN_LITERALS(_)  \
    TOKEN_SYMBOLS(_)   \
    TOKEN_OPERATORS(_) \
    TOKEN_KEYWORDS(_)  \
    ALL_TYPES(_)
