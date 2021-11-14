#ifndef HART_PACKET_
#define HART_PACKET_

#include <stdint.h>
#include <stdbool.h>

struct HartPacket
{
	uint8_t preambula;
	uint8_t delimeter;
	uint8_t addr;
	uint8_t cmd;
	uint8_t n_byte;
	uint8_t status_0;
	uint8_t status_1;
	uint8_t data[25];
	uint8_t crc;
}typedef HartPacket;

#endif

