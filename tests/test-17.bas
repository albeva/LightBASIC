''------------------------------------------------------------------------------
'' test-17
'' - FOR statement
''
'' CHECK: 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
''------------------------------------------------------------------------------

[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Sub main
    For Var i = 1 To 10
        printf "%i, ", i
    Next
    printf "\n"
End Sub
