''------------------------------------------------------------------------------
'' test-013-for.bas
'' - for loop
''
'' CHECK: 1, 2, 3, 4, 5
'' CHECK: 0, 2, 4, 6, 8, 10
'' CHECK: 5, 4, 3, 2, 1
'' CHECK: 6, 3, 0
'' CHECK: 1, 4, 7
''------------------------------------------------------------------------------
[alias="printf"] _
declare function printf(fmt as zstring, ...) as integer

' with scoped var
for var first = true, i = 1 to 5
    if first then first = false else printf ", "
    printf "%d", i
next
printf "\n"

' singe liner
for i = 0 to 10 step 2 do printf "%i, ", i
printf "\n"

' backwards
for i = 5 to 1 do printf "%i, ", i
printf "\n"

' backwards with step
for i = 6 to 0 step -3 do printf "%i, ", i
printf "\n"

' with variables
var b = 1
var e = 7
var s = 3
for i = b to e step s do printf "%i, ", i
printf "\n"
