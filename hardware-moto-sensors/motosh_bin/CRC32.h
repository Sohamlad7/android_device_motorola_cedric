#ifndef CRC_32_H
#define CRC_32_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

uint32_t calculateCrc32(uint8_t *data, size_t dataLen);
int calculateFileCrc32(char *filePath, size_t dataLen, uint8_t fill, uint32_t *outCrc);

#ifdef __cplusplus
}
#endif
#endif
