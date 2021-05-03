''------------------------------------------------------------------------------
'' test-005-var-double.bas
'' - declare floating point value
'' - pass floating point
''
'' CHECK: 3.141590
''------------------------------------------------------------------------------
[alias="printf"] _
declare function printf(str as zstring, ...) as integer

var pi = 3.14159
printf "%lf", pi
