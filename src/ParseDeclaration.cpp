//
//  ParseProgram.cpp
//  LightBASIC
//
//  Created by Albert on 24/06/2012.
//  Copyright (c) 2012 Albert. All rights reserved.
//

#include "ParserShared.h"
using namespace lbc;


/**
 * Declaration  = [ Attributes ]
 *              ( DIM
 *              | VAR
 *              | FunctionDecl
 *              | FunctionImpl
 *              )
 */
std::unique_ptr<AstDeclaration> Parser::declaration()
{
    // [ Attributes ]
    auto attribs = match(TokenType::BracketOpen) ? attributes() : nullptr;
    if (hasError()) return nullptr;
        
    std::unique_ptr<AstDeclaration> decl;
    
    // DIM | VAR | FunctionDecl | FunctionImpl
    switch (m_token->type()) {
        case TokenType::Dim:
            decl = kwDim();
            break;
        case TokenType::Var:
            decl = kwVar();
            break;
        case TokenType::Declare:
            decl = kwDeclare();
            break;
        default:
            // TODO raise error. Expected declaration, found XXX
            m_hasError = true;
            return nullptr;
    }
    if (!decl) return nullptr;
    
    // set the attribs
    if (attribs) decl->attribs = std::move(attribs);
    
    // success
    return decl;
}
