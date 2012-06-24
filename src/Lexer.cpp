//
//  Lexer.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//

#include "Lexer.h"
#include "Token.h"
#include <llvm/Support/MemoryBuffer.h>
#include <iostream>
using namespace lbc;


// character types
// copied from clang source
enum : char {
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
static const char CharInfo[256] =
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
static inline bool IsIdentifierBody(char ch)
{
    return (CharInfo[(int)ch] & (CHAR_LETTER | CHAR_NUMBER | CHAR_UNDER)) ? true : false;
}


/**
 * is line or a file end?
 */
static inline bool IsLineOrFileEnd(char ch)
{
    return ch == '\n' || ch == '\0' || ch == '\r';
}


/**
 * Create new lexer object
 */
Lexer::Lexer(const llvm::MemoryBuffer * buffer)
:   m_src(buffer),
    m_input(buffer->getBufferStart()),
    m_start(nullptr),
    m_line(1),
    m_col(0),
    m_hasStmt(false),
    m_ownsSrc(false)
{
    m_nextCh = nextChar();
}
    

/**
 * create new lexer from a string
 */
Lexer::Lexer(const std::string & buffer)
: Lexer(llvm::MemoryBuffer::getMemBuffer(buffer))
{
    m_ownsSrc = true;
}


/**
 * Clean up
 */
Lexer::~Lexer()
{
    if (m_ownsSrc && m_src) delete m_src;
}


/**
 * Gext next character. Do not advance the
 * input except for line endings
 */
char Lexer::nextChar()
{
    // read the character from the input
    char ch = m_input != m_src->getBufferEnd() ? *m_input : '\0';
    
    // LF : Multics, Unix and Unix-like systems (GNU/Linux, Mac OS X, FreeBSD,
    //      AIX, Xenix, etc.), BeOS, Amiga, RISC OS and others.
    // LF + CR : Acorn BBC and RISC OS spooled text output
    if (ch == '\n') {
        auto next = m_input + 1;
        if (next != m_src->getBufferEnd() && *next == '\r') m_input = next;
    }
    // CR : Commodore 8-bit machines, Acorn BBC, TRS-80, Apple II family,
    //      Mac OS up to version 9 and OS-9
    // CR + LF : Microsoft Windows, DEC TOPS-10, RT-11 and most other early
    //           non-Unix and non-IBM OSes,
    //           CP/M, MP/M, DOS (MS-DOS, PC-DOS, etc.), Atari TOS, OS/2,
    //           Symbian OS, Palm OS
    else if (ch == '\r') {
        auto next = m_input + 1;
        if (next != m_src->getBufferEnd() && *next == '\n') m_input = next;
        ch = '\n';
    }
    
    // done
    return ch;
}

/**
 * Get next character from the buffer,
 * advance the buffer by 1. Deal with different
 * line endings
 * CR is converted to LF internally for consistency
 */
bool Lexer::move()
{
    m_ch = m_nextCh;
    if (m_ch == '\0') return false;
    
    m_input++;
    m_nextCh = nextChar();
    
    return true;
}


/**
 * Get the next token
 */
Token * Lexer::next()
{
    char info;
    
    // loop while not end
    for (m_start = m_input; move(); m_start = m_input) {
        m_col++;
        info = CharInfo[(int)m_ch];
        
        // skip spaces
        if (info & CHAR_HORZ_WS) continue;
        
        // line endings
        if (m_ch == '\n') {
            if (m_hasStmt) {
                m_hasStmt = false;
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
        if (m_ch == '\'') {
            while (!IsLineOrFileEnd(m_nextCh) && move());
            continue;
        }
        
        // multline comments
        if (m_ch == '/' && m_nextCh == '\'') {
            multilineComment();
            continue;
        }
        
        // statement continuation _
        if (m_ch == '_' && !IsIdentifierBody(m_nextCh)) {
            move();
            while (!IsLineOrFileEnd(m_ch) && move());
            if (m_ch == '\n') {
                m_line++;
                m_col = 0;
            }
            continue;
        }
        
        // has a statement
        m_hasStmt = true;
        
        // identifier
        if (info & CHAR_LETTER) return identifier();
        
        // string literal
        if (m_ch == '"') return string();
        
        // number
        if ((info & CHAR_NUMBER) || ((m_ch == '-' || m_ch == '.') && CharInfo[(int)m_nextCh] & CHAR_NUMBER))
            return number();
        
        // 3 char operators
        #define IMPL_TOKENS(ID, STR)                            \
            if (sizeof(STR) == 4 && STR[0] == toupper(m_ch))    \
                if (STR[1] == toupper(m_nextCh)                 \
                    && STR[2] == toupper(m_input[1])) {         \
                    move(); move(); m_col += 2;                 \
                    return MakeToken(TokenType::ID, STR);       \
                }
        TKN_OPERATOR(IMPL_TOKENS)
        #undef IMPL_TOKENS
        
        // 2 char operators
        #define IMPL_TOKENS(ID, STR)                            \
            if (sizeof(STR) == 3 && STR[0] == toupper(m_ch))    \
                if (STR[1] == toupper(m_nextCh)) {              \
                    move(); m_col++;                            \
                    return MakeToken(TokenType::ID, STR);       \
                }
        TKN_OPERATOR(IMPL_TOKENS)
        #undef IMPL_TOKENS
        
        // 1 char operators
        #define IMPL_TOKENS(ID, STR)                            \
            if (sizeof(STR) == 2 && STR[0] == m_ch)             \
                return MakeToken(TokenType::ID, STR);
        TKN_OPERATOR(IMPL_TOKENS)
        #undef IMPL_TOKENS
        
        // should not get here ...
        // invalid input
        // TODO read until end of garpabe and then return
        std::string invalid = "<invalid>";
        invalid += m_ch;
        return MakeToken(TokenType::Invalid, invalid);
    }
    
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
    move();
    m_col++;
    int level = 1;
    while (move()) {
        m_col++;
        // '/ close
        if (m_ch == '\'' && m_nextCh == '/') {
            move();
            m_col++;
            level--;
            if (level == 0) return;
        }
        // /' nested open
        else if (m_ch == '/' && m_nextCh == '\'') {
            move();
            m_col++;
            level++;
        }
        // CR or LF line end
        else if (m_ch == '\n') {
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
    while (IsIdentifierBody(m_nextCh) && move());
    std::string id;
    std::transform(m_start, m_input, std::back_inserter(id), (int(*)(int))std::toupper);
    m_col += (unsigned short)((m_input - m_start) - 1);
    return MakeToken(Token::getTokenType(id, TokenType::Identifier), id);
}


/**
 * lex a number
 * for now simple integer contsants only
 */
Token * Lexer::number()
{
    
    // read whil identifier char
    bool fp = *m_start == '.', invalid = false;
    while (!IsLineOrFileEnd(m_nextCh)) {
        if (m_nextCh == '.') {
            if (fp) {
                invalid = true;
                break;
            }
            fp = true;
        } else if ((CharInfo[(int)m_nextCh] & CHAR_NUMBER) == 0) {
            break;
        }
        move();
    }
    
    // the id
    std::string number(m_start, m_input);
    unsigned short length = (unsigned short)((m_input - m_start) - 1);
    m_col += length;
    
    // invalid ?
    if (invalid) {
        return MakeToken(TokenType::Invalid, number);
    }
    
    // make token
    return MakeToken(fp ? TokenType::FloatingPointLiteral : TokenType::IntegerLiteral, number);
}


/**
 * lex std::string literal
 * simple literals. only escape supported is "", \" and \\
 */
Token * Lexer::string()
{
    // skip 1st "
    m_start = m_input;
    std::string buffer;
    
    // read first
    while (!IsLineOrFileEnd(m_nextCh) && move()) {
        
        // "
        if (m_ch == '"') {
            if (m_nextCh == '"') {
                move();
                buffer += '"';
                continue;
            }
            break;
        }
        
        // \ escape
        if (m_ch == '\\') {
            // \"
            if (m_nextCh == '"' || m_nextCh == '\\') {
                move();
                buffer += m_nextCh;
                continue;
            } else if (m_nextCh == 'n') {
                move();
                buffer += '\n';
                continue;
            } else if (m_nextCh == 't') {
                move();
                buffer += '\t';
                continue;
            } else {
                /// complain about invalid escape
            }
        }
        
        buffer += m_ch;
    }
    
    // fix column
    unsigned short length = (unsigned short)(m_input - m_start) + 1;
    m_col += length - 1;
    
    // return
    return MakeToken(TokenType::StringLiteral, buffer, length);
}


/**
 * Make token
 */
Token * Lexer::MakeToken(TokenType type, int)
{
    return new Token(type, llvm::SMLoc::getFromPointer(m_start));
}


/**
 * Make token
 */
Token * Lexer::MakeToken(TokenType type, const std::string & lexeme, int)
{
    return new Token(type, llvm::SMLoc::getFromPointer(m_start), lexeme);
}
