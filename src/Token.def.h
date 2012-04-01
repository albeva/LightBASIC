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
    _( StringLiteral,       "String-Literal"        ) \
    _( BooleanLiteral,      "Boolean-Literal"       ) \
    _( NullLiteral,         "Null-Literal"          )

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
    _( Equal,           "="     ) \
    _( NotEqual,        "<>"    ) \
    _( LessThanEqual,   "<="    ) \
    _( GreaterThanEqual,">="    ) \
    _( LessThan,        "<"     ) \
    _( GreaterThan,     ">"     ) \
    _( AddressOf,       "&"     ) \
    _( Dereference,     "*"     ) \
    _( Ellipsis,        "..."   ) \
    _( Modulus,         "MOD"   )

//
// KEYWORD defines keyword tokens
//     Name             String
#define TKN_KEYWORD(_) \
    _( Dim,             "DIM"       ) \
    _( Var,             "VAR"       ) \
    _( As,              "AS"        ) \
    _( Declare,         "DECLARE"   ) \
    _( Function,        "FUNCTION"  ) \
    _( Sub,             "SUB"       ) \
    _( Ptr,             "PTR"       ) \
    _( End,             "END"       ) \
    _( Return,          "RETURN"    ) \
    _( True,            "TRUE"      ) \
    _( False,           "FALSE"     ) \
    _( Null,            "NULL"      ) \
    _( Any,             "ANY"       ) \
    _( If,              "IF"        ) \
    _( Then,            "THEN"      ) \
    _( Else,            "ELSE"      ) \
    _( For,             "FOR"       ) \
    _( To,              "TO"        ) \
    _( Step,            "STEP"      ) \
    _( Next,            "NEXT"      ) \
    _( Continue,        "CONTINUE"  ) \
    _( Exit,            "EXIT"      )

//
// Types define the type keywords
//
#define TKN_TYPES(_) \
    KEYWORD_TYPES(_)

//
// All tokens
//
#define ALL_TOKENS(_)   \
    TKN_BASIC(_)        \
    TKN_OPERATOR(_)     \
    TKN_KEYWORD(_)      \
    TKN_TYPES(_)
