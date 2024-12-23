

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

  pop rcx
  pop rdx
  pop rsi
  ret

; ---------------------------------------------

global std_len

; rdi --- subject string
; rax --- return value

std_len:
  mov rsi, rdi

  xor rax, rax   ; counter

.beg:
  cmp byte [rsi + rax], 0
  je .end
  inc rax
  jmp .beg
.end:
  ret

; ---------------------------------------------

global std_print_int

std_print_int:
  push rcx

  mov rax, rdi

  xor rcx, rcx

  mov rsi, buf

  test rax, rax
  jns .convert
  mov byte [rsi], '-'

  neg rax
  inc rsi

.convert:
  mov rdx, 0
  mov rbx, 10
  div rbx

  add dl, '0'

  mov [rsi + rcx], dl
  inc rcx
  test rax, rax
  jnz .convert

.reverse:
  lea rdi, buf
  lea rsi, [buf + rcx - 1]

.rv_loop:
  cmp rdi, rsi
  jge .done_rev

  mov al, [rdi]
  mov bl, [rsi]
  mov [rdi], bl
  mov [rsi], al

  inc rdi
  dec rsi

  jmp .rv_loop
.done_rev:
  lea rdi, buf
  call std_print_string

  pop rcx

  ret

; -----------------------------------------------

global std_flush

std_flush:
  mov rdi, nl
  call std_print_string
  ret

; ------------------------------------------------

global std_copy

; rdi --- src address
; rsi --- dst address

std_copy:
  xor rcx, rcx

.loop:

  mov al, [rdi + rcx]
  mov [rsi + rcx], al

  cmp al, 0

  je .done
  inc rcx
  jmp .loop

.done:
  ret


; ------------------------------------------------
section .bss
  buf resb 20

section .data
  nl db 10

section .note.GNU-stack
