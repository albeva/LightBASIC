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
    _( Byte,        "BYTE",         8,      Type::Integral)         \
    _( Short,       "SHORT",        16,     Type::Integral)         \
    _( Integer,     "INTEGER",      32,     Type::Integral)         \
    _( Single,      "SINGLE",       32,     Type::FloatingPoint)    \
    _( Double,      "DOUBLE",       64,     Type::FloatingPoint)    \
    _( Bool,        "BOOL",         8,      Type::Integral)

//
// All types
#define ALL_TYPES(_)        \
    PRIMITIVE_TYPES(_)
