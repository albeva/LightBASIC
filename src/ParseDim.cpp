//
//  ParseDIM.cpp
//  LightBASIC
//
//  Created by Albert on 24/06/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//

#include "ParserShared.h"
using namespace lbc;


/**
 * DIM = "DIM" id "AS" TypeExpr [ "=" Expression ]
 */
std::unique_ptr<AstVarDecl> Parser::kwDim()
{
    // "DIM"
    EXPECT(TokenType::Dim);
    
    auto decl = make_unique<AstVarDecl>();
    
    // id
    decl->id = identifier();
    if (hasError()) return nullptr;
    
    // "AS"
    EXPECT(TokenType::As);
    
    // TypeExpr
    decl->typeExpr = typeExpr();
    if (hasError()) return nullptr;
    
    // [ "=" Expression ]
    if (accept(TokenType::Assign)) {
        decl->expr = expression();
        if (hasError()) return nullptr;
    }
    
    return decl;
}
