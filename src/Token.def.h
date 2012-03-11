//
//  Token.def.h
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//

#include "Type.def.h"

//
// TOKEN defines basic token types
//     Name                 String
#define TKN_BASIC(_) \
    _( Invalid,             "Invalid Token"         ) \
    _( EndOfFile,           "End-Of-File"           ) \
    _( EndOfLine,           "End-Of-Line"           ) \
    _( Identifier,          "Identifier"            ) \
    _( IntegerLiteral,      "Numeric-Literal"       ) \
    _( FloatingPointLiteral,"Floating-Point-Literal") \
    _( StringLiteral,       "String-Literal"        )

//
// OPERATOR defines operator tokens
//     Name             String
#define TKN_OPERATOR(_) \
    _( BracketOpen,     "["     ) \
    _( BracketClose,    "]"     ) \
    _( ParenOpen,       "("     ) \
    _( ParenClose,      ")"     ) \
    _( Comma,           ","     ) \
    _( Assign,          "="     ) \
    _( Ellipsis,        "..."   )

//
// KEYWORD defines keyword tokens
//     Name             String
#define TKN_KEYWORD(_) \
    _( Dim,             "DIM"       ) \
    _( As,              "AS"        ) \
    _( Declare,         "DECLARE"   ) \
    _( Function,        "FUNCTION"  ) \
    _( Ptr,             "PTR"       ) \
    _( End,             "END"       ) \
    _( Return,          "RETURN"    )

//
// Types define the type keywords
//
#define TKN_TYPES(_) \
    ALL_TYPES(_)

//
// All tokens
//
#define ALL_TOKENS(_)   \
    TKN_BASIC(_)        \
    TKN_OPERATOR(_)     \
    TKN_KEYWORD(_)      \
    TKN_TYPES(_)
