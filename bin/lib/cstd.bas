[Alias = "printf"] _
Declare Function printf(fmt As ZString, ...) As Integer

[Alias = "puts"] _
Declare Function puts(str As ZString) As Integer

[Alias = "getchar"] _
Declare Function getchar() As Integer

[Alias = "scanf"] _
Declare Function scanf(fmt As ZString, ...) As Integer

[Alias = "srand"] _
Declare Sub srand(seed As UInteger)

[Alias = "rand"] _
Declare Function rand As Integer

[Alias = "time"] _
Declare Function time(time_t As Any Ptr) As ULong
