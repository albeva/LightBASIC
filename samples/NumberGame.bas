'' printf
[Alias = "printf"] _
Declare Function printf(fmt As ZString, ...) As Integer
'' scanf
[Alias = "scanf"] _
Declare Function scanf(fmt As ZString, ...) As Integer
'' srand
[Alias = "srand"] _
Declare Sub srand(seed As UInteger)
'' rand
[Alias = "rand"] _
Declare Function rand As Integer
'' time
[Alias = "time"] _
Declare Function time(time_t As Any Ptr) As ULong

'' initialize a random seed
srand time(null)

'' the secret number
Var secret = rand() Mod 100
Var answer = 0

'' show help
printf "Guess a number between 1 and 100. 0 to exit. You have 25 tries\n"

'' Try for 25 times
For i = 1 To 25
    printf "Attempt %d: ", i
    scanf "%d", @answer
    If answer = 0 Then
        Return
    Else If answer = secret Then
        printf "Correct\n"
        Return
    Else If answer > secret Then
        printf "Nope. Too big!\n"
    Else
        printf "Nooo. Try bigger!\n"
    End If
Next

'' too bad
printf "Game Over. Correct number was %d\n", secret
