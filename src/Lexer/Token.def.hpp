//
// Created by Albert Varaksin on 03/07/2020.
//
#pragma once
#include "Type/Type.def.hpp"

// clang-format off

#define TOKEN_GENERAL(_) \
    _( Invalid,    "Invalid"     ) \
    _( EndOfFile,  "End-Of-File" ) \
    _( EndOfStmt,  "End-Of-Stmt" ) \
    _( Identifier, "Identifier"  )

#define TOKEN_LITERALS(_) \
    _( StringLiteral,        "String-Literal"         ) \
    _( IntegerLiteral ,      "Integer-Literal"        ) \
    _( FloatingPointLiteral, "Floating-Point-Literal" ) \
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
    /* ID               Str     Prec    Type    Assoc   Kind       */ \
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

#define TOKEN_KEYWORDS(_) \
    _( Any,      "ANY"      ) \
    _( As,       "AS"       ) \
    _( Continue, "CONTINUE" ) \
    _( Declare,  "DECLARE"  ) \
    _( Do,       "DO"       ) \
    _( Else,     "ELSE"     ) \
    _( End,      "END"      ) \
    _( Exit,     "EXIT"     ) \
    _( False,    "FALSE"    ) \
    _( For,      "FOR"      ) \
    _( Function, "FUNCTION" ) \
    _( If,       "IF"       ) \
    _( Loop,     "LOOP"     ) \
    _( Next,     "NEXT"     ) \
    _( Null,     "NULL"     ) \
    _( Ptr,      "PTR"      ) \
    _( Return,   "RETURN"   ) \
    _( Step,     "STEP"     ) \
    _( Sub,      "SUB"      ) \
    _( Then,     "THEN"     ) \
    _( To,       "TO"       ) \
    _( True,     "TRUE"     ) \
    _( Until,    "UNTIL"    ) \
    _( Var,      "VAR"      ) \
    _( While,    "WHILE"    )

#define TOKEN_OPERAOTR_KEYWORD_MAP(_) \
    _( LogicalNot ) \
    _( Modulus    ) \
    _( LogicalAnd ) \
    _( LogicalOr  )

// All tokens combined
#define ALL_TOKENS(_) \
    TOKEN_GENERAL(_)   \
    TOKEN_LITERALS(_)  \
    TOKEN_SYMBOLS(_)   \
    TOKEN_OPERATORS(_) \
    TOKEN_KEYWORDS(_)  \
    ALL_TYPES(_)
