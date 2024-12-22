section .text
global _start

extern std_print_string
extern std_terminate_process
extern std_len
extern std_print_int
extern std_flush

_start:
  mov rdi, msg
  call std_print_string

  mov rdi, hello
  call std_len
  
  mov rdi, rax
  call std_print_int

  call std_flush

  mov rdi, 1
  call std_terminate_process


section .data
  hello db "AbdulJabbar and chuchu", 0
  msg db "Length of 'AbdulJabbar and chuchu' : ", 0

section .note.GNU-stack
