' Declare printf
[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

' entry point
Function main() As Integer
    Dim i As Integer
    Dim ip As Integer Ptr
    ip = &i
	*ip = 10
    printf("ip = %p, *ip = %i\n", ip, *ip)
    Return 0
End Function
