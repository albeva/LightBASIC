''------------------------------------------------------------------------------
'' fail-07
'' - Using a SUB in an expression
''------------------------------------------------------------------------------
Sub foo
End Sub

Function main As Integer
    Var i = foo()
End Function
