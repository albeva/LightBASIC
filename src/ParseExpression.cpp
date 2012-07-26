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
    //    AstExpression * expr = nullptr;
    //
    //    // create scope
    //    {
    //        SCOPED_GUARD(m_expectAssign);
    //        switch (m_token->type()) {
    //            // IntegerLiteral
    //            // FloatingPointLiteral
    //            // StringLiteral
    //            case TokenType::IntegerLiteral:
    //            case TokenType::StringLiteral:
    //            case TokenType::FloatingPointLiteral:
    //                expr = new AstLiteralExpr(m_token);
    //                if (!expr) return nullptr;
    //                move();
    //                break;
    //            // true, false
    //            case TokenType::True:
    //            case TokenType::False:
    //                expr = new AstLiteralExpr(m_token);
    //                if (!expr) return nullptr;
    //                move();
    //                break;
    //            // null
    //            case TokenType::Null:
    //                expr = new AstLiteralExpr(m_token);
    //                if (!expr) return nullptr;
    //                move();
    //                break;
    //            // CallExpr
    //            // id
    //            case TokenType::Identifier:
    //                if (m_next->type() == TokenType::ParenOpen) {
    //                    m_expectAssign = false;
    //                    expr = callExpr();
    //                    if (!expr) return nullptr;
    //                    break;
    //                } else {
    //                    expr = identifier();
    //                    if (!expr) return nullptr;
    //                    break;
    //                }
    //            // AddressOf
    //            case TokenType::AddressOf:
    //                move();
    //                expr = new AstAddressOfExpr(identifier());
    //                if (!expr) return nullptr;
    //                break;
    //            // Dereference
    //            case TokenType::Dereference:
    //                move();
    //                expr = new AstDereferenceExpr(expression());
    //                if (!expr) return nullptr;
    //                break;
    //            // ( Expression )
    //            case TokenType::ParenOpen:
    //                move();
    //                expr = expression();
    //                if (!expr) return nullptr;
    //                if (!expect(TokenType::ParenClose)) return nullptr;
    //                break;
    //            default:
    //                // TODO: raise error
    //                return nullptr;
    //        }
    //    }
    //
    //    // =
    //    if (!m_expectAssign) {
    //        switch (m_token->type()) {
    //            case TokenType::Assign:
    //                m_token->type(TokenType::Equal);
    //                // fall through!
    //            case TokenType::Modulus:
    //            case TokenType::NotEqual:
    //            case TokenType::GreaterThan:
    //            case TokenType::GreaterThanEqual:
    //            case TokenType::LessThan:
    //            case TokenType::LessThanEqual:
    //            {
    //                auto op = m_token;
    //                move();
    //                auto rhs = expression();
    //                if (!rhs) return nullptr;
    //                expr = new AstBinaryExpr(op, expr, rhs);
    //                break;
    //            }
    //            default:
    //                break;
    //        }
    //    }
    //    
    //    return expr;
    return nullptr;
}
