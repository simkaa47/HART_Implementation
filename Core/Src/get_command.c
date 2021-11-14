#include "main.h"
#include "ByteFifo.h"
#include "HartPacket.h"
#include "set_command.h"
#include "flash.h"

#define TIMEOUT_WAIT_RECEIVE       				50 // ms

uint8_t hart_address;
uint8_t time_without_receive;
uint8_t tlg_cnt;
HartPacket in_hart_packet;
extern ByteFifo rcv_hart_bytes;// recived bytes
extern HartPacket out_hart_packet;
//function prototips
uint8_t CheckSumInHart(void);
void InTlgReaction(void);
void CmdChangeAddrExecute(void);
//end of function prototips

// functions
void RecognizeCommand()
{	
	if(time_without_receive<TIMEOUT_WAIT_RECEIVE)
	{		
		return;//wait receive pause
	}
	if(rcv_hart_bytes.count==0)return;
	if(rcv_hart_bytes.count<8)goto packet_invalid; // too small packet	
	if(rcv_hart_bytes.count == 256)goto packet_invalid;// overflow packet
	if(ByteFifoPop(&rcv_hart_bytes)!=0xFF)goto packet_invalid;//preamble not found
	while((in_hart_packet.delimeter = ByteFifoPop(&rcv_hart_bytes))==0xFF)
	{
		if(rcv_hart_bytes.count<5)goto packet_invalid;//too few bytes left
	}
	if(in_hart_packet.delimeter!=0x06)goto packet_invalid;// wrong delimeter
	in_hart_packet.addr = ByteFifoPop(&rcv_hart_bytes);
	if(in_hart_packet.addr!=hart_address)goto packet_invalid; // wrong address
	in_hart_packet.cmd = ByteFifoPop(&rcv_hart_bytes);// read cmd num
	in_hart_packet.n_byte = ByteFifoPop(&rcv_hart_bytes);// read n_byte
	if(rcv_hart_bytes.count<in_hart_packet.n_byte+1 || in_hart_packet.n_byte>25)//bytes num<>bytes num SV
	{
		SendInvalidNumBytesTLG();
		goto packet_invalid;
	}
	for(uint8_t i = 0;i<in_hart_packet.n_byte;i++)
	{
		in_hart_packet.data[i] = ByteFifoPop(&rcv_hart_bytes);
	}
	in_hart_packet.crc = ByteFifoPop(&rcv_hart_bytes);
	if(in_hart_packet.crc!= CheckSumInHart())// if check sum invalid
	{
		SetInvalidCheckSumTLG();
		goto packet_invalid;
	}
	tlg_cnt++;
	InTlgReaction();
	packet_invalid:
		ByteFifoClear(&rcv_hart_bytes);
		return;
}
// Проверка checksum принятого пакета
// Вычисляется как логическое XOR для всех байт пакета, начиная с delimeter
uint8_t CheckSumInHart()
{
	uint8_t sum = 0;
	sum^=in_hart_packet.delimeter;
	sum^=in_hart_packet.addr;
	sum^= in_hart_packet.cmd;
	sum^= in_hart_packet.n_byte;
	for(uint8_t i=0;i<in_hart_packet.n_byte;i++)
	{
		sum^= in_hart_packet.data[i];
	}
	return sum;
}

// Реакция программы на принятый пакет в зависимости от команды
void InTlgReaction()
{
	switch(in_hart_packet.cmd)
	{
		case 6:// change hart addr cmd
			CmdChangeAddrExecute();
		break;
		default:
		SetInvalidCmdTLG();
	}
}

// Поменять адрес устройства с записью во flash
void CmdChangeAddrExecute()// change hart addr cmd
{
			if(in_hart_packet.data[0]>15)
			{
				out_hart_packet.status_0 = 32;				
			}
			else
			{
				if(hart_address!=in_hart_packet.data[0])
				{
					uint8_t temp = hart_address;
					hart_address = in_hart_packet.data[0];
					if(FlashWrite())// if write is successfull
					{
						out_hart_packet.status_0 = 0;// no errors
					}
					else
					{
						out_hart_packet.status_0 = 2;// internal error
						hart_address = temp;
					}
				}
				else out_hart_packet.status_0 = 0;// no errors
				
			}
			out_hart_packet.cmd = 0x06;
			out_hart_packet.n_byte = 0x03;
			out_hart_packet.data[0] = hart_address;
			SendRequest();			
}
// end of functions

