''------------------------------------------------------------------------------
'' test-008-logical-not.bas
'' - unary NOT to bool literal
'' - unary NOT to bool variable
''
'' CHECK: 0, 1, 0, 1
''------------------------------------------------------------------------------
import cstd

var f = not true
var t = not f
var ff = not not false
var tt = not not t

var int_f = if f then 1 else 0
var int_t = if t then 1 else 0
var int_ff = if ff then 1 else 0
var int_tt = if tt then 1 else 0
printf "%d, %d, %d, %d", int_f, int_t, int_ff, int_tt