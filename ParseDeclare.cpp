//
//  ParseDECLARE.cpp
//  LightBASIC
//
//  Created by Albert on 04/11/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//

#include "ParserShared.h"
using namespace lbc;


/**
 * DECLARE = "DECLARE" FuncSignature
 */
std::unique_ptr<AstFunctionDecl> Parser::kwDeclare()
{
    EXPECT(TokenType::Declare);
    auto decl = make_unique<AstFunctionDecl>(funcSignature());
    if (!decl->signature) {
        m_hasError = true;
        return nullptr;
    }
    return decl;
}


/**
 * FuncSignature = "FUNCTION" id ["(" [ FuncParamList ] ")"] "AS" TypeExpr
 *               | "SUB" id ["(" [ FuncParamList ] ")"]
 */
std::unique_ptr<AstFuncSignature> Parser::funcSignature()
{
    // sub or function
    bool isSub = accept(TokenType::Sub);
    if (!isSub) {
        EXPECT(TokenType::Function);
    }

    // id
    auto id = identifier();
    if (!id) return nullptr;
    bool vararg = false;

    // args
    std::unique_ptr<AstFuncParamList> params = nullptr;

    // make ( and ) optional
    if (accept(TokenType::ParenOpen)) {
        // [ FuncArgumentList ]
        if (!accept(TokenType::ParenClose)) {

            params = funcParamList();
            if (!params) return nullptr;
            
            // ellipsis
            if (accept(TokenType::Ellipsis)) {
                vararg = true;
            }

            // ")"
            if (!expect(TokenType::ParenClose)) return nullptr;
        }
    }

    // return type
    std::unique_ptr<AstTypeExpr> type = nullptr;

    // if is a function then expect a type declaration
    if (!isSub) {
        // AS
        EXPECT(TokenType::As);

        // type
        type = typeExpr();
        if (!type) {
            // TODO raise error: expect type expression
            return nullptr;
        }
    }

    // done
    return make_unique<AstFuncSignature>(std::move(id),
                                         std::move(params),
                                         std::move(type),
                                         vararg);
}


/**
 * FuncParamList = FuncParam { "," FuncParam }
 */
std::unique_ptr<AstFuncParamList> Parser::funcParamList()
{
    auto ast = make_unique<AstFuncParamList>();
    do {
        if (match(TokenType::Ellipsis)) break;
        auto param = funcParam();
        if (!param) {
            // TODO raise error: expected parameter
            return nullptr;
        }
        ast->params.push_back(std::move(param));
    } while (accept(TokenType::Comma));

    return ast;
}


/**
 *  FuncParam = id "AS" TypeExpr
 */
std::unique_ptr<AstFuncParam> Parser::funcParam()
{
    // id
    auto id = identifier();
    if (!id) return nullptr;
    // AS
    if (!expect(TokenType::As)) return nullptr;
    // type
    auto type = typeExpr();
    if (!type) return nullptr;

    // done
    return make_unique<AstFuncParam>(std::move(id),
                                     std::move(type));
}

