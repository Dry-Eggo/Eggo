
section .bss

	std_print_string.src resb 1024
	println.msg resb 1024
section .data

	main.if0.name db "In IF" ,0
	main.if0.text db "In Else" ,0
section .text
global _start

extern std_terminate_process
extern std_print_string
extern std_flush
extern std_print_int
extern std_copy
extern std_clear_string

_start:


	mov rbp, rsp
	call main

	extern std_print_string

	extern std_flush

get:

	push rbp

	mov rbp, rsp

	mov rax, 10
	pop rbp
	ret

get2:

	push rbp

	mov rbp, rsp

	mov rax, 10
	pop rbp
	ret

println:

	push rbp

	mov rbp, rsp

	mov [rbp -8], rdi

	mov rdi, println.msg
	call std_print_string

	call std_flush

	pop rbp

	lea rdi, [println.msg]
	mov rsi, 1024
	call std_clear_string
	ret

main:

	push rbp

	mov rbp, rsp

	call get

	mov r8, rax
	call get2

	mov r9, rax
	cmp r8, r9
	je main.if0
	jne main.if0else
	
main.if0:
	mov rdi, main.if0.name
	mov rsi, println.msg
	call std_copy

	mov rdi, main.if0.name

	call println

	jmp main.if0end
	main.if0else:
	mov rdi, main.if0.text
	mov rsi, println.msg
	call std_copy

	mov rdi, main.if0.text

	call println
jmp main.if0end
main.if0end:
	mov rdi, 0
	pop rbp

	call std_terminate_process

section .note.GNU-stack