' from libc
[Alias = "puts", lib="crt"] _
Declare Function Print(msg As Byte Ptr) As Integer
' from module_2.bas
Declare Function GetFromModule2() As Byte Ptr

Function main() As Integer
    Print(GetFromModule2())
    Return 0
End Function
