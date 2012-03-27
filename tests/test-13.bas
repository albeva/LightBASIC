''------------------------------------------------------------------------------
'' test-12
'' - variable initalizer within function
''
'' CHECK: i = 10, s = LightBASIC, p = 0x{{[0-9a-f]+}}, b = 1{{$}}
''------------------------------------------------------------------------------
[Extern = "C", Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Sub main
    Dim i As Integer     = 10
    Dim s As Byte Ptr    = "LightBASIC"
    Dim p As Integer Ptr = &i
    Dim b As Bool        = &i = p
    printf "i = %i, s = %s, p = %p, b = %i\n", _
            i, s, p, b
End Sub
