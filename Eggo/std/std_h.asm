
section .text
global std_terminate_process

; rdi --- exit code

std_terminate_process:
mov rax, 60
syscall


global std_print_string

std_print_string:

push rsi
push rdx
push rcx
push rax

mov rsi, rdi
xor rcx, rcx

find_length:
cmp byte [rsi + rcx], 0
je length_found

inc rcx
jmp find_length

length_found:
mov rax, 1
mov rdi, 1
mov rdx, rcx
syscall

pop rax
pop rcx
pop rdx
pop rsi
ret

; ---------------------------------------------

global std_len

; rdi --- subject string
; rax --- return value

std_len:
push rax
push rsi

mov rsi, rdi

xor rax, rax   ; counter

.beg:
cmp byte [rsi + rax], 0
je .end
inc rax
jmp .beg
.end:
pop rsi
pop rax
ret
; ---------------------------------------------

section .text
global std_print_int

; --- rdi :: number to convert
; --- std_print_int.value :: ret val
std_print_int:
	mov rax, rdi
	mov rdi, std_print_int.value
	xor rcx, rcx

.convert:
	xor rdx, rdx
	div dword [divisor]
	add dl, 48
	mov [rdi + rcx], dl
	inc rcx

	test eax, eax
	jnz .convert

	mov byte [rdi + rcx], 0		; Null Terminate , 0

	lea rsi, std_print_int.value
	lea rdi, [std_print_int.value + rcx -1]
	xor rax, rax

.reverse:
	cmp rsi, rdi
	jge .done

	mov al, [rsi]
	mov bl, [rdi]
	mov [rsi], bl
	mov [rdi], al

	inc rsi
	dec rdi
	
	jmp .reverse
.done:	
	lea rdi, std_print_int.value
	call std_print_string
	ret
; -----------------------------------------------

global std_flush
std_flush:
push rdi
mov rdi, nl
call std_print_string
pop rdi
ret

; ------------------------------------------------

global std_copy

; rdi --- src address
; rsi --- dst address

std_copy:
push rax
push rcx
xor rcx, rcx

.loop:

mov al, [rdi + rcx]
mov [rsi + rcx], al

cmp al, 0

je .done
inc rcx
jmp .loop

.done:
pop rcx
pop rax
ret

; ------------------------------------------------
global std_clear_string

; rdi --- src
; rsi --- size of src
std_clear_string:

push rax

test rdi, rdi
je .done

test rsi, rsi
jz .done

.clear:
mov byte [rdi], 0

inc rdi
dec rsi

jnz .clear


.done:
pop rax
ret

; ------------------------------------------------
section .bss
buf resb 20
std_print_int.value resb 32  ; Reserve space for a 32-byte buffer to store the string
section .data
nl db 10
divisor dd 10

section .note.GNU-stack
