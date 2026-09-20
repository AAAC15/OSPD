/* shell/shell.c */
#include "shell.h"
#include "keyboard.h" 

// activamos la declaracion externa del modulo parser
extern void cmd(char* buffer, int* actualoffset, int* currentSector, char* currentPath);

typedef unsigned short uint16_t; // 2b

/* puntero base a la memoria fisica vga y atributos de color por defecto */
#define VIDEOPOINTER 0xB8000
#define BLACKNCYAN   0x0B
#define BLACKNWHITE  0x0F

/* prompt del sistema operativo y ruta inicial por defecto en formato crudo */
static char currentPath[256] = {'/', 0};

// funcion auxiliar local para dibujar el prompt compuesto dinamico
static int drawDynamicPrompt(char* videomem, int offset) {
    int idx = offset;
    
    char base[] = {'O', 'S', 'P', 'D', 0};
    int i = 0;
    while (base[i] != 0) {
        videomem[idx] = base[i];
        videomem[idx + 1] = BLACKNCYAN;
        i++;
        idx += 2;
    }
    
    i = 0;
    while (currentPath[i] != 0) {
        videomem[idx] = currentPath[i];
        videomem[idx + 1] = BLACKNCYAN;
        i++;
        idx += 2;
    }
    
    char close[] = {'>', ' ', 0};
    i = 0;
    while (close[i] != 0) {
        videomem[idx] = close[i];
        videomem[idx + 1] = BLACKNCYAN;
        i++;
        idx += 2;
    }
    
    return idx;
}

/* actualiza la posicion del cursor de hardware de la vga mediante puertos crudos crt */
static void vgacursorfollowing(int cursorpos) {
    unsigned short position = (unsigned short)(cursorpos / 2);
    
    uint16_t port3d4 = 0x3D4;
    uint16_t port3d5 = 0x3D5;
    unsigned char reg0f = 0x0F;
    unsigned char reg0e = 0x0E;
    unsigned char posLow = (unsigned char)(position & 0xFF);
    unsigned char posHigh = (unsigned char)((position >> 8) & 0xFF);

    __asm__ __volatile__("outb %0, %1" : : "a"(reg0f), "Nd"(port3d4));
    __asm__ __volatile__("outb %0, %1" : : "a"(posLow), "Nd"(port3d5));
    __asm__ __volatile__("outb %0, %1" : : "a"(reg0e), "Nd"(port3d4));
    __asm__ __volatile__("outb %0, %1" : : "a"(posHigh), "Nd"(port3d5));
}

void shell_main(void) {
    char* videomem = (char*) VIDEOPOINTER;
    
    /* offset inicial de 640 bytes equivalente a arrancar en la linea 5 exacta */
    int offset = 640; 
    int currentSector = 60; 

    /* bucle para dibujar el prompt inicial */
    int cursorpos = drawDynamicPrompt(videomem, offset);
    int promptLengthBytes = cursorpos - offset;
    vgacursorfollowing(cursorpos);
    
    /* inicializacion del buffer local para la captura de texto */
    char cmdbuffer[64];
    int bufidx = 0;
    
    while(1) {
        /* llamamos al driver pa procesar la tecla */
        char key = getAscii();
        
        /* si no hay pulsaciones del hardware volvemos a evaluar el ciclo */
        if (key == 0) {
            continue;
        }
        
        if (key != 0) { 
            
            /* caso 1: manejo del backspace para borrar caracteres de la pantalla */
            if (key == 8) {
                if (cursorpos > (offset + promptLengthBytes)) {
                    cursorpos -= 2; 
                    vgacursorfollowing(cursorpos);
                    videomem[cursorpos] = 0x20;    
                    videomem[cursorpos + 1] = BLACKNWHITE; 
                }
                if (bufidx > 0) {
                    bufidx--; 
                }
            } 
            
            /* caso 2: manejo del enter para mandar a procesar el comando acumulado */
            else if (key == 10) {
                cmdbuffer[bufidx] = 0;   
                
                /* ¡ACTIVAMOS LA LLAMADA AL PARSER DE COMANDOS! */
                cmd(cmdbuffer, &offset, &currentSector, currentPath); 
                
                bufidx = 0;              
                
                /* rutina de scroll vertical en caso de llegar al limite de la vga (3840 bytes) */
                if (offset >= 3840) {
                    for (int pos = 0; pos < 3840; pos++) {
                        videomem[pos] = videomem[pos + 160]; 
                    }
                    for (int pos = 3840; pos < 4000; pos += 2) {
                        videomem[pos] = ' ';
                        videomem[pos + 1] = BLACKNWHITE;
                    }
                    offset = 3840;
                }
                
                /* redibujamos el prompt del sistema al inicio de la nueva linea */
                cursorpos = drawDynamicPrompt(videomem, offset);
                promptLengthBytes = cursorpos - offset;
                vgacursorfollowing(cursorpos);
            }
            
            /* caso 3: captura e impresion en vivo de caracteres comunes imprimibles */
            else {
                if (bufidx < 63) {
                    videomem[cursorpos] = key;       
                    videomem[cursorpos + 1] = BLACKNWHITE; 
                    cursorpos += 2;
                    vgacursorfollowing(cursorpos);
                    cmdbuffer[bufidx] = key;         
                    bufidx++;                        
                }
            }
        }
    } 
    
    /* porr las dudas si pasa algo con la cpu metemos halt */
    while(1) {
        __asm__ __volatile__("hlt");
    }
}
