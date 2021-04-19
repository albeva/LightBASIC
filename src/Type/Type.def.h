//
// Created by Albert Varaksin on 08/07/2020.
//
#pragma once

// clang-format off

// ID and STR are used in Token.deh.h for keyword tokens

//     ID        STR         Kind
#define PRIMITIVE_TYPES(_) \
    _( Bool,     "BOOL",     Bool    )  \
    _( ZString,  "ZSTRING",  ZString )

//     ID        STR         Kind,    Bits Signed
#define INTEGER_TYPES(_) \
    _( Byte,     "BYTE",     Integer, 8,   true  ) \
    _( UByte,    "UBYTE",    Integer, 8,   false ) \
    _( Short,    "SHORT",    Integer, 16,  true  ) \
    _( UShort,   "USHORT",   Integer, 16,  false ) \
    _( Integer,  "INTEGER",  Integer, 32,  true  ) \
    _( UInteger, "UINTEGER", Integer, 32,  false ) \
    _( Long,     "LONG",     Integer, 64,  true  ) \
    _( ULong,    "ULONG",    Integer, 64,  false )

//     ID        STR         Kind           Bits
#define FLOATINGPOINT_TYPES(_) \
    _( Single,   "SINGLE",   FloatingPoint, 32 ) \
    _( Double,   "DOUBLE",   FloatingPoint, 64 )

#define ALL_TYPES(_)   \
    PRIMITIVE_TYPES(_) \
    INTEGER_TYPES(_)   \
    FLOATINGPOINT_TYPES(_)
