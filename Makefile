# Makefile definitivo para OSPD (Generacion de ISO con GRUB + Soporte HDA)

CC = /usr/bin/gcc
AS = nasm
LINKER = /usr/bin/gcc

CFLAGS = -c -m32 -nostdlib -ffreestanding -O0 -no-pie \
         -fno-asynchronous-unwind-tables -fno-stack-protector -I./headers

ASFLAGS = -f elf32

OBJ = head.o kernel.o panic.o atadisk.o keyboard.o shell.o cmd.o fs.o

# Regla principal: compila, inyecta el disco crudo, arma la ISO y arranca
all: ospdisk.img mykernel.bin ospd.iso run

# Enlazado puro del ejecutable del Kernel
mykernel.bin: $(OBJ)
	$(LINKER) -T linker.ld -m32 -nostdlib $(OBJ) -o mykernel.bin -no-pie -Wl,-n -Wl,-z,max-page-size=4096

# DISCO VIRTUAL DATA HDA: Archivo plano de 10 megabytes para tus carpetas dinamicas
ospdisk.img:
	qemu-img create -f raw ospdisk.img 10M

# FUNDICION DE LA ISO BOOTABLE: Armamos la estructura de GRUB real y fundimos con xorriso
ospd.iso: mykernel.bin
	mkdir -p iso_root/boot/grub
	cp mykernel.bin iso_root/boot/mykernel.bin
	@echo 'set timeout=0' > iso_root/boot/grub/grub.cfg
	@echo 'set default=0' >> iso_root/boot/grub/grub.cfg
	@echo 'menuentry "OSPD - Operating System of the Petted Dodo" {' >> iso_root/boot/grub/grub.cfg
	@echo '    multiboot /boot/mykernel.bin' >> iso_root/boot/grub/grub.cfg
	@echo '    boot' >> iso_root/boot/grub/grub.cfg
	@echo '}' >> iso_root/boot/grub/grub.cfg
	grub-mkrescue -o ospd.iso iso_root

# Compilacion modulo por modulo
head.o: kernel/boot/head.asm
	$(AS) $(ASFLAGS) kernel/boot/head.asm -o head.o

kernel.o: kernel/kernel.c
	$(CC) $(CFLAGS) kernel/kernel.c -o kernel.o

panic.o: kernel/panic.c
	$(CC) $(CFLAGS) kernel/panic.c -o panic.o

atadisk.o: kernel/drivers/atadisk.c
	$(CC) $(CFLAGS) kernel/drivers/atadisk.c -o atadisk.o

keyboard.o: kernel/drivers/keyboard.c
	$(CC) $(CFLAGS) kernel/drivers/keyboard.c -o keyboard.o

shell.o: shell/shell.c
	$(CC) $(CFLAGS) shell/shell.c -o shell.o

cmd.o: shell/cmd.c
	$(CC) $(CFLAGS) shell/cmd.c -o cmd.o

fs.o: kernel/fs/fs.c
	$(CC) $(CFLAGS) kernel/fs/fs.c -o fs.o

# LANZAMOS QEMU MAXIMO: Booteamos desde el CD-ROM virtual de la ISO y acoplamos el disco duro HDA en paralelo
run:
	qemu-system-i386 -cdrom ospd.iso -hda ospdisk.img

# Limpieza absoluta de la trinchera
clean:
	rm -f $(OBJ) mykernel.bin ospd.iso ospdisk.img
	rm -rf iso_root
