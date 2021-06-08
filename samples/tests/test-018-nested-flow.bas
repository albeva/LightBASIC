''------------------------------------------------------------------------------
'' test-018-nested-flow.bas
'' - nested continue statement
'' - nested exit statement
''
'' CHECK: 1, 2, 4
''------------------------------------------------------------------------------
import cstd

for i = 1 to 3
    if i <> 1 then printf ", "
    printf "%d", i
    for x = 4 to 7
        if i = 1 then continue for for
        if x = 5 then exit for for
        printf ", %d", x
    next
next
