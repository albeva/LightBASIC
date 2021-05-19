''------------------------------------------------------------------------------
'' test-014-cast.bas
'' - type cast
''
'' CHECK: 10.000000, 10
''------------------------------------------------------------------------------
[alias="printf"] _
declare function printf(fmt as zstring, ...) as integer

var i = 10
var f = i as double
var b = f as byte

printf "%lf, %hhi", f, b
