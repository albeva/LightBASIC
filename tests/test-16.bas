''------------------------------------------------------------------------------
'' test-12
'' - triple dereference
''
'' CHECK: i = 10{{$}}
''------------------------------------------------------------------------------
[Extern = "C", Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Sub main
    var i = 0
    var ip = &i
    Dim ipp As Integer Ptr Ptr
    var ippp = &ipp
    *ippp = &ip
    ***ippp = 10
    printf "i = %i\n", i
End Sub
