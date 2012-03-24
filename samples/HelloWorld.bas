' Declare puts from libc
[Extern = "C", Alias = "puts", Lib = "crt"] _
Declare Function print(str As Byte Ptr) As Integer

Function message() As Byte Ptr
    Return "I am the first LightBASIC app"
End Function

' implement main function
Function main(argc As Integer, argv AS Byte Ptr Ptr) As Integer
    Dim greeting As Byte Ptr
    greeting = "Hello World,"
    print(greeting)
    print(message())
    print("Bye fo now...")
    Return 0
End Function
