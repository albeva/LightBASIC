//
//  ParseAttributes.cpp
//  LightBASIC
//
//  Created by Albert on 24/06/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//

#include "ParserShared.h"
using namespace lbc;


/**
 * Attributes = "[" Attribute { ","  Attribute } "]"
 */
std::unique_ptr<AstAttributeList> Parser::attributes()
{
    // "["
    EXPECT(TokenType::BracketOpen);
    
    // Attribute { ","  Attribute }
    auto list = make_unique<AstAttributeList>();
    do {
        list->attribs.push_back(attribute());
        if (hasError()) return nullptr;
    } while (accept(TokenType::Comma));
    
    // "]"
    EXPECT(TokenType::BracketClose);
        
    // success
    return list;
}


/**
 * Attribute = id [ AttribParamList ]
 */
std::unique_ptr<AstAttribute> Parser::attribute()
{
    // id
    auto attr = make_unique<AstAttribute>(identifier());
    if (hasError()) return nullptr;
    
    // [ AttribParamList ]
    if (match(TokenType::Assign) || match(TokenType::ParenOpen)) {
        attr->params = attribParamList();
        if (hasError()) return nullptr;
    }
    
    // success
    return attr;
}


/**
 * AttribParamList = "=" AttribParam
 *                 | "(" AttribParam { "," AttribParam } ")"
 *                 .
 */
std::unique_ptr<AstAttribParamList> Parser::attribParamList()
{
    auto list = make_unique<AstAttribParamList>();
    
    // "=" AttribParam
    if (accept(TokenType::Assign)) {
        list->params.push_back(std::move(attribParam()));
        if (hasError()) return nullptr;
        return list;
    }
    
    // "("
    EXPECT(TokenType::ParenOpen);
    
    // AttribParam { "," AttribParam }
    do {
        list->params.push_back(attribParam());
        if (hasError()) return nullptr;
    } while (accept(TokenType::Comma));
    
    // ")"
    EXPECT(TokenType::ParenClose);
    
    // Success
    return list;
}


/**
 * AttribParam = IntegerLiteral
 *             | StringLiteral
 *             .
 */
std::unique_ptr<AstLiteralExpr> Parser::attribParam()
{
    auto tmp = m_token;
    if (accept(TokenType::StringLiteral) || accept(TokenType::IntegerLiteral)) {
        return make_unique<AstLiteralExpr>(tmp);
    }
    // TODO raise error: expected integer or a string
    m_hasError = true;
    return nullptr;
}

