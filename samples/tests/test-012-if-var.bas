''------------------------------------------------------------------------------
'' test-012-if-var.bas
'' - if statement with var
'' - single liners with follow up else
''
'' CHECK: 0 1 2 i = 5
''------------------------------------------------------------------------------
import cstd

test(0)
test(1)
test(2)
test(5)
printf "\n"

sub test(n as integer)
    if var i = get(n), i = 0 then printf "0 "
    else if            i = 1 then printf "1 "
    else if            i = 2 then printf "2 "
    else                          printf "i = %d", i
end sub

function get(n as integer) as integer
    return n
end function
