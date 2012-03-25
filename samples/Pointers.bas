' Declare printf
[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

' entry point
Function main() As Integer
    
    Dim ip As Integer Ptr
    Dim sp As Single Ptr
    Dim ap As Any Ptr
    ip = null
    ap = ip
    
    Return 0
End Function
