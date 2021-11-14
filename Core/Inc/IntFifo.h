#ifndef INT_FIFO_
#define INT_FIFO_

#include <stdint.h>
#include <stdbool.h>

struct IntFifo
{
	bool busy;
	uint8_t count;
	uint8_t start_index;
	uint8_t finish_index;	
	int32_t arr[4];
	
}
typedef IntFifo;

void IntFifoPush(IntFifo *queue, int32_t value);
int32_t IntFifoPop(IntFifo *queue);

#endif



