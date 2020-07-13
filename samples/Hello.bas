' Declare func from C std lib: int printf(const char* format, ...);
[Alias = "printf"] _
declare function Printf(fmt as zstring, ...) as integer

' Declare func from C std lib: int printf(const char* format, ...);
[Alias = "puts"] _
declare function Puts(fmt as zstring) as integer

' deduce typeExpr of message from expression: zstring
var message = "World"
var copy as zstring = message

' no need for braces when function does not return
Printf "Hello %s!\n", message
Puts copy
