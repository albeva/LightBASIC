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
    /* ID               Ch      Prec    Type    Assoc   Kind       */ \
    _( AddressOf,       "@",    10,     Unary,  Left,   Memory      ) \
    _( Dereference,     "*",    10,     Unary,  Left,   Memory      ) \
                                                                      \
    _( Negate,          "-",    9,      Unary,  Left,   Arithmetic  ) \
    _( LogicalNot,      "NOT",  9,      Unary,  Left,   Logical     ) \
                                                                      \
    _( Multiply,        "*",    8,      Binary, Left,   Arithmetic  ) \
    _( Divide,          "/",    8,      Binary, Left,   Arithmetic  ) \
                                                                      \
    _( Modulus,         "MOD",  7,      Binary, Left,   Arithmetic  ) \
                                                                      \
    _( Plus,            "+",    6,      Binary, Left,   Arithmetic  ) \
    _( Minus,           "-",    6,      Binary, Left,   Arithmetic  ) \
                                                                      \
    _( Equal,           "=",    5,      Binary, Left,   Comparison  ) \
    _( NotEqual,        "<>",   5,      Binary, Left,   Comparison  ) \
                                                                      \
    _( LessThan,        "<",    4,      Binary, Left,   Comparison  ) \
    _( LessOrEqual,     "<=",   4,      Binary, Left,   Comparison  ) \
    _( GreaterThan,     ">",    4,      Binary, Left,   Comparison  ) \
    _( GreaterOrEqual,  ">=",   4,      Binary, Left,   Comparison  ) \
                                                                      \
    _( LogicalAnd,      "AND",  3,      Binary, Left,   Logical     ) \
                                                                      \
    _( LogicalOr,       "OR",   2,      Binary, Left,   Logical     ) \
    _( CommaAnd,        ",",    1,      Binary, Left,   Logical     )

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
    _( Null,            "NULL"     ) \
    _( If,              "IF"       ) \
    _( Then,            "THEN"     ) \
    _( Else,            "ELSE"     ) \
    _( For,             "FOR"      ) \
    _( Next,            "NEXT"     ) \
    _( Step,            "STEP"     ) \
    _( To,              "TO"       ) \
    _( Do,              "DO"       ) \
    _( Ptr,             "PTR"      ) \
    _( Any,             "ANY"      )

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
