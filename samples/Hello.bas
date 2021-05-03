[alias="printf"] _
declare function printf(fmt as zstring, ...) as integer

var val = ----5
var neg = ----val
printf "%d", neg
