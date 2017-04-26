LightBASIC is a simple toy compiler written in c++ using llvm and boost libraries. Its goal is simply to explore how compilers work internally and try out some ideas.

http://lightbasic.com/

```VB
' Declare puts from libc
[Alias = "puts"] _
DECLARE FUNCTION puts(str AS BYTE PTR) AS INTEGER

' implement main function
FUNCTION main(argc AS INTEGER, argv AS BYTE PTR PTR) AS INTEGER
    DIM msg AS BYTE PTR
    msg = "Hello World"
    puts(msg)
END FUNCTION
```
