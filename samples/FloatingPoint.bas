' Declare printf
[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

' entry point
Function main() As Integer
    Dim s As Single
    Dim d As Double
    d = 42
    s = d
    printf("s = %.2f, d = %.2f\n", s, d)
    Return 0
End Function