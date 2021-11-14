#include "ByteFifo.h"
#include "byte_to_bits.h"

ByteFifo bytes_for_send_hart;
extern ByteFifo bit_fifo_dac;

void GetBitPacket()
{
	if(bytes_for_send_hart.count<1)return;
	if(!bit_fifo_dac.busy && bit_fifo_dac.count<=(sizeof(bit_fifo_dac.arr)-12))
	{
		uint8_t log_bit = 0;
		uint8_t count_true = 0;// number of logical "1" in byte
		uint8_t parity = 0;
		ByteFifoPush(&bit_fifo_dac, 0);// push start bit
		uint8_t byte = ByteFifoPop(&bytes_for_send_hart);// get byte from byte queue 	
		for(int i=0;i<8;i++)
		{
			log_bit = byte>>i & (00000001);
			if(log_bit==1)count_true++;
			ByteFifoPush(&bit_fifo_dac, log_bit);// divide byte into bits
		}
		parity = count_true%2==0 ? 1 : 0;// odd parity
		ByteFifoPush(&bit_fifo_dac, parity);// push parity
		ByteFifoPush(&bit_fifo_dac, 1);// push stop bit		
	}
}