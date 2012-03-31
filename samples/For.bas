''------------------------------------------------------------------------------
'' test-17
'' - FOR statement
''
'' CHECK: 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
''------------------------------------------------------------------------------

[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

'' get pointer
Function getPtr(ip As Integer Ptr) As Integer Ptr
    Return ip
End Function

'' entry point
Sub main
    '' Simple QBASIC style
    Dim i As Integer
    For i = 1 To 10
        printf "%i ", i
    Next
    printf "\n"
    
    '' Declare variable inline
    For Var x = 1 to 10
        printf "%i ", x
    Next
    printf "\n"
    
    '' more verbose, but same
    For z As Integer = 1 to 10
        printf "%i ", z    
    Next
    printf "\n"
        
    '' With expression and a STEP
    For *getPtr(&i) = 0 to 18 Step 3
        printf "%i ", i        
    Next
    printf "\n"
End Sub
