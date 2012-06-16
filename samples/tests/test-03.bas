''------------------------------------------------------------------------------
'' test-03
'' - test pointer dereferencing
'' - test address of
'' - test assignment
''
'' CHECK: 10
''------------------------------------------------------------------------------
[Alias = "printf", Extern = "C"] _
Declare Function printf(msg As Byte Ptr, ...) As Integer

Function main() As Integer
    Dim i As Integer
    Dim ip As Integer Ptr
    ip = &i
    *ip = 10
    printf("%d\n", i)
    return 0
End Function
