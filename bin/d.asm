
section .bss

section .data

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

get:

	push rbp

	mov rbp, rsp

	mov rax, 10
	pop rbp
	ret

main:

	push rbp

	mov rbp, rsp

	mov rdi, 0
	pop rbp

	call std_terminate_process

section .note.GNU-stack