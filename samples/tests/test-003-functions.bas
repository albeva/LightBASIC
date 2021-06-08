''------------------------------------------------------------------------------
'' test-003-functions.bas
'' - explicit main function
'' - declare local functions
'' - call functions
'' - pass arguments
'' - return getLlvmValue
''
'' CHECK: Hello, World!
''------------------------------------------------------------------------------
import cstd

sub main
    say "Hello"
end sub

sub say(prefix as zstring)
    printf "%s, %s!", prefix, getSuffix()
end sub

function getSuffix() as zstring
    return "World"
end function
