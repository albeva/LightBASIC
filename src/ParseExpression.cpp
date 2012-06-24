//
//  ParseExpression.cpp
//  LightBASIC
//
//  Created by Albert on 24/06/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//

#include "ParserShared.h"
using namespace lbc;


/**
 * Expression = IntegerLiteral
 *            | FloatingPointLiteral
 *            | StringLiteral
 *            | CallExpr
 *            | id
 *            | AddressOf
 *            | Dereference
 *            | True
 *            | False
 *            | Null
 *            | "(" Expression ")"
 *            | Comparison
 *            | Mod
 *
 * for the moment use hackish way to add
 * a binary expression.
 * later will use an expression parser
 * that is not easily expressed in ebnf anyways
 */
std::unique_ptr<AstExpression> Parser::expression()
{
    return nullptr;
}
