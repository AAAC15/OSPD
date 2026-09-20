# OSPD: the Operating System of the Petted Dodo

Un OS de 32 bits que hice por meros propositos educacionales, se nota todavia que, en primer lugar, el 
Makefile esta vacio, y que ademas, no tiene comandos funcionales (es una shell triste)

Como se hace notar, todavia esta en desarrollo, y tengo ciertas ideas para su posterior avance.
Tengo reciclados ciertos archivos (los que estan en lowercase) porque ya estaba intentando desarrollar
un OS antes. Muchas batallas, lo rehice desde 0.

Vamos con ciertos puntos:

## head.asm: Multiboot
Uso el estandar MultiBoot para el gestor de arranque GRUB (el de casi todos los linux).
Lo hice para no lidiar con el salto de 16 a 32 bits a cada rato.

## kernel/...: Archivos de kernel
Aca estoy desarrollando los respectivos archivos del kernel, (fs/ esta vacio, espeeeren) donde tengo pensado
inspirarme en sistemas operativos UNIX-like para gestion de procesos, pipes, binarios y archivos (tengo un fanatismo por
los linux y su estructura)

## shell/...: La shell
La procesadora de comandos, por ahora solo muestra un prompt y tiene funcionalidades basicas del enter
y del backspace. Aca voy a implementar un cmd.c para el procesado de comandos y busqueda de binarios en disco.

## headers/...: Cabeceras
Aca van todas las cabeceras. Y ya. Nada mas, solo interconeccion de .c's.

## Herramientas de trabajo
Por ahora uso NASM para assembly y GCC para c. Uso QEMU para emulacion porque es ligerisimo (mi pc
tiene 4gb de ram y un i5 de 4ta, sepan comprender)

## Como lo corro?
Como no se nada de flags de compiladores, la bendita IA me hizo este codigo:

```
nasm -f elf32 kernel/boot/head.asm -o head.o
/usr/bin/gcc -c kernel/kernel.c -o kernel.o -m32 -nostdlib -ffreestanding -O0 -no-pie -fno-asynchronous-unwind-tables -fno-stack-protector -I./headers
/usr/bin/gcc -c kernel/panic.c -o panic.o -m32 -nostdlib -ffreestanding -O0 -no-pie -fno-asynchronous-unwind-tables -fno-stack-protector -I./headers
/usr/bin/gcc -c kernel/drivers/atadisk.c -o atadisk.o -m32 -nostdlib -ffreestanding -O0 -no-pie -fno-asynchronous-unwind-tables -fno-stack-protector -I./headers
/usr/bin/gcc -c kernel/drivers/keyboard.c -o keyboard.o -m32 -nostdlib -ffreestanding -O0 -no-pie -fno-asynchronous-unwind-tables -fno-stack-protector -I./headers
/usr/bin/gcc -c shell/shell.c -o shell.o -m32 -nostdlib -ffreestanding -O0 -no-pie -fno-asynchronous-unwind-tables -fno-stack-protector -I./headers
/usr/bin/gcc -T linker.ld -m32 -nostdlib head.o kernel.o panic.o atadisk.o keyboard.o shell.o -o mykernel.bin -no-pie -Wl,-n -Wl,-z,max-page-size=4096
rm head.o kernel.o panic.o atadisk.o keyboard.o shell.o
qemu-system-i386 -kernel mykernel.bin
```

Copien y peguen, con eso me arranco

## Abierto a sugerencias!
Quien quiera hacer recomendaciones sobre el codigo: bienvenido sea!
Estoy abierto a propuestas de mejora y consejos si estos no van con odio

segun la ia soy alto crack por desarrollar esto con 13, subanme el ego loco
