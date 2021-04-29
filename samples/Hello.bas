' Declare func from C std lib: int puts(const char*);
[Alias = "puts", DiscardableResult] _
declare function print(str as zstring) as integer

sub say(message as zstring)
    print message
end sub

sub main
    ' deduce type of message from value
    var message = "Hello World!"

    ' no need for braces when function call is not inside expression
    say message
end sub
