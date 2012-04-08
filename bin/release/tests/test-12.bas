''------------------------------------------------------------------------------
'' test-12
'' - subs
'' - passing parameters to sub without parens and parens around expressions
''
'' CHECK: a: 1, 1{{$}}
'' CHECK: b: 2, 0{{$}}
''------------------------------------------------------------------------------
[Extern = "C", Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Sub main
    printf ("%s: %i, %i\n"), "a", 1,  (true <> false)
    printf ("%s: %i, %i\n",  "b", 2,  (true = false))
End Sub
