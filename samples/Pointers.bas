' Declare printf
[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Function getPtr (ip As Integer Ptr) As Integer Ptr
    Return ip
End Function

' entry point
Function main() As Integer
    Dim i As Integer
    *getPtr(&i) = 20
    printf("i = %i\n", i)
    Return 0
End Function
