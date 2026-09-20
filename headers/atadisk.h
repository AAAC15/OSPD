/* headers/atadisk.h */
#ifndef ATADISK_H
#define ATADISK_H

typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;

void ataStart(void);

void ataReadSector(uint32_t lba, uint8_t* bufer);

void ataWriteSector(uint32_t lba, const uint8_t* bufer);

#endif
