' Declare printf
[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Function main(argc As Integer, argv As Byte Ptr Ptr) As Integer
	If argc == 1 Then
		printf("1")
	Else
		printf("argc = %d", argc)		
	End If
	return 0
End Function
