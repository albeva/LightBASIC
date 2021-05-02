' Declare func from C std lib: int puts(const char*);
[Alias = "printf"] _
declare function printf(str as zstring, ...) as integer

printf "Hello\n"
