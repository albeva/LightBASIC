' Declare puts from libc
[Extern = "C", Alias = "puts", Lib = "libc"] _
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

/'
    // This is equivelent C program
    extern "C" {
        int puts(const char *);
    }
     
    const char * message() {
        return "I am the first LightBASIC app";
    }

    int main(int argc, char **argv) {
        const char * greeting = "Hello World,";
        puts(greeting);
        puts(message());
        puts("Bye fo now...");
    }
'/