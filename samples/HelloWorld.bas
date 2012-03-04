' Declare puts from libc
[Extern = "C", Alias = "puts", Lib = "libc"] _
DECLARE FUNCTION print(str AS BYTE PTR) AS INTEGER

function message() AS BYTE PTR
    return "I am the first LightBASIC app"
end function

' implement main function
FUNCTION main(argc AS INTEGER, argv AS BYTE PTR PTR) AS INTEGER
    DIM greeting AS BYTE PTR
    greeting = "Hello World,"
    print(greeting)
    print(message())
    print("Bye fo now...")
END FUNCTION

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