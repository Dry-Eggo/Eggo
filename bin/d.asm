
section .bss

section .data

	name db "Eggo", 0

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

	extern std_print_string

main:

	mov rbp, rsp

	call std_print_string

	mov rdi, 0
	call std_terminate_process

section .note.GNU-stack