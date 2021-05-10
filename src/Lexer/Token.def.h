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
    _( Ellipsis,        "..." ) \
    _( Assign,          "="   )

#define TOKEN_OPERATORS(_) \
    _( Factorial,       "!",   10, Unary,  Right ) \
                                                   \
    _( Exponent,        "^",   9,  Binary, Right ) \
                                                   \
    _( Negate,          "-",   8,  Unary,  Left  ) \
    _( LogicalNot,      "NOT", 8,  Unary,  Left  ) \
                                                   \
    _( Multiply,        "*",   7,  Binary, Left  ) \
    _( Divide,          "/",   7,  Binary, Left  ) \
                                                   \
    _( Modulus,         "MOD", 6,  Binary, Left  ) \
                                                   \
    _( Plus,            "+",   5,  Binary, Left  ) \
    _( Minus,           "-",   5,  Binary, Left  ) \
                                                   \
    _( Equal,           "=",   4,  Binary, Left  ) \
    _( NotEqual,        "<>",  4,  Binary, Left  ) \
                                                   \
    _( LessThan,        "<",   3,  Binary, Left  ) \
    _( LessOrEqual,     "<=",  3,  Binary, Left  ) \
    _( GreaterThan,     ">",   3,  Binary, Left  ) \
    _( GreaterOrEqual,  ">=",  3,  Binary, Left  ) \
                                                   \
    _( LogicalAnd,      "AND", 2,  Binary, Left  ) \
                                                   \
    _( LogicalOr,       "OR",  1,  Binary, Left  )

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
