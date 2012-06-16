''------------------------------------------------------------------------------
'' test-02
'' - crt printf function
'' - string literal and Byte Ptr type
'' - passing var arg
''
'' CHECK: Hello World!
''------------------------------------------------------------------------------
[Alias = "printf", Extern = "C"] _
Declare Function printf(msg As Byte Ptr, ...) As Integer

Function main() As Integer
    Dim s As Byte Ptr
    s = "Hello World!"
    printf("%s\n", s)
    return 0
End Function
