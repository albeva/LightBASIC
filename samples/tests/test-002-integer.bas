''------------------------------------------------------------------------------
'' test-002-integer.bas
'' - variadic function
'' - pass zstring and an integer as argument
''
'' CHECK: 42
''------------------------------------------------------------------------------
[alias="printf"] _
declare function printf(str as zstring, ...) as integer

printf "%d", 42
