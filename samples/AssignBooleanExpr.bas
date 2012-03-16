' Declare printf
[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Function main() As Integer
	Dim t As Bool
	Dim f As Bool
	Dim s As Single
	s = 1.6
	t = s = 1.6
	f = s <> 1.6
	printf("t = %i, f = %i\n", t, f)
	Return 0
End Function
