#include "main.h"
#include "fsk.h"
#include "ByteFifo.h"

// Функция помещения байта в очередь
void ByteFifoPush(ByteFifo *queue, uint8_t value) //
{
	if(queue->count<sizeof(queue->arr))
	{		
		queue->arr[queue->finish_index]=value;
		queue->finish_index++;
		if(queue->finish_index>(sizeof(queue->arr)-1))
		{
			queue->finish_index=0;
		}
		queue->count++;		
	}	
}
// Функция изьятия байта из очереди
uint8_t ByteFifoPop(ByteFifo *queue)
{
	uint8_t result = 0;
	queue->busy = true;
	if(queue->count>0)
	{
		result = queue->arr[queue->start_index];
		queue->start_index++;
		if(queue->start_index>(sizeof(queue->arr)-1))
		{
			queue->start_index=0;
		}
		queue->count--;
	}
	queue->busy = false;
	return result;
}

// Функция проверки доступности байта в битовом потоке
bool ByteFifoByteAvialable(ByteFifo *queue)
{
		uint8_t temp[11]={0};
		uint16_t i = 0;
		uint8_t count_true = 0;
	  uint8_t start_index = queue->start_index;		
		queue->avialable_byte = 0;
		if(queue->count<11)return false;
		if(queue->arr[start_index]!=0)return false;// check start bit == false
		
		if(start_index + 11 <= sizeof(queue->arr))
		{
			for(i=start_index;i<start_index+11;i++)
			{
				temp[i-start_index] = queue->arr[i];
			}
		}
		else
		{
			for(i=start_index;i<=sizeof(queue->arr)-1;i++)
			{
				temp[i-start_index] = queue->arr[i];
			}
			for(i=0;i<=start_index + 10 - sizeof(queue->arr);i++)
			{
				temp[sizeof(queue->arr)-start_index+i] = queue->arr[i];
			}
			
		}
		  if(temp[10]!=1)return false;//check stop bit == true
			for(i=1;i<9;i++)
			{
				if(temp[i]==1)count_true++;				
			}
			if((count_true+temp[9])%2==1)// if patity == odd
			{
					for(i=1;i<9;i++)
					{
						queue->avialable_byte|=temp[i]<<(i-1);			
					}
					return true;
			}
			else return false;		
}
// Очистить очередь
 void ByteFifoClear(ByteFifo* fifo) 
 {
	 fifo->count=0;
	 fifo->start_index=0;
	 fifo->finish_index=0;
	 fifo->busy = false;	 
 }

