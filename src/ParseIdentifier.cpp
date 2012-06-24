//
//  ParseIdentifier.cpp
//  LightBASIC
//
//  Created by Albert on 24/06/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//

#include "ParserShared.h"
using namespace lbc;


/**
 * Identifier = id
 */
std::unique_ptr<AstIdentExpr> Parser::identifier()
{
    Token * tmp = m_token;
    EXPECT(TokenType::Identifier);
    return make_unique<AstIdentExpr>(tmp);
}