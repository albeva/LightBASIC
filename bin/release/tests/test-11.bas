''------------------------------------------------------------------------------
'' test-11
'' - getting address
'' - returning a pointer
'' - dereferencing function result
'' - using params from function
''
'' CHECK: i = 42{{$}}
''------------------------------------------------------------------------------
[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Function ultimateAnswer(ip As Integer Ptr) As Integer Ptr
    *ip = 42
    Return ip
End Function

Function main() As Integer
    Dim i As Integer
    printf("i = %d\n", *ultimateAnswer(&i))
    Return 0
End Function
