build:
	@nasm -f elf64 tests/test.asm -o tests.o
	@nasm -f elf64 std/std_h.asm -o std/std_h.o
	@ld tests.o std/std_h.o -o test

run:
	@./test

clean:
	@rm test
