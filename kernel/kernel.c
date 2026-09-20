
/* kernel/kernel.c */
// kernel de ospd (el .c principal, son varios archivos el kernel)

// incluimos las cabeceras desde headers/
#include "atadisk.h"
#include "keyboard.h"
#include "fs.h"
#include "shell.h"
#include "panic.h" 

// tipos de datos 
typedef unsigned int   uint32_t; // 4b
typedef unsigned short uint16_t; // 2b

// numero magico para grub
#define MULTIBOOT_MAGIC 0X1BADB002

// main
void kernelMain(uint32_t magic, uint32_t multibootInfoPtr){
    /*if (magic != MULTIBOOT_MAGIC) {
        // si grub no metio el numero magico entramos en panico total
        sumthingWentWrong("ERROR: Multiboot magic number doesn't match.");
        return;
    }*/
    
    uint16_t* videoMemory = (uint16_t*) 0xb8000;
    
    // indice 160 = inicio de la linea 3 de la pantalla (80 columnas * 2 lineas previas)
    int i = 160;
    
    // log de inicializacion en mayusculas en la tercera linea
    // 0x07 es gris estandar, 0x0a es verde brillante
    videoMemory[i++] = '[' | (0x07 << 8);
    videoMemory[i++] = 'K' | (0x0a << 8);
    videoMemory[i++] = 'R' | (0x0a << 8);
    videoMemory[i++] = 'N' | (0x0a << 8);
    videoMemory[i++] = 'L' | (0x0a << 8);
    videoMemory[i++] = 'O' | (0x0a << 8);
    videoMemory[i++] = 'K' | (0x0a << 8);
    videoMemory[i++] = ']' | (0x07 << 8);

    // inicializamos el hardware del disco duro
    ataStart();
    
    // montamos el disco con tus carpetas del dodo
    fsInit();

    // levantamos el front end de la shell interactiva
    shell_main();
}
