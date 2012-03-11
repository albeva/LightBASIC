' Declare printf
[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

' global variables
Dim i As Byte
Dim s As Byte Ptr

' entry point
Function main() As Integer
    i = -10
    s = "Hello"
    printf("s = %s, i = %i\n", s, i)
    Return 0
End Function