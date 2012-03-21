' Declare printf
[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Function main(argc As Integer, argv As Byte Ptr Ptr) As Integer
    If argc = 1 Then
        printf("No arguments passed\n")
    Else If argc = 2 Then
        printf("1 argument passed\n")
    Else
        printf("%d - 1 arguments\n", argc)
    End If
    Return 0
End Function
