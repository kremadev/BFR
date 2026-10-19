
.data
cls: .ascii "\033[2J\033[H"
len = . - cls
.bss
cells: .skip 30000
.text
.globl _start
clear:
mov $1, %rax
mov $1, %rdi
lea cls(%rip), %rsi
mov $len, %rdx
syscall
ret

_start:
xor %rdi, %rdi
call clear
rdrand %ax
mov %al, cells(%r12)
decb cells(%r12)
mov $60, %rax
movzbl cells(%r12), %edi
syscall


.section .note.GNU-stack, "", @progbits
