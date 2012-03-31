''------------------------------------------------------------------------------
'' test-12
'' - Double dereference
''
'' CHECK: i = 10{{$}}
''------------------------------------------------------------------------------
[Extern = "C", Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Sub main
    var i = 0
    var ip = &i
    var ipp = &ip
    **ipp = 10
    printf "i = %i\n", i
End Sub
