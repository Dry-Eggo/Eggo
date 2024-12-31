
section .bss

	std_print_string.src resb 1024
	std_print_int.val resb 1024
	println.msg resb 1024
	for1.i resb 1
section .data

	main.if0.name db "In If" ,0
	main.if0.text db "In Else" ,0
	for1.label db "Current Number : " ,0
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

	extern std_print_int

get:

	push rbp

	mov rbp, rsp

	mov rax, 10
	pop rbp
	ret

get2:

	push rbp

	mov rbp, rsp

	mov rax, 30
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

	mov r8, 10
	mov r9, 1
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

	mov dword [for1.i], 0
for1:

	mov eax, [for1.i]
	cmp eax, 10
	jge for1_end

	mov rdi, for1.label
	call std_print_string

	mov rdi, [for1.i]
	call std_print_int

	call std_flush
	add byte [for1.i], 1
	jmp for1

for1_end:
jmp main.if0end
main.if0end:
	mov rdi, 0
	pop rbp

	call std_terminate_process

section .note.GNU-stack