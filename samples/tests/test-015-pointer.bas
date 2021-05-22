''------------------------------------------------------------------------------
'' test-015-pointer.bas
'' - take address of
'' - dereference
''
'' CHECK: 10
''------------------------------------------------------------------------------
[alias="printf"] _
declare function printf(fmt as zstring, ...) as integer

var i = 10
var ip = @i
var iv = *ip
printf "%d\n", iv
