

build :
	@nasm -f elf64 ../../Eggo/std/std_h.asm -o ../../Eggo/std/bin/std_h.o
	@nasm -f elf64 ../d.asm -o d.o
	@ld d.o ../../Eggo/std/bin/std_h.o -o ../d

clean:
	@rm d.o ../d ../d.o

.PHONY: clean build
