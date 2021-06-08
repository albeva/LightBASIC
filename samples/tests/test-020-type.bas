''------------------------------------------------------------------------------
'' test-020-type.bas
'' - TYPE User defined
''
'' CHECK:       Player1.name = Albert
'' CHECK-NEXT:  Player1.age = 35
'' CHECK-NEXT:  Player2.name = Mario
'' CHECK-NEXT:  Player2.age = 58
''------------------------------------------------------------------------------
import cstd

Type Player
    name As ZString
    age As UInteger
End Type

Var p1 As Player
p1.name = "Albert"
p1.age = 35

printf "Player1.name = %s\n", p1.name
printf "Player1.age = %d\n", p1.age

Var p2 As Player
p2.name = "Mario"
p2.age = 58

printf "Player2.name = %s\n", p2.name
printf "Player2.age = %d\n", p2.age
