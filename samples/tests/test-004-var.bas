''------------------------------------------------------------------------------
'' test-004-var.bas
'' - declare global variable
'' - declare local variable
'' - assign variables
'' - pass variables as arguments
''
'' CHECK: Hello, World!
''------------------------------------------------------------------------------
import cstd

var a = "Hello"
var b = "World"
var c = a
sendMessage c

sub sendMessage(greeting as zstring)
    var exclamation = getExclamation()
    printf "%s, %s%s", greeting, b, exclamation
end sub'

function getExclamation() as zstring
    return "!"
end function
