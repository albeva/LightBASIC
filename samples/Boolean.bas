' Declare printf
[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Function main() As Integer
	Dim b As Bool
	b = true
	printf("b = %i\n", b)
	Return 0
End Function
