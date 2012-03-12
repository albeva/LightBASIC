' Declare printf
[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

' malloc
[Alias = "malloc"] _
Declare Function malloc(size As Integer) As Integer Ptr

' free
' need SUBs
[Alias = "free"] _
Declare Function free(data As Integer Ptr) As Integer

' entry point
Function main() As Integer
    Dim ip As Integer Ptr
    ip = 0
    ip = malloc(4)
    *ip = 10
    printf("ip = %p, *ip = %i\n", ip, *ip)
    free(ip)
    Return 0
End Function
