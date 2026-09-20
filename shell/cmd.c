/* shell/cmd.c */ 
// procesador de comandos

// incluimos
#include "atadisk.h"
#include "fs.h"
#include "panic.h"

// par de aliases
#define VIDEOPOINTER 0XB8000
#define BLACKNWHITE 0X0F
#define REDNWHITE 0X4F

// molde pa interpretar sectores del disco
struct dirEntry {
    char name[MAX_NAME_LEN];
    unsigned char is_directory;
    unsigned char permissions;
    unsigned int startsector;
    unsigned int sizebytes;
} __attribute__((packed));

// reservamos un bloque de 512bytes (un sector)
static uint16_t binDirBuffer[256] = {0};
// reservamos vector de 16bytes para palabra
static char cmdName[16] = {0};

// error de comando desconocido 
static const char msgError[] = {'C', 'o', 'm', 'a', 'n', 'd', 'o', ' ', 'd', 'e', 's', 'c', 'o', 'n', 'o', 'c', 'i', 'd', 'o', '.', 0};

//el parser, osea el coso q procesa los comandos
int cmdparser(char* str1, char* str2){
    int i = 0;
    while(str1[i] != 0 || str2[i] != 0) {
        if (str1[i] != str2[i]) {
            return 0;
        }
        i++;
    }
    return 1;
}

//obtiene la primera palabra del comando
static void getCmdName(char* buffer, char* dest) {
    int i = 0;
    while (buffer[i] != 0 && buffer[i] != ' ' && i < 15){
        dest[i] = buffer[i];
        i++;
    }
    dest[i] = 0;
}

//obtiene el argumento del comando
static void getCmdArg(char* buffer, char* dest) {
    int i = 0;
    while (buffer[i] != 0 && buffer[i] != ' ') i++;
    if (buffer[i] == ' ') i++;
    int j = 0;
    while (buffer[i] != 0 && buffer[i] != ' ' && j < 15) {
        dest[j] = buffer[i];
        i++;
        j++;
    }
    dest[j] = 0;
}

//el cerebro ahora si, el decididor
void cmd(char* buffer, int* actualoffset, int* currentSector, char* currentPath){
    char* videomem = (char*) VIDEOPOINTER;
    *actualoffset += 160;
    
    // lp (limpiar pantalla)
    if (cmdparser(buffer, "lp")){
        for (int i = 0; i < 4000; i += 2){
            videomem[i] = ' ';
            videomem[i + 1] = BLACKNWHITE;
        }
        *actualoffset = 0;
        return;
    }
    //imp (imprimir)
    else if (buffer[0] == 'i' && buffer[1] == 'm' && buffer[2] == 'p' && buffer[3] == ' '){
        int i = 4;
        while (buffer[i] != 0 && buffer[i] != '"'){
            i++;
        }
        if (buffer[i] == '"') {
            i++; 
            int printidx = 0; 
            while (buffer[i] != 0 && buffer[i] != '"') {
                videomem[*actualoffset + (printidx * 2)] = buffer[i];
                videomem[*actualoffset + (printidx * 2) + 1] = BLACKNWHITE;
                printidx++;
                i++;
            }
        }
        *actualoffset += 160;
        return;
    }
    // panic (autodestruccion y panico inducido por el usuario sin romper nada)
    else if (cmdparser(buffer, "panic")) {
        sumthingWentWrong("dont worry! panic is induced, nothing is damaged");
        return;
    }
    // idt (imprimir directorio de trabajo con la ruta real y dinamica)
    else if (cmdparser(buffer, "idt")) {
        int i = 0;
        while (currentPath[i] != 0 && (i * 2) < 160) {
            videomem[*actualoffset + (i * 2)] = currentPath[i];
            videomem[*actualoffset + (i * 2) + 1] = BLACKNWHITE;
            i++;
        }
        *actualoffset += 160;
        return;
    }
    // ls (listar)
    else if (cmdparser(buffer, "ls")) {
        fsListDirectory(*currentSector, videomem, actualoffset);
        return;
    }
    // ir (de ir, asi nomas - cambia de directorio)
    else if (buffer[0] == 'i' && buffer[1] == 'r' && buffer[2] == ' ') {
        char arg[16];
        getCmdArg(buffer, arg);
        unsigned int foundSector;
        unsigned char isDirectory;
        
        if (arg[0] == '.' && arg[1] == '.') {
            *currentSector = 60;
            currentPath[0] = '/';
            currentPath[1] = 0;
            return;
        }
        
        if (fsSearchEntry(*currentSector, arg, &foundSector, &isDirectory)) {
            if (isDirectory) {
                *currentSector = foundSector;
                int p = 0;
                while (currentPath[p] != 0) p++;
                if (p > 1) {
                    currentPath[p++] = '/';
                }
                int a = 0;
                while (arg[a] != 0) currentPath[p++] = arg[a++];
                currentPath[p] = 0;
            }
        }
        return;
    }
    // nd (nuevo directorio - ¿o archivo?)
    else if (buffer[0] == 'n' && buffer[1] == 'd' && buffer[2] == ' ') {
        char arg[16];
        getCmdArg(buffer, arg);
        unsigned int assignedSector;
        fsCreateEntry(*currentSector, arg, 1, &assignedSector);
        return;
    }
    // na (nuevo archivo)
    else if (buffer[0] == 'n' && buffer[1] == 'a' && buffer[2] == ' ') {
        char arg[16];
        getCmdArg(buffer, arg);
        unsigned int assignedSector;
        fsCreateEntry(*currentSector, arg, 0, &assignedSector);
        return;
    }
    // brr (borrar directorios o archivos de la lista)
    else if (buffer[0] == 'b' && buffer[1] == 'r' && buffer[2] == 'r' && buffer[3] == ' ') {
        char arg[16];
        getCmdArg(buffer, arg);
        fsDeleteEntry(*currentSector, arg);
        return;
    }
    // cargador de binarios de sys/bin
    else {
        getCmdName(buffer, cmdName);
        
        if (cmdName[0] == 0) {
            return;
        }

        int found = 0;
        int targetSector = 0;

        if (cmdName[0] != 0) {
            /* invocamos la lectura modular del sector de directorios 63 */
            ataReadSector(63, (uint8_t*)binDirBuffer);
            struct dirEntry* dir = (struct dirEntry*)binDirBuffer;

            for (int idx = 0; idx < 18; idx++) {
                if (dir[idx].startsector != 0 && dir[idx].name[0] != 0) {
                    if (cmdparser(cmdName, dir[idx].name)) {
                        found = 1;
                        targetSector = dir[idx].startsector;
                        break;
                    }
                }
            }
        }

        // si existe el archivo en el disco lo cargamos a la ram y saltamos
        if (found == 1) {
            uint16_t* ramExecAddress = (uint16_t*)0x90000;
            /* leemos el sector del ejecutable y lo mandamos a la ram */
            ataReadSector(targetSector, (uint8_t*)ramExecAddress);
            
            void (*runProgram)() = (void (*)())0x90000;
            runProgram();
            return;
        }
        
        int i = 0;
        while (msgError[i] != 0 && (i * 2) < 160) {
            videomem[*actualoffset + (i * 2)] = msgError[i];
            videomem[*actualoffset + (i * 2) + 1] = REDNWHITE;
            i++;
        }
        *actualoffset += 160;
    }
}
