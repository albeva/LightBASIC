''------------------------------------------------------------------------------
'' test-007-conversions.bas
'' - declare variables of different types
'' - assign different types
''
'' CHECK: 3.140000, 3
''------------------------------------------------------------------------------
[alias="printf"] _
declare function printf(str as zstring, ...) as integer

var d as double = 3.14
var s as single = d
var i as integer = d

printf "%lf, %d", d, i
