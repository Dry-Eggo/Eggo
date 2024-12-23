
section .bss

section .data

section .text
global _start

extern std_terminate_process
extern std_print_string
extern std_flush
extern std_print_int
extern std_copy

_start:


	mov rbp, rsp
	call main

	mov rdi, 0
	call std_terminate_process

terminate:

	mov rbp, rsp

	mov [rbp -8], rdi

	call std_terminate_process
	ret

main:

	mov rbp, rsp

	mov rdi, 40

	call terminate
	ret

section .note.GNU-stack