' Declare func from C std lib: int printf(const char* format, ...);
[Alias = "printf"] _
declare function Print(fmt as zstring, ...) as integer

' deduce typeExpr of message from expression: zstring
var message = "World"

' no need for braces when function does not return
Print "Hello %s!\n", message
