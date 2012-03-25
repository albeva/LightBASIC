''------------------------------------------------------------------------------
'' test-07
'' - calls to external functions
'' - pointers
'' - null literal assignment
'' - null literal comparison
''
'' CHECK: ip = 0x{{[0-9]+}}, *ip = 10
''------------------------------------------------------------------------------

[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

[Alias = "malloc"] _
Declare Function malloc(size As Integer) As Any Ptr

[Alias = "free"] _
Declare Function free(data As Any Ptr) As Integer

' entry point
Function main() As Integer
    Dim ip As Integer Ptr
    ip = null
    ip = malloc(4)
    If ip <> null Then
        *ip = 10
        printf("ip = %p, *ip = %i\n", ip, *ip)
        free(ip)
    End If
    Return 0
End Function
