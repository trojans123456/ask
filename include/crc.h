#ifndef __CRC_H
#define __CRC_H

#include "default.h"
#include <stdint.h>
C_API_BEGIN

uint8_t crc8(uint8_t *input,int len);

uint16_t crc16(uint8_t *input,int len);

uint32_t crc32(uint8_t *input,int len);

C_API_END

#endif
