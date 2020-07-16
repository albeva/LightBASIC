' Declare func from C std lib: int puts(const char*);
[Alias = "puts"] _
declare function print(str as zstring) as integer

' deduce type of message from value
var message = "Hello World!"

' no need for braces when function is not inside expression
print message
