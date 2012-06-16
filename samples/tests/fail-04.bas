''------------------------------------------------------------------------------
'' fail-03
'' - Assigning an incompatible type in initalization
''   integer -> Any Ptr
''------------------------------------------------------------------------------
Sub main
    Dim i As Integer
    Dim p As Any Ptr = i
End Sub
