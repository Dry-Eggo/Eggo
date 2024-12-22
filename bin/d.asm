
section .bss

section .data

	compiler_name db "Eggo", 0

section .text
global _start

extern std_terminate_process
extern std_print_string
extern std_flush
extern std_print_int
extern std_copy

_start:


	mov rbp, rsp

	call foo

	mov rdi, 0
	call std_terminate_process

foo:

	mov rbp, rsp
	ret

section .note.GNU-stack