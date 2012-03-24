''------------------------------------------------------------------------------
'' test-01
'' - extern C function
'' - alias attribute
'' - pass string argument
'' CHECK: Hello World!
''------------------------------------------------------------------------------
[Alias = "puts", Extern = "C"] _
Declare Function print(msg As Byte Ptr) As Integer

Function main() As Integer
    print("Hello World!")
    return 0
End Function
