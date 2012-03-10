' Declare puts from libc
[Extern = "C", Alias = "puts", Lib = "crt"] _
Declare Function print(str As Byte Ptr) As Integer

' global variables
Dim i As Byte
Dim b As Byte Ptr

' entry point
Function main() As Integer
    i = 10
    b = "Hello"
    print(b)
    Return 0
End Function