/* kernel/panic.c */
#include "panic.h"

#define VIDEO_POINTER 0xB8000
#define RED_N_WHITE   0x4F

void sumthingWentWrong(char* errorMessage) {
    __asm__ __volatile__("cli" : : : "memory");
    char* videoMem = (char*) VIDEO_POINTER;

    /* limpiamos toda la pantalla vga con fondo rojo */
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        videoMem[i] = ' ';
        videoMem[i + 1] = RED_N_WHITE;
    }
    
    char* banner = "DODO DYING! X.X - Something went wrong! Restart";
    int offset = 160;
    int j = 0;

    while (banner[j] != 0) {
        videoMem[offset + (j * 2)] = banner[j];
        videoMem[offset + (j * 2) + 1] = RED_N_WHITE;
        j++;
    }
    
    int msgOffset = 320;
    int k = 0;
    while (errorMessage[k] != 0) {
        videoMem[msgOffset + (k * 2)] = errorMessage[k];
        videoMem[msgOffset + (k * 2) + 1] = RED_N_WHITE;
        k++;
    }

    /* congelamiento de resguardo */
    while(1) {}
}
