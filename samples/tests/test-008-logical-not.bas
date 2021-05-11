'' test-008-logical-not.bas
'' - unary NOT to bool literal
'' - unary NOT to bool variable
''
'' CHECK: 0, 1, 0, 1
[alias="printf"] _
declare function printf(str as zstring, ...) as integer

var f = not true
var t = not f
var ff = not not false
var tt = not not t

var int_f as integer = f
var int_t as integer = t
var int_ff as integer = ff
var int_tt as integer = tt
printf "%d, %d, %d, %d", int_f, int_t, int_ff, int_tt