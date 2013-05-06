//
//  ParserShared.h
//  LightBASIC
//
//  Created by Albert on 24/06/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

#include "Parser.h"
#include "Context.h"
#include "Lexer.h"
#include "Ast.h"
#include "Token.h"

// Expect a token. If doesn't match, the expect will raise an error,
// exit from the current function and return nullptr
#define EXPECT(_tok) if (!this->expect(_tok)) return nullptr;
