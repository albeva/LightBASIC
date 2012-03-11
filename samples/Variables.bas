' Declare printf
[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

' global variables
Dim i As Byte
Dim s As Byte Ptr
Dim f As Single

' entry point
Function main() As Integer
    i = -10
    s = "Hello"
    f = 12.3
    printf("s = %s, i = %i, s = %.2f\n", s, i, f)
    Return 0
End Function