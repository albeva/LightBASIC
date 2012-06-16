''------------------------------------------------------------------------------
'' test-06
'' - single data type
'' - type conversion to / from single
''
'' CHECK: 10.5
''------------------------------------------------------------------------------
[Alias = "printf", Extern = "C"] _
Declare Function printf(msg As Byte Ptr, ...) As Integer

Function main() As Integer
    Dim s As Single
    s = 10.5
    printf("%.2f", s)
    return 0
End Function
