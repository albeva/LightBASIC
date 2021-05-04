[alias="printf"] _
declare function printf(fmt as zstring, ...) as integer

var d as double = 3.14
var s as single = d
var i as integer = d

printf "%lf, %d", d, i
