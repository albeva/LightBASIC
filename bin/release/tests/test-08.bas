''------------------------------------------------------------------------------
'' test-08
'' - global variables
'' - signed numbers
'' - floating point numbers
'' - passing var arg
''
'' CHECK: s = Hello, i = -10, s = 12.30{{$}}
''------------------------------------------------------------------------------
[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Dim i As Byte
Dim s As Byte Ptr
Dim f As Single

Function main() As Integer
    i = -10
    s = "Hello"
    f = 12.3
    printf("s = %s, i = %i, s = %.2f\n", s, i, f)
    Return 0
End Function
