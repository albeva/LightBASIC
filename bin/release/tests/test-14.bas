''------------------------------------------------------------------------------
'' test-12
'' - VAR statement. Deduct type from expression
''
'' CHECK: i = 10, s = LightBASIC, p = 0x{{[0-9a-f]+}}, b = 1{{$}}
''------------------------------------------------------------------------------
[Extern = "C", Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Sub main
    Var i = 10
    Var s = "LightBASIC"
    Var p = &i
    Var b = &i = p
    printf "i = %i, s = %s, p = %p, b = %i\n", _
            i, s, p, b
End Sub
