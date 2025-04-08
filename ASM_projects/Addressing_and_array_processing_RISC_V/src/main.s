# threshold -> a6
# res1 -> t1
# res2 -> s5

.text 
.global lr6 

lr6: 
    la     s0, array             # s0 <- адрес начала array
    addi   a0, zero, a           # a0 = a
    addi   a0, a0, b             # a0 = a0 + b = a + b
    addi   a0, a0, c             # a0 = a0 + c = a + b + c
    sw     a0, 0(s0)             # M[s0] = array[0] <- a0 = a + b + c
    addi   s0, s0, 4             # s0 = s0 + 4 = адрес array[1]
    addi   s1, zero, 9           # s1 = 9 = счетчик

loop:
    addi   a0, a0, a             # a0 + a = array[9 - s1] + a = array[i] + a
    addi   a0, a0, b             # a0 + b = array[9 - s1] + a + b = array[i] + a + b
    addi   a0, a0, -c            # a0 - c = array[9 - s1] + a + b - c = array[i] + a + b - c
    sw     a0, 0(s0)             # M[s0] = array[9 - s1 + 1] = array[i+1] <- a0 = array[i] + a + b - c
    addi   s0, s0, 4             # s0 = s0 + 4 = адрес array[9 - s1 + 2] = array[i+2]
    addi   s1, s1, -1            # s1 = s1 - 1 = уменьшение счетчика
    bne    s1, zero, loop        # если s1 != 0, переход к loop 

    la     s0, array             # s0 <- адрес начала array
    lw     t0, 7*4(s0)           # t0 <- M[s0 + 7*4] = array[7] 
    add    a5, zero, t0          # a5 = t0 = array[7]
    lw     t0, 4*4(s0)           # t0 <- M[s0 + 4*4] = array[4]
    add    a5, a5, t0            # a5 = a5 + t0 = array[7] + array[4] 
    lw     t0, 1*4(s0)           # t0 <- M[s0 + 1*4] = array[1]
    add    a5, a5, t0            # a5 = a5 + t0 = array[7] + array[4] + array[1]
     
    lw     a6, threshold         # a6 <- threshold 
    ble    a5, a6, continue      # если a5 <= a6, переход к continue
    lw     s5, 8*4(s0)           # s2 <- M[s0 + 8*4] = array[8] 
    addi   t0, zero, a           # t0 = a
    or     s5, s5, t0            # s5 = s5 | t0 = array[8] | a
    j      end                   # переход к end
                   
continue:                        # если a0 <= a6   
    lw     t1, 5*4(s0)           # t1 <- M[s0 + 5*4] = array[5]
    lw     t0, 7*4(s0)           # t0 <- M[s0 + 7*4] = array[7]
    or     t1, t1, t0            # t1 = t1 | t0 = array[5] | array[7]

end:
    li     a0, 0           	 # a0 = 0 = код возврата
    li     a7, 10          	 # a7 = 10 = служебная команда завершения работы
    ecall                        # вызов системы для завершения работы программы

.data 
array:     .word 1,2,3,4,5,6,7,8,9,10 
threshold: .word 175 
.equ a,10  
.equ b,7  
.equ c,9
