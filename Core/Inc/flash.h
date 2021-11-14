#ifndef INC_FLASH_
#define INC_FLASH_

#include <stdint.h>
#include <stdbool.h>

bool FlashErase(void);
bool FlashWrite(void);
void FlashInit(void);

#endif


