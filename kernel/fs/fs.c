/* kernel/fs/fs.c */
#include "fs.h"
#include "atadisk.h"

// molde unificado empacado al byte para evitar padding rancio de gcc
struct dirEntry {
    char name[MAX_NAME_LEN];
    unsigned char is_directory;
    unsigned char permissions;
    unsigned int startsector;
    unsigned int sizebytes;
} __attribute__((packed));

// reservamos un bloque de 512bytes en bss para el arranque seguro
static uint16_t fsRootBuffer[256] = {0};

// reservamos otro bloque para operaciones dinamicas de lectura y escritura
static uint16_t fsOperBuffer[256] = {0};

// utilidad local segura para copiar cadenas sin desbordar el vector
static void strCopy(char* dest, const char* src, int maxLen) {
    int i = 0;
    while(src[i] != 0 && i < (maxLen - 1)) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = 0;
}

// utilidad local para comparar nombres de archivos byte a byte
static int strCompare(const char* str1, const char* str2) {
    int i = 0;
    while(str1[i] != 0 || str2[i] != 0) {
        if (str1[i] != str2[i]) return 0;
        i++;
    }
    return 1;
}

// escribe la estructura inicial de directorios en los sectores del disco
void fsFormat(void) {
    struct dirEntry* dir = (struct dirEntry*)fsOperBuffer;
    char* rawPtr = (char*)fsOperBuffer;
    
    // root, me gusta decirle ospdisk
    for(int i = 0; i < 512; i++) rawPtr[i] = 0;
    
    strCopy(dir[0].name, "sys", MAX_NAME_LEN);
    dir[0].is_directory = 1;
    dir[0].permissions = 0; 
    dir[0].startsector = 61; 
    dir[0].sizebytes = 512;
    
    strCopy(dir[1].name, "usr", MAX_NAME_LEN);
    dir[1].is_directory = 1;
    dir[1].permissions = 1; 
    dir[1].startsector = 62; 
    dir[1].sizebytes = 512;
    ataWriteSector(60, (uint8_t*)fsOperBuffer);

    // que contiene sys
    for(int i = 0; i < 512; i++) rawPtr[i] = 0;
    
    // bin (binarios): programitas y binarios
    strCopy(dir[0].name, "bin", MAX_NAME_LEN);
    dir[0].is_directory = 1;
    dir[0].permissions = 0;
    dir[0].startsector = 63; 
    dir[0].sizebytes = 512;
    
    // esen (esenciales): kernel y shell basicamente
    strCopy(dir[1].name, "esen", MAX_NAME_LEN);
    dir[1].is_directory = 1;
    dir[1].permissions = 0;
    dir[1].startsector = 64; 
    dir[1].sizebytes = 512;
    
    // infusr (info de usuarios): basicamente el dni de los usuarios
    strCopy(dir[2].name, "infusr", MAX_NAME_LEN);
    dir[2].is_directory = 0; 
    dir[2].permissions = 1; 
    dir[2].startsector = 65; 
    dir[2].sizebytes = 512;
    ataWriteSector(61, (uint8_t*)fsOperBuffer);
    
    // que contiene bin (el programita mu)
    for(int i = 0; i < 512; i++) rawPtr[i] = 0;
    strCopy(dir[0].name, "mu", MAX_NAME_LEN);
    dir[0].is_directory = 0; 
    dir[0].permissions = 0;   
    dir[0].startsector = 67;  
    dir[0].sizebytes = 512;
    ataWriteSector(63, (uint8_t*)fsOperBuffer); 
    
    // que contiene usr (la carpeta de usuario invitado solamente sin mucha ciencia)
    for(int i = 0; i < 512; i++) rawPtr[i] = 0;
    strCopy(dir[0].name, "invitado", MAX_NAME_LEN);
    dir[0].is_directory = 1;
    dir[0].permissions = 1;
    dir[0].startsector = 66; 
    dir[0].sizebytes = 512;
    ataWriteSector(62, (uint8_t*)fsOperBuffer);
    
    // blanqueamos los sectores de datos restantes por seguridad
    for(int i = 0; i < 512; i++) rawPtr[i] = 0;
    ataWriteSector(64, (uint8_t*)fsOperBuffer); 
    ataWriteSector(65, (uint8_t*)fsOperBuffer); 
    ataWriteSector(66, (uint8_t*)fsOperBuffer); 
    ataWriteSector(67, (uint8_t*)fsOperBuffer); 
}

// inicializa y monta el sistema de archivos de ospd
void fsInit(void) {
    ataReadSector(60, (uint8_t*)fsRootBuffer);
    struct dirEntry* rootDir = (struct dirEntry*)fsRootBuffer;
    
    // si nota que el sector raiz esta vacio o corrupto metemos formateo en vivo
    if (rootDir[0].startsector != 61) {
        fsFormat();
    }
}

// busca una entrada especifica adentro de un sector directorio
int fsSearchEntry(unsigned int parentSector, const char* name, unsigned int* foundSector, unsigned char* isDirectory) {
    ataReadSector(parentSector, (uint8_t*)fsOperBuffer);
    struct dirEntry* dir = (struct dirEntry*)fsOperBuffer;
    
    // recorremos las 32 entradas posibles que entran en un sector de 512 bytes
    for (int i = 0; i < 32; i++) {
        if (dir[i].startsector != 0 && dir[i].name[0] != 0) {
            if (strCompare(name, dir[i].name)) {
                *foundSector = dir[i].startsector;
                *isDirectory = dir[i].is_directory;
                return 1;
            }
        }
    }
    return 0;
}

// inserta una nueva entrada en el casillero vacio del disco (los directorios SON archivos)
int fsCreateEntry(unsigned int parentSector, const char* name, unsigned char isDirectory, unsigned int* assignedSector) {
    ataReadSector(parentSector, (uint8_t*)fsOperBuffer);
    struct dirEntry* dir = (struct dirEntry*)fsOperBuffer;
    
    int freeSlot = -1;
    unsigned int maxSector = 67; // resguardo para proteger la estructura fija inicial
    
    // buscamos un hueco libre y calculamos el proximo bloque disponible
    for (int i = 0; i < 32; i++) {
        if (dir[i].startsector == 0 && freeSlot == -1) {
            freeSlot = i;
        }
        if (dir[i].startsector > maxSector) {
            maxSector = dir[i].startsector;
        }
    }
    
    if (freeSlot == -1) return 0;
    
    unsigned int newSector = maxSector + 1;
    
    strCopy(dir[freeSlot].name, name, MAX_NAME_LEN);
    dir[freeSlot].is_directory = isDirectory;
    dir[freeSlot].permissions = 1;
    dir[freeSlot].startsector = newSector;
    dir[freeSlot].sizebytes = 512;
    
    // impactamos los cambios fisicamente en el directorio padre
    ataWriteSector(parentSector, (uint8_t*)dir);
    
    // si la nueva entrada es un directorio blanqueamos su sector interno en el disco
    uint16_t emptyBuffer[256] = {0};
    ataWriteSector(newSector, (uint8_t*)emptyBuffer);
    
    *assignedSector = newSector;
    return 1;
}

// borra un casillero de la lista del disco inyectando ceros
int fsDeleteEntry(unsigned int parentSector, const char* name) {
    ataReadSector(parentSector, (uint8_t*)fsOperBuffer);
    struct dirEntry* dir = (struct dirEntry*)fsOperBuffer;
    
    for (int i = 0; i < 32; i++) {
        if (dir[i].startsector != 0 && strCompare(name, dir[i].name)) {
            // liberamos el casillero metiendo ceros crudos
            dir[i].name[0] = 0;
            dir[i].startsector = 0;
            dir[i].is_directory = 0;
            
            ataWriteSector(parentSector, (uint8_t*)dir);
            return 1;
        }
    }
    return 0;
}

// lee el sector del disco y lista todo lo que tiene adentro en la pantalla
void fsListDirectory(unsigned int currentSector, char* videomem, int* actualoffset) {
    ataReadSector(currentSector, (uint8_t*)fsOperBuffer);
    struct dirEntry* dir = (struct dirEntry*)fsOperBuffer;
    
    int foundAny = 0;
    
    // escaneamos los 32 casilleros posibles del bloque de 512 bytes
    for (int i = 0; i < 19; i++) {
        if (dir[i].startsector != 0 && dir[i].name[0] != 0 && dir[i].name[0] >= 32 && dir[i].name[0] < 127) {
            foundAny = 1;
            int j = 0;
            
            // si es un directorio le metemos verde brillante (0x0A) y si es archivo blanco (0x0F)
            unsigned char color = dir[i].is_directory ? 0x0A : 0x0F;
            
            // volcamos el nombre del archivo en la linea de la vga
            while (dir[i].name[j] != 0 && j < MAX_NAME_LEN) {
                videomem[*actualoffset + (j * 2)] = dir[i].name[j];
                videomem[*actualoffset + (j * 2) + 1] = color;
                j++;
            }
            *actualoffset += 160;
        }
    }
    
    // si la carpeta estaba totalmente vacia o limpia abrimos linea
    if (foundAny == 0) {
        *actualoffset += 160;
    }
}
