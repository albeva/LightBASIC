''------------------------------------------------------------------------------
'' test-12
'' - variable initalizer within function
''
'' CHECK: i = 10, s = LightBASIC, ip = 0x{{[0-9a-f]+}}, b = 1{{$}}
''------------------------------------------------------------------------------
[Extern = "C", Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Sub main
    Dim i  As Integer     = 10
    Dim s  As Byte Ptr    = "LightBASIC"
    Dim ip As Integer Ptr = &i
    Dim b  As Bool        = &i = ip
    printf "i = %i, s = %s, ip = %p, b = %i\n", _
            i, s, ip, b
End Sub
