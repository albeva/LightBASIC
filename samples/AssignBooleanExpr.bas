' Declare printf
[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Function main() As Integer
	Dim i As Integer
	Dim b As Bool
	i = 10
	b = i = 10
	printf("b = %i\n", b)
	return 0
End Function
