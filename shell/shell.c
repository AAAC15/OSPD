/* shell/shell.c */
#include "shell.h"
#include "keyboard.h" 

/* declaracion externa de la funcion encargada de procesar el buffer de comandos */
/* extern void cmd(const char* bufer, int* offset, int* sector, char* path); */

typedef unsigned short uint16_t; // 2B

/* puntero base a la memoria fisica vga y atributos de color por defecto */
#define VIDEOPOINTER 0xB8000
#define BLACKNCYAN   0x0B
#define BLACKNWHITE  0x0F

/* prompt del sistema operativo y ruta inicial por defecto en formato crudo */
static const char prompt[] = {'O', 'S', 'P', 'D', '>', ' ', 0};
static char current_path[] = {'/', 'u', 's', 'r', '/', 'i', 'n', 'v', 'i', 't', 'a', 'd', 'o', 0};

/* actualiza la posicion del cursor de hardware de la vga mediante puertos crudos crt */
static void vgacursorfollowing(int cursorpos) {
    unsigned short position = (unsigned short)(cursorpos / 2);
    
    /* creamos variables limpias para los puertos para no confundir a gcc */
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
    int current_sector = 56;

    /* bucle para dibujar el prompt inicial con el esquema de color seleccionado */
    int start_idx = 0;
    while (prompt[start_idx] != 0) {
        videomem[offset + (start_idx * 2)] = prompt[start_idx];
        videomem[offset + (start_idx * 2) + 1] = BLACKNCYAN;
        start_idx++;
    }

    /* calculo de la posicion inicial del cursor sumando el espacio del prompt */
    int cursorpos = offset + 12;
    vgacursorfollowing(cursorpos);
    
    /* inicializacion del buffer local para la captura de texto */
    char cmdbuffer[64];
    int bufidx = 0;
    
    while(1) {
        /* invocamos al driver del teclado para obtener el caracter ascii ya procesado */
        char key = getAscii();
        
        /* si no hay pulsaciones del hardware volvemos a evaluar el ciclo */
        if (key == 0) {
            continue;
        }
        
        if (key != 0) { 
            
            /* caso 1: manejo del backspace para borrar caracteres de la pantalla */
            if (key == 8) {
                if (cursorpos > (offset + 12)) {
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
                
                /* despachamos el comando al modulo del parser central */
                /* cmd(cmdbuffer, &offset, &current_sector, current_path); */
                bufidx = 0;              
                
                offset += 160;
                
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
                int prompt_idx = 0;
                while (prompt[prompt_idx] != 0) {
                    videomem[offset + (prompt_idx * 2)] = prompt[prompt_idx];
                    videomem[offset + (prompt_idx * 2) + 1] = BLACKNCYAN;
                    prompt_idx++;
                }
                
                cursorpos = offset + 12;
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
    
    /* lazo de seguridad final para contener la cpu ante un fallo critico de salida */
    while(1) {
        __asm__ __volatile__("hlt");
    }
}
