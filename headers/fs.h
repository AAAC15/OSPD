/* headers/fs.h */
#ifndef FS_H
#define FS_H

#define MAX_NAME_LEN 16

void fsInit(void);
void fsFormat(void);

int fsSearchEntry(unsigned int parentSector, const char* name, unsigned int* foundSector, unsigned char* isDirectory);
int fsCreateEntry(unsigned int parentSector, const char* name, unsigned char isDirectory, unsigned int* assignedSector);
int fsDeleteEntry(unsigned int parentSector, const char* name);

void fsListDirectory(unsigned int currentSector, char* videomem, int* actualoffset);

#endif
