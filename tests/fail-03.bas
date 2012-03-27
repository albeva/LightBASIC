''------------------------------------------------------------------------------
'' fail-03
'' - Assigning an incompatible type
''   integer -> Any Ptr
''------------------------------------------------------------------------------
Sub main
    Dim i As Integer
    Dim p As Any Ptr
    p = i
End Sub
