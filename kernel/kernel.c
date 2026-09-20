/* kernel/kernel.c */

// incluimos
#include "atadisk.h"
#include "shell.h"
#include "panic.h"

//xtipos de datos
typedef unsigned int   uint32_t; // 4B
typedef unsigned short uint16_t; // 2B


//main del kernel
void kernelMain(void) {
    uint16_t* videoMemory = (uint16_t*) 0xb8000;
    
    /* confirmacion */
    int j = 160;
    videoMemory[j++] = '[' | (0x07 << 8);
    videoMemory[j++] = 'K' | (0x0a << 8);
    videoMemory[j++] = 'R' | (0x0a << 8);
    videoMemory[j++] = 'N' | (0x0a << 8);
    videoMemory[j++] = 'L' | (0x0a << 8);
    videoMemory[j++] = 'O' | (0x0a << 8);
    videoMemory[j++] = 'K' | (0x0a << 8);
    videoMemory[j++] = ']' | (0x07 << 8);

    ataStart();
    //arrancamos shell
    shell_main();
}
