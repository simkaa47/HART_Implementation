#include "ByteFifo.h"

ByteFifo rcv_bits;// recived bits
ByteFifo rcv_hart_bytes;// recived bytes
extern uint8_t time_without_receive;
bool need_preambula;

// Функция изьятия из очереди 11 бит
void GetByteFromFifo()
{
	for(uint8_t i=0;i<11;i++)
	{
		ByteFifoPop(&rcv_bits);
	}
}
void DecodeBits()// Функция определения байта из битовой последовательности
{
	while(rcv_bits.count>=11)  // Если бит меньше 11, то нет смысла что то декодировать
	{
		if(!ByteFifoByteAvialable(&rcv_bits))// Если первые 11 бит не проходят по старт-стоп битам и контролю четности
		{
			need_preambula = true; // необходима синхронизация
			ByteFifoPop(&rcv_bits);// изымаем крайний бит в очереди
		}
		else
		{
			if(need_preambula)// Если битовый поток не синхронизирован 
			{
				if(rcv_bits.avialable_byte==0xFF)// Если первые 11 бит проходят по старт-стоп битам и контролю четности
				{
					GetByteFromFifo();// Удаляем из очереди крайние 11 бит
					need_preambula = false;	// битовый поток синхронизирован				
				}
				else ByteFifoPop(&rcv_bits); // изымаем крайний бит в очереди
			}
			else // В случае, если поток синхронизирован и байт соответствует
			{
				GetByteFromFifo();
				ByteFifoPush(&rcv_hart_bytes, rcv_bits.avialable_byte);// ложим байт в очередь байтов, чтобы потом декодировать как пакет
				time_without_receive=0;// сброс счетчика времени непрниятия информации
			}
		}
	}
}
