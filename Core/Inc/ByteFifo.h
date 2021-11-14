#ifndef INC_BYTE_FIFO_
#define INC_BYTE_FIFO_

#include <stdint.h>
#include <stdbool.h>

struct ByteFifo
{
	bool busy;
	uint16_t count;
	uint16_t start_index;
	uint16_t finish_index;
	uint8_t avialable_byte;
	uint8_t arr[256];
	
}
typedef ByteFifo;
void GetBitPacket(void);
void ByteFifoPush(ByteFifo *queue, uint8_t value);
bool ByteFifoByteAvialable(ByteFifo *queue);
uint8_t ByteFifoPop(ByteFifo *queue);
void ByteFifoClear(ByteFifo* fifo);

#endif

