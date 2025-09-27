.global read
.global write
.global exit
.global _start
.section .text

read:
li a7, 63
ecall
ret

write:
li a7, 64
ecall
ret

exit:
li a7, 93
ecall

hahaha:


_start:
# passing argc, argv[]
lw a0, 0(sp)
addi a1, sp, 4 # 8 for 64-bit mode
call main
j exit