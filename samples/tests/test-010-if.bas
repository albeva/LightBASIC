''------------------------------------------------------------------------------
'' test-010-if.bas
'' - if statement
''
'' CHECK: true, true, true, true, true
''------------------------------------------------------------------------------
import cstd

var b = true

if b then printf "true"

if not b then printf "unreachable" else
    printf ", true"
end if

if b then if b then
    printf ", true"
end if

if not b then
    printf "unreachable"
else if b then printf ", true"

if b and b then
    printf ", true"
else if b or b then
    printf "unreachable"
end if
