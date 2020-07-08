//
// Created by Albert on 08/07/2020.
//
#pragma once

// this table is also used to generate tokens!

//     ID        STR         Kind
#define PRIMITIVE_TYPES(_) \
    _( Bool,     "BOOL",     Bool          ) \
    _( ZStrring, "ZSTRING",  ZString       )

//     ID        STR         Bits Signed Kind
#define INTEGER_TYPES(_) \
    _( Byte,     "BYTE",     8,   true,  Integer       ) \
    _( UByte,    "UBYTE",    8,   false, Integer       ) \
    _( Short,    "SHORT",    16,  true,  Integer       ) \
    _( UShort,   "USHORT",   16,  false, Integer       ) \
    _( Integer,  "INTEGER",  32,  true,  Integer       ) \
    _( UInteger, "UINTEGER", 32,  false, Integer       ) \
    _( Long,     "LONG",     64,  true,  Integer       ) \
    _( ULong,    "ULONG",    64,  false, Integer       )

//     ID        STR         Bits Kind
#define FLOATINGPOINT_TYPES(_) \
    _( Single,   "SINGLE",   32,  FloatingPoint ) \
    _( Double,   "DOUBLE",   64,  FloatingPoint )

#define ALL_TYPES(_) \
    PRIMITIVE_TYPES(_)     \
    INTEGER_TYPES(_)       \
    FLOATINGPOINT_TYPES(_)
