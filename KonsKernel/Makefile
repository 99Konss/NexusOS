# NexusOS Makefile mit GUI Support
CC = gcc
LD = ld
ASM = nasm

# Compiler-Flags
CFLAGS = -w -m32 -ffreestanding -fno-stack-protector -fno-pic -nostdlib -I kernel -I kernel/GUI/include

# Linker-Flags
LDFLAGS = -m elf_i386 -T kernel/linker.ld -nostdlib

# Assembler-Flags
ASMFLAGS = -felf32

# C Dateien
C_SOURCES = $(wildcard kernel/*.c kernel/*/*.c kernel/GUI/core/*.c kernel/GUI/widgets/*.c)
C_OBJ = ${C_SOURCES:.c=.o}

# Header
C_HEADERS = $(wildcard kernel/*.h kernel/*/*.h kernel/GUI/core/*.h kernel/GUI/widgets/*.h kernel/GUI/include/*.h)

# Assembly Dateien
ASM_SOURCES = $(wildcard kernel/*.asm kernel/*/*.asm)
ASM_OBJ = ${ASM_SOURCES:.asm=.o}

# Standard Target
all: kernel.bin

# Kernel bauen (alle .o files linken)
kernel.bin: ${C_OBJ} ${ASM_OBJ}
	${LD} ${LDFLAGS} -o $@ $^

# .c Dateien kompilieren
%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

# .asm Dateien assemblieren
%.o: %.asm
	${ASM} ${ASMFLAGS} $< -o $@

# Clean - alles löschen
clean:
	rm -rf *.o kernel/*.o kernel/*/*.o kernel/GUI/core/*.o kernel/GUI/widgets/*.o kernel.bin KonsKernel.iso iso

# Direkt in QEMU booten (mit GUI)
run: kernel.bin
	qemu-system-x86_64 -kernel kernel.bin -vga std -m 256M

# QEMU mit Debug-Ausgaben
run-debug: kernel.bin
	qemu-system-i386 -kernel kernel.bin -vga std -m 256M -d int -no-reboot -serial stdio

# QEMU mit GDB Server (zum Debuggen)
run-gdb: kernel.bin
	qemu-system-i386 -kernel kernel.bin -vga std -m 256M -s -S

# ISO erstellen (für Bootloader)
iso: kernel.bin
	mkdir -p iso/boot/grub
	cp kernel.bin iso/boot/
	echo 'menuentry "NexusOS" {' > iso/boot/grub/grub.cfg
	echo '  multiboot /boot/kernel.bin' >> iso/boot/grub/grub.cfg
	echo '  boot' >> iso/boot/grub/grub.cfg
	echo '}' >> iso/boot/grub/grub.cfg
	grub-mkrescue -o KonsKernel.iso iso

# ISO in QEMU testen
run-iso: iso
	qemu-system-i386 -cdrom KonsKernel.iso -vga std -m 256M

# Auf echter Hardware (Hinweis)
run-hardware: iso
	@echo "========================================="
	@echo "Auf USB-Stick kopieren:"
	@echo "sudo dd if=KonsKernel.iso of=/dev/sdX bs=4M status=progress"
	@echo "========================================="
	@echo "WICHTIG: /dev/sdX durch richtiges Gerät ersetzen (nicht /dev/sda!)"

# Phony Targets (keine echten Dateien)
.PHONY: all clean run run-debug run-gdb iso run-iso run-hardware
