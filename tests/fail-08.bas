''------------------------------------------------------------------------------
'' fail-03
'' - Pass incomptible pointer to a SUB
''   Integer Ptr -> Single Ptr
''------------------------------------------------------------------------------
Sub foo(p As Single Ptr)
End Sub

Sub main
    Dim i As Integer
    foo(&i)
End Sub

