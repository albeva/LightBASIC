LightBASIC is a simple toy compiler written in c++ using llvm and boost libraries. Its goal is simply to explore how compilers work internally and try out some ideas.

http://lightbasic.com/

```VB
' Declare puts from libc
[Alias = "puts"] _
Declare Function puts(str As Byte Ptr) As Integer

' implement main function
Function main(argc AS Integer, argv AS Byte Ptr Ptr) AS Integer
    Dim msg AS Byte Ptr
    msg = "Hello World"
    puts(msg)
End Function
```
