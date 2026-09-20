/* headers/keyboard.h */
#ifndef KEYBOARD_H
#define KEYBOARD_H

unsigned char inbyte(unsigned short port);
unsigned char keyboardpressed();
unsigned char scancodekey();

char getAscii(void);

#endif
