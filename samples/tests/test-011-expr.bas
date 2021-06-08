''------------------------------------------------------------------------------
'' test-011-expr.bas
'' - arithmetic expressions
'' - comparison expressions
'' - implicit conversions
''
'' CHECK: 7, 4, 1
'' CHECK: 7.000000, 4.000000, 1.000000
'' CHECK: true, true, true, false, true
''------------------------------------------------------------------------------
import cstd

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

out "",   iseven > ifour
out ", ", ifour >= ione
out ", ", ione < ifour
out ", ", c3 > dfour
out ", ", iseven = dseven
printf "\n"

sub out(sep as zstring, b as bool)
    printf "%s%s", sep, if b then "true" else "false"
end sub
