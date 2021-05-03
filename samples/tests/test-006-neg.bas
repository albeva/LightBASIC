''------------------------------------------------------------------------------
'' test-006-neg.bas
'' - declare negative value
'' - negate values
''
'' CHECK: -3.141590, -5, 5
''------------------------------------------------------------------------------
[alias="printf"] _
declare function printf(str as zstring, ...) as integer

var a = 3.14159
var b = -a

var c = -5
var d = -c

printf "%lf, %d, %d", b, c, d
