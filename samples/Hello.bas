' Declare func from C std lib: int puts(const char*);
[Alias = "puts"] _
declare function print(s as zstring) as integer

' deduce typeExpr of message from expression: zstring
var message = "Hello World!"

' no need for braces when function does not return
print message
