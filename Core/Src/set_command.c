#include "main.h"
#include "HartPacket.h"
#include "ByteFifo.h"

// begin private variables
HartPacket out_hart_packet;
// end private variables

// begin extern variables
extern ByteFifo bytes_for_send_hart;
extern uint8_t hart_address;
extern HartPacket in_hart_packet;
// end extern variables

// begin function prototips
void SendInvalidNumBytesTLG(void);
void SetInvalidCheckSumTLG(void);
void SetInvalidCmdTLG(void);
void SendRequest(void);
uint8_t GetCheksumOutHart(void);
// end function prototips

// begin functions

// Функция отправки телеграммы "В входящей телеграмме неправильно указаны кол-во байт"
void SendInvalidNumBytesTLG()
{
	out_hart_packet.cmd = in_hart_packet.cmd;
	out_hart_packet.n_byte = 2;
	out_hart_packet.status_0 = 16;
	SendRequest();
}

// Функция отправки телеграммы "В входящей телеграмме неправильно посчитана контрольная сумма"
void SetInvalidCheckSumTLG()
{
	out_hart_packet.cmd = in_hart_packet.cmd;
	out_hart_packet.n_byte = 2;
	out_hart_packet.status_0 = 8;
	SendRequest();
}

// Функция отправки телеграммы "В входящей телеграмме указан недопустимый номер команды"
void SetInvalidCmdTLG()
{
	out_hart_packet.cmd = in_hart_packet.cmd;
	out_hart_packet.n_byte = 2;
	out_hart_packet.status_0 = 4;
	SendRequest();
}

// Функция отправки телеграммы в ответ
void SendRequest()
{
	uint8_t i = 0;
	for(i=0;i<4;i++)ByteFifoPush(&bytes_for_send_hart,0xFF);// preambula
	out_hart_packet.delimeter = 0x01;
	out_hart_packet.delimeter = hart_address;
	ByteFifoPush(&bytes_for_send_hart,out_hart_packet.delimeter);// delimeter
	ByteFifoPush(&bytes_for_send_hart,out_hart_packet.addr);// hart_address
	ByteFifoPush(&bytes_for_send_hart,out_hart_packet.cmd);// cmd
	if(out_hart_packet.n_byte<2)out_hart_packet.n_byte = 2;
	ByteFifoPush(&bytes_for_send_hart,out_hart_packet.n_byte);// n_byte
	if(hart_address!=0)out_hart_packet.status_0|=1;// if device num >0, then  network mode
	ByteFifoPush(&bytes_for_send_hart,out_hart_packet.status_0);// status_0
	ByteFifoPush(&bytes_for_send_hart,out_hart_packet.status_1);// status_1
	for(i=0;i<out_hart_packet.n_byte-2;i++)ByteFifoPush(&bytes_for_send_hart,out_hart_packet.data[i]);// data
	ByteFifoPush(&bytes_for_send_hart, GetCheksumOutHart());// crc
}

// Функция вычисления контрольной суммы 
uint8_t GetCheksumOutHart()
{
	uint8_t crc = 0;
	crc^=out_hart_packet.delimeter;// crc xor delimeter
	crc^=out_hart_packet.addr;// crc xor addr
	crc^=out_hart_packet.cmd;// crc xor cmd
	crc^=out_hart_packet.n_byte;// crc xor n_byte
	crc^=out_hart_packet.status_0;// crc xor status_0
	crc^=out_hart_packet.status_1;// crc xor status_1
	for(uint8_t i=0;i<out_hart_packet.n_byte-2;i++)crc^=out_hart_packet.data[i];// data
	return crc;
}
// end functions