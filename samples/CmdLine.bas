' Declare printf
[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Function main(argc As Integer, argv As Byte Ptr Ptr) As Integer
	printf("0 = %s\n", *argv)
	return 0
End Function
