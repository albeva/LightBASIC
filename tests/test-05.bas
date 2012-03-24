''------------------------------------------------------------------------------
'' test-05
'' - boolean data type
'' - conversion to boolean
'' - equality and inequality
''
'' CHECK: b1 = 1
'' CHECK: b2 = 0
'' CHECK: b3 = 1
'' CHECK: b4 = 1
'' CHECK: b5 = 1
'' CHECK: b6 = 0
'' CHECK: b7 = 1
'' CHECK: b8 = 0
'' CHECK: b9 = 0
'' CHECK: b10 = 1
'' CHECK: b11 = 1
'' CHECK: b12 = 1
''------------------------------------------------------------------------------
[Alias = "printf"] _
Declare Function printf(str As Byte Ptr, ...) As Integer

Function main() As Integer
    Dim b1 As Bool
    Dim b2 As Bool
    Dim b3 As Bool
    Dim b4 As Bool
    Dim b5 As Bool
    Dim b6 As Bool
    Dim b7 As Bool
    Dim b8 As Bool
    Dim b9 As Bool
    Dim b10 As Bool
    Dim b11 As Bool
    Dim b12 As Bool
    
    Dim i As Integer
    i = 10
    
    Dim s As Single
    s = 3.1415

    b1 = true
    b2 = false
    b3 = i
    b4 = s
    b5 = i = 10
    b6 = b1 <> 10
    b7 = True <> False
    b8 = True = False
    b9 = s = i
    b10 = s <> i
    b11 = s = 3.1415
    b12 = b1 = true = b7

    printf("b1 = %d\nb2 = %d\nb3 = %d\nb4 = %d\nb5 = %d\nb6 = %d\n", _
            b1, b2, b3, b4, b5, b6)
    
    printf("b7 = %d\nb8 = %d\nb9 = %d\nb10 = %d\nb11 = %d\nb12 = %d\n", _
            b7, b8, b9, b10, b11, b12)
    Return 0
End Function

