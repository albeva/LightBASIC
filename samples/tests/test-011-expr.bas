''------------------------------------------------------------------------------
'' test-011-expr.bas
'' - logical expressions
'' - arithmetic expressions
'' - comparison expressions
''
'' CHECK: 7, 4, 1
'' CHECK: 7.000000, 4.000000, 1.000000
'' CHECK: true, true, true, false, true
''------------------------------------------------------------------------------
[alias="printf"] _
declare function printf(fmt as zstring, ...) as integer

var iseven = 1 + 2 * 3
var ifour = iseven - 6 / 2
var ione = 10 mod 3
printf "%d, %d, %d\n", iseven, ifour, ione

var c3 as double = 3
var d10 as double = 10
var dseven = 1 + 2 * c3
var dfour = dseven - 6 / 2
var done = d10 mod c3
printf "%lf, %lf, %lf\n", dseven, dfour, done

out iseven > ifour, false
out ifour >= ione, true
out ione < ifour, true
out c3 > dfour, true
out iseven = dseven, true

sub out(b as bool, first as bool)
    if first then printf ", "
    printf "%s", if b then "true" else "false"
end sub
