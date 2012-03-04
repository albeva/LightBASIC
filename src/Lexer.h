//
//  Lexer.h
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

#include "Source.h"
#include "Token.h"

namespace lbc {
	
	class Token;
	
	/**
	 * This class deals with lexing the input into tokens
	 */
	struct Lexer : NonCopyable
	{
		// create new lexer instance
		Lexer(const shared_ptr<Source> & src);
		
		// get next token
		Token * next();
		
		private:
		
        /// make token
        Token * MakeToken(TokenType type, int len = 0);
        Token * MakeToken(TokenType type, const std::string & lexeme, int len = -1);
		
		void multilineComment();
		Token * identifier();
		Token * number();
		Token * string();
		
		// the source
		shared_ptr<Source> m_src;
        Source::const_iterator m_input;
		
        // state info
        unsigned int m_line;
        unsigned short m_col;
        unsigned short m_tokenStart;
        bool m_hasStmt;
	};

}