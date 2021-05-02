[Alias="printf"] _
declare function printf(fmt as zstring, ...) as integer

var a = 10
var b = a

sub main
    printf "%d", b
end sub
