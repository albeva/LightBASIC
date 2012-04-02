//
//  Type.def.h
//  LightBASIC
//
//  Created by Albert Varaksin on 01/03/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

//
// Define integral types
//     Id           String          Size    Flags
#define PRIMITIVE_TYPES(_)  \
    _( Byte,        "BYTE",         8,      Type::Integral)                     \
    _( UByte,       "UBYTE",        8,      Type::Integral | Type::Unsigned)    \
    _( Short,       "SHORT",        16,     Type::Integral)                     \
    _( UShort,      "USHORT",       16,     Type::Integral | Type::Unsigned)    \
    _( Integer,     "INTEGER",      32,     Type::Integral)                     \
    _( UInteger,    "UINTEGER",     32,     Type::Integral | Type::Unsigned)    \
    _( LongInt,     "LONGINT",      64,     Type::Integral)                     \
    _( ULongInt,    "ULONGINT",     64,     Type::Integral | Type::Unsigned)    \
    _( Single,      "SINGLE",       32,     Type::FloatingPoint)                \
    _( Double,      "DOUBLE",       64,     Type::FloatingPoint)                \
    _( Bool,        "BOOL",         8,      Type::Boolean | Type::Integral | Type::Unsigned)

//
// All types
#define ALL_TYPES(_)        \
    PRIMITIVE_TYPES(_)

//
// User facing types (keywords)
#define KEYWORD_TYPES(_) \
    _( Byte,        "BYTE"      ) \
    _( UByte,       "UBYTE"     ) \
    _( Short,       "SHORT"     ) \
    _( UShort,      "USHORT"    ) \
    _( Integer,     "INTEGER"   ) \
    _( UInteger,    "UINTEGER"  ) \
    _( LongInt,     "LONGINT"   ) \
    _( ULongInt,    "ULONGINT"  ) \
    _( Single,      "SINGLE"    ) \
    _( Double,      "DOUBLE"    ) \
    _( Bool,        "BOOL"      )

