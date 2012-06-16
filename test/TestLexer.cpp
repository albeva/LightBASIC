//
//  TestLexer.cpp
//  LightBASIC
//
//  Created by Albert on 16/06/2012.
//  Copyright (c) 2012 Albert. All rights reserved.
//

#include <boost/test/unit_test.hpp>
#include "Lexer.h"
#include <llvm/Support/MemoryBuffer.h>

BOOST_AUTO_TEST_SUITE(TestLexer)
using namespace lbc;
using namespace std;

/**
 * Helper to test expected token
 */
void ExpectToken(Lexer & lexer, TokenType type, const string & lexeme, int line = -1, int col = -1, int len = -1)
{
    auto * token = lexer.next();
    BOOST_REQUIRE(token != nullptr);
    BOOST_CHECK(token->type() == type);
    BOOST_CHECK(token->lexeme() == lexeme);
//    if (line != -1) BOOST_CHECK(token->GetLocation().GetLine() == line);
//    if (col != -1)  BOOST_CHECK(token->GetLocation().GetColumn() == col);
//    if (len != -1)  BOOST_CHECK(token->GetLocation().GetLength() == len);
    delete token;
}


/**
 * Perform basic lexer testing with various empty strings
 * all should return end of file token
 */
BOOST_AUTO_TEST_CASE( EmptySource )
{
    // random strings. they will all be lexed to empty
    const char * strings[] = {
        "",
        "   ",
        "\t\t",
        "\n   \n   ",
        "\r\n",
        "   \r   \n  \t  ",
        "'comment string",
        " /' stream \n '/ ",
        "/'somethign",
        "/'/' doubly nested '/'/",
        " \t _ this should be ignored \n_ ignored again",
        0
    };
    for (int i = 0; strings[i] != 0; i++) {
        Lexer lexer(strings[i]);
        ExpectToken(lexer, TokenType::EndOfFile, "");
    }
}


/**
 * Test multiline comments
 */
BOOST_AUTO_TEST_CASE( MultiLineComments )
{
    const char * strings[] = {
        "a/''/b",
        "a/' '/b",
        "a /''/ b",
        "a /' '/ b",
        "a /'\n'/ b",
        "a /'\r\n'/b",
        "a/'\r'/b",
        "a/'/''/'/b",
        "a/' / ' ' / '/b",
        "/' \n '/a/' /' '/\n '/b/'",
        "a _\n /' some multiline coment \n on a new line '/ _\n /' \n /' cont '/ \r\n '/ b",
        0
    };
    
    for (int i = 0; strings[i] != 0; i++) {
        Lexer lexer(strings[i]);
        // start
        ExpectToken(lexer, TokenType::Identifier, "A");
        // end
        ExpectToken(lexer, TokenType::Identifier, "B");
        // end of statement
        ExpectToken(lexer, TokenType::EndOfLine, "");
        // end of file
        ExpectToken(lexer, TokenType::EndOfFile, "");
    }
}


/**
 * Test token locations
 */
BOOST_AUTO_TEST_CASE( TokenLocation )
{
    const char * source =
        "one \"two\" three 42 = <= ...\n"
        "four \t IF a = b THEN \r\n"
        "five /'/' nested '/'/ six\n"
        "seven/' trash\n trash /' nested\n'/\nend?'/eight";
    
    lbc::Lexer lexer(source);
        // line 1
    ExpectToken(lexer, TokenType::Identifier,       "ONE",      1,  1,   3);
    ExpectToken(lexer, TokenType::StringLiteral,    "two",      1,  5,   5);
    ExpectToken(lexer, TokenType::Identifier,       "THREE",    1,  11,  5);
    ExpectToken(lexer, TokenType::IntegerLiteral,   "42",       1,  17,  2);
    ExpectToken(lexer, TokenType::Assign,           "=",        1,  20,  1);
    ExpectToken(lexer, TokenType::LessThanEqual,    "<=",       1,  22,  2);
    ExpectToken(lexer, TokenType::Ellipsis,         "...",      1,  25,  3);
    ExpectToken(lexer, TokenType::EndOfLine,        "",         1,  28,  0);
        // line 2
    ExpectToken(lexer, TokenType::Identifier,       "FOUR",     2,  1,   4);
    ExpectToken(lexer, TokenType::If,               "IF",       2,  8,   2);
    ExpectToken(lexer, TokenType::Identifier,       "A",        2,  11,  1);
    ExpectToken(lexer, TokenType::Assign,           "=",        2,  13,  1);
    ExpectToken(lexer, TokenType::Identifier,       "B",        2,  15,  1);
    ExpectToken(lexer, TokenType::Then,             "THEN",     2,  17,  4);
    ExpectToken(lexer, TokenType::EndOfLine,        "",         2,  22,  0);
        // line 3
    ExpectToken(lexer, TokenType::Identifier,       "FIVE",     3,  1,   4);
    ExpectToken(lexer, TokenType::Identifier,       "SIX",      3,  23,  3);
    ExpectToken(lexer, TokenType::EndOfLine,        "",         3,  26,  0);
        // line 4
    ExpectToken(lexer, TokenType::Identifier,       "SEVEN",    4,  1,   5);
        // line 7
    ExpectToken(lexer, TokenType::Identifier,       "EIGHT",    7,  7,   5);
    ExpectToken(lexer, TokenType::EndOfLine,        "",         7,  12,  0);
    ExpectToken(lexer, TokenType::EndOfFile,        "",         7,  12,  0);
}



BOOST_AUTO_TEST_SUITE_END()
