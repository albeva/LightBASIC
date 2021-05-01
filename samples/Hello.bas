' Declare func from C std lib: int puts(const char*);
[Alias = "puts", DiscardableResult] _
declare function print(str as zstring) as integer

function getMessage(from as zstring) as zstring
    return from
end function

sub main
    print getMessage("Hello World")
end sub
