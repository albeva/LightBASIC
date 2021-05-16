''------------------------------------------------------------------------------
'' test-010-if.bas
'' - if statement
''
'' CHECK: true, true, true, true
''------------------------------------------------------------------------------
[alias="printf"] _
declare function printf(fmt as zstring, ...) as integer

var b = true

if b then
    printf "true"
end if

if not b then
    printf "wrong"
else
    printf ", true"
end if

if b then
    if b then
        printf ", true"
    end if
end if

if not b then
    printf "wrong"
else if b then
    printf ", true"
end if
