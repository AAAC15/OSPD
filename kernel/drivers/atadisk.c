//drivers de discos ata

//incluimos
#include "atadisk.h"

//lectura de disco con ensamblador
//uin8 = 1 byte, uin16 = 2 bytes, uin32 = 4 bytes
//vamos a decirle word o palabra a los 2 bytes
//usamos word para inyectar o leer datos a maxima velocidad
//mientras q byte para leer estados de forma rapida
//leemos 2 bytes desde el puerto correspondiente (c no tiene opcion para
//leer puertos, usamos in y out en assembly)
static inline uint16_t inword(uint16_t port) {
    uint16_t result;
    __asm__ __volatile__("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

//escribimos 2 bytes desde el puerto correspondiente
static inline void outword(uint16_t port, uint16_t data) {
    __asm__ __volatile__("outw %0, %1" : : "a"(data), "Nd"(port));
}

//leemos un byte para revisar el estado del disco en el puerto correspondiente (0x1f7)
static inline uint8_t inbyte(uint16_t port) {
    uint8_t result;
    __asm__ __volatile__("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

//escribimos un byte para comandos de lectura o escritura
static inline void outbyte(uint16_t port, uint8_t data) {
    __asm__ __volatile__("outb %0, %1" : : "a"(data), "Nd"(port));
}

//funciones de lectura y escritura
//lectura:
void ataReadSector(uint32_t lba, uint8_t* bufer) {
    //chusmeamos si el disco esta ocupado
    //mientras se NO se cumplan estos parametros, no seguimos
    while ((inbyte(0x1F7) & 0X80) != 0);
    
    //avisamos q queremos leer un sector
    outbyte(0x1F2, 1);
    
    //mandamos cordenada lba del sector repartida en puertos
    outbyte(0x1F3, (uint8_t)lba); //bits 0-7
    outbyte(0x1F4, (uint8_t)(lba >> 8)); //8 a 15
    outbyte(0x1F5, (uint8_t)(lba >> 16)); //16 a 23
    //seleccionamos master drive mas los bits restados del lba
    outbyte(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    
    //mandamos lectura en modo pollito (pio)
    outbyte(0x1F7, 0x20);
    
    //si NO tiene los datos listos no sigue
    while ((inbyte(0x1F7) & 0x08) == 0);
    
    //nos llevamos los 512 bytes (2*256)
    uint16_t* targetbuffer = (uint16_t*)bufer;
    for (int idx = 0; idx < 256; idx++) {
        targetbuffer[idx] = inword(0x1F0);
    }
}

//escritura:
void ataWriteSector(uint32_t lba, const uint8_t* bufer) {
    //chusmeamos si el disco esta ocupado
    while ((inbyte(0x1F7) & 0X80) != 0);
    
    //avisamos q queremos escribir un sector
    outbyte(0x1F2, 1);
    
    //mandamos cordenada lba del sector repartida en puertos
    outbyte(0x1F3, (uint8_t)lba); //bits 0-7
    outbyte(0x1F4, (uint8_t)(lba >> 8)); //8 a 15
    outbyte(0x1F5, (uint8_t)(lba >> 16)); //16 a 23
    //seleccionamos master drive mas los bits restados del lba
    outbyte(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    
    // mandamos comando de escritura en modo en la radio hay un pollito (pio)
    outbyte(0x1F7, 0x30);
    
    // si NO esta preparado para recibir datos no sigue
    while ((inbyte(0x1F7) & 0x08) == 0);
    
    //empujamos los 512 bytes
    uint16_t* sourcebuffer = (uint16_t*)bufer;
    for (int idx = 0; idx < 256; idx++) {
        outword(0x1F0, sourcebuffer[idx]);
    }
}

void ataStart(void) {
    inbyte(0x1F7);
}
