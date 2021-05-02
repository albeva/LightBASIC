''------------------------------------------------------------------------------
'' test-001-hello.bas
'' - extern C function
'' - alias attribute
'' - pass zstring argument
''
'' CHECK: Hello World!
''------------------------------------------------------------------------------
[alias="puts"] _
declare function print(str as zstring) as integer

print "Hello World!"
