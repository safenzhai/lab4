.file "hello_asm.S"
.text
.align 2
.global change_r8

change_r8:
	mov r8, #0
	mov pc, lr
