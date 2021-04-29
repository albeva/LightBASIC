//
// Created by Albert Varaksin on 08/07/2020.
//
#pragma once

// clang-format off

// ID and STR are used in Token.deh.h for keyword tokens

//     ID        STR         Kind
#define PRIMITIVE_TYPES(_) \
    _( Bool,     "BOOL",     Boolean )  \
    _( ZString,  "ZSTRING",  ZString )

//     ID        STR         Kind,    Bits  Signed
#define INTEGER_TYPES(_) \
    _( Byte,     "BYTE",     Integral, 8,   true  ) \
    _( UByte,    "UBYTE",    Integral, 8,   false ) \
    _( Short,    "SHORT",    Integral, 16,  true  ) \
    _( UShort,   "USHORT",   Integral, 16,  false ) \
    _( Integer,  "INTEGER",  Integral, 32,  true  ) \
    _( UInteger, "UINTEGER", Integral, 32,  false ) \
    _( Long,     "LONG",     Integral, 64,  true  ) \
    _( ULong,    "ULONG",    Integral, 64,  false )

//     ID        STR         Kind           Bits
#define FLOATINGPOINT_TYPES(_) \
    _( Single,   "SINGLE",   FloatingPoint, 32 ) \
    _( Double,   "DOUBLE",   FloatingPoint, 64 )

#define ALL_TYPES(_)   \
    PRIMITIVE_TYPES(_) \
    INTEGER_TYPES(_)   \
    FLOATINGPOINT_TYPES(_)
