''------------------------------------------------------------------------------
'' test-09
'' - values returned from functions
'' - absence of duplicate strings (same addr)
'' - comparison of function results
''
'' CHECK: getInt = 10, getStr = Hello, getDouble = 3.1415{{$}}
'' CHECK: a = 1, b = 1, c = 1{{$}}
''------------------------------------------------------------------------------
[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Function getInt() As Integer
    Return 10
End Function

Function getStr() As Byte Ptr
    Return "Hello"
End Function

Function getDouble() As Double
    Return 3.1415
End Function

Function main() As Integer
    printf("getInt = %i, getStr = %s, getDouble = %.4f\n", _
            getInt(), getStr(), getDouble())
    printf("a = %i, b = %i, c = %i\n", _
            getInt() = 10, getStr() = "Hello", getDouble() = 3.1415)
    Return 0
End Function
