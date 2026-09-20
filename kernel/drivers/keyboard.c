//driver de teclado para chip intel 8042
//recicladisimo de otro lado esto eh 

//incluimos
#include "keyboard.h"
//definimos par de alias
#define DATAPORT 0x60
#define STATUSPORT 0x64

//keymaps
//el normalito
static const char keymap[] = {
    0,  0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',  8,
    9, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 10,  0,
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',  0, '\\',
  'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',   0, 32
};
//el de shift
static const char skeymap[] = {
    0,  0, '!', '"', '$', '%', '&', '/', '(', ')', '=', '?', '-', '-',  8,
    9, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '^', '*', 10,  0,
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'N', '[', ']',  0, '}',
  'Z', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '_',   0, '*',   0, 32
};

//el shift esta siendo presionado?
static int shiftIsPressed = 0; //por defecto, no

//funcion q llama la shell para obtener ascii
char getAscii(void) {
    unsigned char code = scancodekey();
    
    //si no hay teclas presionadas, devuelve 0
    if (code == 0) return 0;
    
    if (code == 0x2A || code == 0x36) {
        shiftIsPressed = 1;
        return 0;
    }
    
    if (code == 0xAA || code == 0xB6) {
        shiftIsPressed = 0;
        return 0;
    }
    
    //ignoramos si sueltan otra cosa
    if (code & 0x80) return 0;
    
    //si hay desborde, no se hace nada
    if (code >= sizeof(keymap)) return 0;
    
    //si el shift esta siendo presionado
    if (shiftIsPressed){
        return skeymap[code];
    } else {
        return keymap[code];
    }
}

//leemos el byte que ingresa al chip (unsigned=sin numeros negativos, short=16bits)
//esta funcion es generica. le pasas cualquier puerto en port y te devuelve lo q haya en el
unsigned char inbyte(unsigned short port) {
    unsigned char result;
    __asm__ __volatile__("inb %1, %0" : "=a" (result) : "Nd" (port)); //metemos instruccion in para extraer el byte
    return result;
}

//leemos si el teclado esta siendo oprimido o no basandonos en el puerto de estado (0x64)
unsigned char keyboardpressed(){
    return (inbyte(STATUSPORT) & 0x01);
}

//leemos QUE TECLA esta siendo oprimida atraves del scancode
unsigned char scancodekey(){
    if (keyboardpressed()) {
        return inbyte(DATAPORT);
    } //si lo tocaron, leemos que valor entra en 0x60
    else {
        return 0;
    } //si no, devolvemos 0 para no hinchar
}
