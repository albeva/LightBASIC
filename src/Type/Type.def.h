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

//     ID        STR         Kind,    Bits  Signed  C++
#define INTEGRAL_TYPES(_) \
    _( Byte,     "BYTE",     Integral, 8,   true,   int8_t   ) \
    _( UByte,    "UBYTE",    Integral, 8,   false,  uint8_t  ) \
    _( Short,    "SHORT",    Integral, 16,  true,   int16_t  ) \
    _( UShort,   "USHORT",   Integral, 16,  false,  uint16_t ) \
    _( Integer,  "INTEGER",  Integral, 32,  true,   int32_t  ) \
    _( UInteger, "UINTEGER", Integral, 32,  false,  uint32_t ) \
    _( Long,     "LONG",     Integral, 64,  true,   int64_t  ) \
    _( ULong,    "ULONG",    Integral, 64,  false,  uint64_t )

//     ID        STR         Kind           Bits    c++
#define FLOATINGPOINT_TYPES(_) \
    _( Single,   "SINGLE",   FloatingPoint, 32,     float  ) \
    _( Double,   "DOUBLE",   FloatingPoint, 64,     double )

#define ALL_TYPES(_)   \
    PRIMITIVE_TYPES(_) \
    INTEGRAL_TYPES(_)   \
    FLOATINGPOINT_TYPES(_)
