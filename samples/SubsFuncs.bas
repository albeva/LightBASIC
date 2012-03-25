' Declare puts from libc
[Extern = "C", Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Sub main
    printf ("%i, %s, %f, %i\n"), 10, "hi", 12.3, (true <> false)
End Sub
