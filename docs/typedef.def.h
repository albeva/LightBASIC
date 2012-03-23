//
// Info needed:
// - parsable: include in lexer and parser ad a keyword?
// - sizes for 32bit build
// - sizes for 64bit build
// - kind: FloatingPoint, Integral, Pointer, Function
// - vararg behaviour
// - return type
// - function param type
// ---------------------
// * storage size. ( sizeof() )
// * size when is function argument
// * size when is function parameter
// * size when is passed as vararg
#define DEFINE_TYPE(_)                                                         \
    /* Id       String      32{S, F, V}		  64{S, F, V},    Kind
    ------------------------------------------------------------------------*/ \
    _( Byte,    "BYTE",     {8,   8,   32},   {8,   8,   32},	Signed       ) \
    _( UByte,   "UBYTE",    {8,   8,   32},   {8,   8,   32},	Unsigned     ) \
    _( Short,   "SHORT",    {16,  16,  32},   {16,  16,  32},   Signed       ) \
    _( UShort,  "USHORT",   {16,  16,  32},   {16,  16,  32},   Unsigned     ) \
    _( Integer, "INTEGER",  {32,  32,  32},   {32,  32,  32},   Signed       ) \
    _( Uinteger,"UINTEGER", {32,  32,  32},   {32,  32,  32},   Unsigned     ) \
    _( Long,    "LONG",     {32,  32,  32},   {64,  64,  64},   Signed       ) \
    _( ULong,   "ULONG",    {32,  32,  32},   {64,  64,  64},   Unsigned     ) \
    _( LongInt, "LONGINT",  {64,  64,  64},   {64,  64,  64},   Signed       ) \
    _( ULongInt,"ULONGINT", {64,  64,  64},   {64,  64,  64},   Unsigned     ) \
    _( Single,  "SINGLE",   {32,  32,  64},   {32,  32,  64},   FloatingPoint) \
    _( Double,  "DOUBLE",   {64,  64,  32},   {64,  64,  64},   FloatingPoint) \
    _( Decimal, "DECIMAL",  {128, 80,  128},  {128, 80, 128},   FloatingPoint) \
    _( Bool,    "BOOL",     {8,   1,   32},   {8,   1,   32},   Boolean      )

#define CONSTANT_VALUE(_) \
    _( Null,    "NULL",     AnyPtr,     0) \
    _( True,    "TRUE",     Bool,       1) \
    _( False,   "FALSE",    Bool,       0)


/**
 * Return common base type or a nullptr if none found
 */
Type * common(Type * t1, Type * t2);	