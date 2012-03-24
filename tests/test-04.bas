''------------------------------------------------------------------------------
'' test-04
'' - test pointer dereferencing from function result
'' - test address of
'' - test assignment
'' - test passing argument to a function
'' - test single data type
''
'' CHECK: 3.1415
''------------------------------------------------------------------------------
[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Function getPtr (sp As Single Ptr) As Single Ptr
    Return sp
End Function

Function main() As Integer
    Dim s As Single
    *getPtr(&s) = 3.1415
    printf("i = %.4f\n", s)
    Return 0
End Function

