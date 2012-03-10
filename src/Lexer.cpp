//
//  Lexer.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//

#include "Lexer.h"
#include "Token.h"
#include "Source.h"
#include "SourceLocation.h"
using namespace lbc;


// character types
// copied from clang source
enum {
    CHAR_HORZ_WS  = 0x01,  // ' ', '\t', '\f', '\v'.  Note, no '\0'
    CHAR_VERT_WS  = 0x02,  // '\r', '\n'
    CHAR_LETTER   = 0x04,  // a-z,A-Z
    CHAR_NUMBER   = 0x08,  // 0-9
    CHAR_UNDER    = 0x10,  // _
    CHAR_PERIOD   = 0x20   // .
};

// Statically initialize CharInfo table based on ASCII character set
// Reference: FreeBSD 7.2 /usr/share/misc/ascii
// copied from clang source
static const unsigned char CharInfo[256] =
{
    // 0 NUL         1 SOH         2 STX         3 ETX
    // 4 EOT         5 ENQ         6 ACK         7 BEL
    0           , 0           , 0           , 0           ,
    0           , 0           , 0           , 0           ,
    // 8 BS          9 HT         10 NL         11 VT
    //12 NP         13 CR         14 SO         15 SI
    0           , CHAR_HORZ_WS, CHAR_VERT_WS, CHAR_HORZ_WS,
    CHAR_HORZ_WS, CHAR_VERT_WS, 0           , 0           ,
    //16 DLE        17 DC1        18 DC2        19 DC3
    //20 DC4        21 NAK        22 SYN        23 ETB
    0           , 0           , 0           , 0           ,
    0           , 0           , 0           , 0           ,
    //24 CAN        25 EM         26 SUB        27 ESC
    //28 FS         29 GS         30 RS         31 US
    0           , 0           , 0           , 0           ,
    0           , 0           , 0           , 0           ,
    //32 SP         33  !         34  "         35  #
    //36  $         37  %         38  &         39  '
    CHAR_HORZ_WS, 0           , 0           , 0           ,
    0           , 0           , 0           , 0           ,
    //40  (         41  )         42  *         43  +
    //44  ,         45  -         46  .         47  /
    0           , 0           , 0           , 0           ,
    0           , 0           , CHAR_PERIOD , 0           ,
    //48  0         49  1         50  2         51  3
    //52  4         53  5         54  6         55  7
    CHAR_NUMBER , CHAR_NUMBER , CHAR_NUMBER , CHAR_NUMBER ,
    CHAR_NUMBER , CHAR_NUMBER , CHAR_NUMBER , CHAR_NUMBER ,
    //56  8         57  9         58  :         59  ;
    //60  <         61  =         62  >         63  ?
    CHAR_NUMBER , CHAR_NUMBER , 0           , 0           ,
    0           , 0           , 0           , 0           ,
    //64  @         65  A         66  B         67  C
    //68  D         69  E         70  F         71  G
    0           , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
    CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
    //72  H         73  I         74  J         75  K
    //76  L         77  M         78  N         79  O
    CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
    CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
    //80  P         81  Q         82  R         83  S
    //84  T         85  U         86  V         87  W
    CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
    CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
    //88  X         89  Y         90  Z         91  [
    //92  \         93  ]         94  ^         95  _
    CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , 0           ,
    0           , 0           , 0           , CHAR_UNDER  ,
    //96  `         97  a         98  b         99  c
    //100  d       101  e        102  f        103  g
    0           , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
    CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
    //104  h       105  i        106  j        107  k
    //108  l       109  m        110  n        111  o
    CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
    CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
    //112  p       113  q        114  r        115  s
    //116  t       117  u        118  v        119  w
    CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
    CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
    //120  x       121  y        122  z        123  {
    //124  |        125  }        126  ~        127 DEL
    CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , 0           ,
    0           , 0           , 0           , 0
};


/**
 * Is part of identifier
 */
static inline bool IsIdentifierBody(unsigned char ch)
{
    return (CharInfo[ch] & (CHAR_LETTER | CHAR_NUMBER | CHAR_UNDER)) ? true : false;
}


/**
 * is line or a file end?
 */
static inline bool IsLineOrFileEnd(unsigned char ch)
{
    return ch == '\n' || ch == '\0';
}


/**
 * Create new lexer object
 */
Lexer::Lexer(const shared_ptr<Source> & src)
:   m_src(src),
    m_input(m_src->begin()),
    m_line(1),
    m_col(0),
    m_tokenStart(0),
    m_hasStmt(false)
{
}


/**
 * Get the next token
 */
Token * Lexer::next()
{
    unsigned char ch, nextCh, info;
    while((ch = *m_input) != '\0') {
        m_input++;
        m_col++;
        info = CharInfo[ch];
        
        // skip spaces
        if (info & CHAR_HORZ_WS) continue;
        
        nextCh = *m_input;
        
        
        // line endings
        if (ch == '\n') {
            if (m_hasStmt) {
                m_hasStmt = false;
                m_tokenStart = m_col;
                Token * tkn = MakeToken(TokenType::EndOfLine);
                m_line++;
                m_col = 0;
                return tkn;
            }
            m_line++;
            m_col = 0;
            continue;
        }
        
        // single line comment
        if (ch == '\'') {
            ch = *m_input++;
            while(!IsLineOrFileEnd(ch)) ch = *m_input++;
            m_input--; // TODO get rid of backtracking ?
            continue;
        }
        
        // multline comments
        if (ch == '/' && nextCh == '\'') {
            multilineComment();
            continue;
        }
        
        // statement continuation _
        if (ch == '_' && (CharInfo[nextCh] & (CHAR_LETTER | CHAR_NUMBER | CHAR_UNDER)) == 0) {
            ch = *m_input++;
            while(!IsLineOrFileEnd(ch)) ch = *m_input++;
            // CR + LF
            if (ch == '\n') {
                m_line++;
                m_col = 0;
            } else if (ch == '\0') {
                m_input--;
            }
            continue;
        }
        
        // has a statement
        m_hasStmt = true;
        m_tokenStart = m_col;
        
        // identifier
        if (info & CHAR_LETTER) return identifier();
        
        // string literal
        if (ch == '"') return string();
        
        // number
        if ((info & CHAR_NUMBER) || (ch == '-' && CharInfo[nextCh] & CHAR_NUMBER)) return number();
        
        // 3 char operators
        #define IMPL_TOKENS(ID, STR, ...)                       \
            if (sizeof(STR) == 4 && STR[0] == ch)               \
                if (STR[1] == nextCh && STR[2] == m_input[1]) { \
                    m_input++; m_input++; m_col += 2;          \
                    return MakeToken(TokenType::ID, STR);       \
                }
        TKN_OPERATOR(IMPL_TOKENS)
        #undef IMPL_TOKENS
        
        // 2 char operators
        #define IMPL_TOKENS(ID, STR, ...)                       \
            if (sizeof(STR) == 3 && STR[0] == ch)               \
                if (STR[1] == nextCh) {                         \
                    m_input++; m_col++;                         \
                    return MakeToken(TokenType::ID, STR);       \
                }
        TKN_OPERATOR(IMPL_TOKENS)
        #undef IMPL_TOKENS
        
        // 1 char operators
        #define IMPL_TOKENS(ID, STR, ...)                       \
            if (sizeof(STR) == 2 && STR[0] == ch)               \
                return MakeToken(TokenType::ID, STR);
        TKN_OPERATOR(IMPL_TOKENS)
        #undef IMPL_TOKENS
        
        // should not get here ...
        // invalid input
        // TODO read until end of garpabe and then return
        std::string invalid = "<invalid>";
        invalid += ch;
        return MakeToken(TokenType::Invalid, invalid);
    }
    
    // end token mark
    m_tokenStart = m_col + 1;
    
    // has a statement?
    if (m_hasStmt) {
        m_hasStmt = false;
        return MakeToken(TokenType::EndOfLine);
    }
    
    return MakeToken(TokenType::EndOfFile);
}


/**
 * Lex multiline comment
 */
void Lexer::multilineComment()
{
    m_input++;
    m_col++;
    int level = 1;
    while (*m_input != '\0') {
        char ch = *m_input++;
        m_col++;
        // '/ close
        if (ch == '\'' && *m_input == '/') {
            m_input++;
            m_col++;
            level--;
            if (level == 0) return;
        }
        // /' nested open
        else if (ch == '/' && *m_input == '\'') {
            m_input++;
            m_col++;
            level++;
        }
        // CR or LF line end
        else if (ch == '\n') {
            m_col = 0;
            m_line++;
        }
    }
}


/**
 * lex identifier
 * can be a keyword!
 */
Token * Lexer::identifier()
{
    // strat point
    Source::const_iterator begin = m_input;
    begin--;
    // read whil identifier char
    while (IsIdentifierBody(*m_input++));
    m_input--;
    
    // the id
    std::string id;
    std::transform(begin, m_input, std::back_inserter(id), (int(*)(int))std::toupper);
    m_col += (unsigned short)((m_input - begin) - 1);
    
    // Get type. if is a keyword make only from TokenID
    // as it will be faster string lookup from the keywords name lookup where its constant array access
    TokenType type = Token::getTokenType(id, TokenType::Identifier);
    return MakeToken(type, id);
}


/**
 * lex a number
 * for now simple integer contsants only
 */
Token * Lexer::number()
{
    // strat point
    Source::const_iterator begin = m_input;
    begin--;
    
    // read whil identifier char
    while (CharInfo[(*m_input++)] & CHAR_NUMBER);
    m_input--;
    
    // the id
    std::string number(begin, m_input);
    unsigned short length = (unsigned short)((m_input - begin) - 1);
    m_col += length;
    
    // make token
    return MakeToken(TokenType::NumericLiteral, number);
}


/**
 * lex string literal
 * simple literals. only escape supported is "", \" and \\
 */
Token * Lexer::string()
{
    // skip 1st "
    Source::const_iterator begin = m_input;
    std::string buffer;
    
    // read first
    char ch = *m_input++;
    while (true) {
        
        // end of file or line
        if (IsLineOrFileEnd(ch)) {
            m_input--;
            break;
        }
        
        // "
        if (ch == '"') {
            if (*m_input == '"') {
                ch = *(++m_input);
                m_input++;
                buffer += '"';
                continue;
            }
            break;
        }
        
        // \ escape
        if (ch == '\\') {
            char nextCh = *m_input;
            // \"
            if (nextCh == '"' || nextCh == '\\') {
                ch = *(++m_input);
                m_input++;
                buffer += nextCh;
                continue;
            } else if (nextCh == 'n') {
                ch = *(++m_input);
                m_input++;
                buffer += '\n';
                continue;
            } else if (nextCh == 't') {
                ch = *(++m_input);
                m_input++;
                buffer += '\t';
                continue;
            } else {
                /// complain about invalid escape
            }
        }
        
        buffer += ch;
        ch = *m_input++;
    }
    
    // fix column
    unsigned short length = (unsigned short)(m_input - begin) + 1;
    m_col += length - 1;
    
    // return
    return MakeToken(TokenType::StringLiteral, buffer, length);
}


/**
 * Make token
 */
Token * Lexer::MakeToken(TokenType type, int len)
{
    return new Token(type, SourceLocation(m_line, m_col, len));
}


/**
 * Make token
 */
Token * Lexer::MakeToken(TokenType type, const std::string & lexeme, int len)
{
    return new Token(type, SourceLocation(m_line, m_col, len == -1 ? lexeme.length() : len), lexeme);
}
