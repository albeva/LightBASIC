' Declare func from C std lib: int puts(const char*);
[Alias = "printf"] _
declare function print(str as zstring, ...) as integer

sub main
    say getMessage("Hello World")
end sub

function getMessage(from as zstring) as zstring
    return from
end function

sub say(message as zstring)
    print "%s", message
end sub
