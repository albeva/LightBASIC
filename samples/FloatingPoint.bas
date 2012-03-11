' Declare printf
[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

' entry point
Function main() As Integer
    Dim s As Single
    s = 42.12
    printf("s = %.2f\n", s)
    Return 0
End Function