#include "IntFifo.h"

void IntFifoPush(IntFifo *queue, int32_t value)
{
	if(queue->count<4)
	{		
		queue->arr[queue->finish_index]=value;
		queue->finish_index++;
		if(queue->finish_index>3)
		{
			queue->finish_index=0;
		}
		queue->count++;		
	}	
}

int32_t IntFifoPop(IntFifo *queue)
{
	int32_t result = 0;
	queue->busy = true;
	if(queue->count>0)
	{
		result = queue->arr[queue->start_index];
		queue->start_index++;
		if(queue->start_index>3)
		{
			queue->start_index=0;
		}
		queue->count--;
	}
	queue->busy = false;
	return result;
}

