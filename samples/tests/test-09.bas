''------------------------------------------------------------------------------
'' test-09
'' - signed and unsigned integers
''
'' CHECK: si = 10, ui = 10{{$}}
''------------------------------------------------------------------------------
[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Function main() As Integer
    Dim si As Integer
    Dim ui As UInteger 
    si = 10
    ui = si
    printf("si = %d, ui = %u", si, ui)
    Return 0
End Function
