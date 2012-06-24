//
//  ParseTypeExpr.cpp
//  LightBASIC
//
//  Created by Albert on 24/06/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//

#include "ParserShared.h"
using namespace lbc;


/**
 * TypeExpr = ( TypeID | "ANY" "PTR") { "PTR" }
 */
std::unique_ptr<AstTypeExpr> Parser::typeExpr()
{
    Token * tmp = m_token;
    #define EXPECT_TYPE(ID, ...) accept(TokenType::ID) ||
    if (KEYWORD_TYPES(EXPECT_TYPE) false) {
        int deref = 0;
        while (accept(TokenType::Ptr)) deref++;
        return make_unique<AstTypeExpr>(tmp, deref);
    } else if (accept(TokenType::Any)) {
        if (!expect(TokenType::Ptr)) {
            // TODO raise error: expect ANY PTR
            return nullptr;
        }
        int deref = 1;
        while (accept(TokenType::Ptr)) deref++;
        return make_unique<AstTypeExpr>(tmp, deref);
    }
    
    // TODO raise error: expected type, found XXX
    m_hasError = true;
    return nullptr;
}
