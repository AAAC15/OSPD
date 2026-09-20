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
Aca estoy desarrollando los respectivos archivos del kernel, donde tengo pensado
inspirarme en sistemas operativos UNIX-like para gestion de procesos, pipes, binarios y archivos (tengo un fanatismo por
los linux y su estructura)
El `panic.c` tiene el sistema de panico, el cual se puede inducir poniendo `panic` en el sistema.

## kernel/fs/...: File System
El sistema de archivos y estructura de carpetas y archivos!
Aca me vengo influenciando bastante de unix, sobre las carpetas como archivos, etc.
Tengo que admitir que los errores me los arreglo bastante la IA peeero bueno, ¿que se le va a hacer?

## kernel/drivers/...: Drivers
Por ahora aca solo meti los drivers de ATA PIO y del chip de teclado Intel 8042.
Es funcional, no dio problemas, asi que le tengo confianza

## shell/...: La shell
En `shell.c` esta la interfaz grafica, con lo que interactua el usuario.
En `cmd.c` esta el parser y el cerebro de los comandos, ademas del buscador de binarios en /sys/bin
Funciona guardando un buffer y pasando este al procesador de comandos, quien ejecuta las ordenes.

## headers/...: Cabeceras
Aca van todas las cabeceras. Y ya. Nada mas, solo interconexion de .c's.

## Herramientas de trabajo
Por ahora uso NASM para assembly y GCC para c. Uso QEMU para emulacion porque es ligerisimo (mi pc
tiene 4gb de ram y un i5 de 4ta, sepan comprender)

## Como lo corro?
Como no se nada de flags de compiladores, la bendita IA me hizo el Makefile.
Con poner `make clean && make` en la carpeta en la q tengan el codigo deberia arrancar
Copien y peguen, con eso me arranco

## Abierto a sugerencias!
Quien quiera hacer recomendaciones sobre el codigo: bienvenido sea!
Estoy abierto a propuestas de mejora y consejos si estos no van con odio

segun la ia soy alto crack por desarrollar esto con 13, subanme el ego loco
