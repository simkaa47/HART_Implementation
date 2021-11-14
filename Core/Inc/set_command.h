#ifndef SET_COMMAND_
#define SET_COMMAND_

#include <stdint.h>
#include <stdbool.h>

void SendInvalidNumBytesTLG(void);
void SetInvalidCheckSumTLG(void);
void SetInvalidCmdTLG(void);
void SendRequest(void);

#endif


