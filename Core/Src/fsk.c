#include "main.h"
#include "ByteFifo.h"
#include "IntFifo.h"
#include "gpio_rw.h"

#define DPH_1               134217728 // FREQ_1 / FS * (1 << 32) (using 2*pi rad = 2^32)
#define DPH_0               246065835 // FREQ_0 / FS * (1 << 32) (using 2*pi rad = 2^32)
int16_t level = 544; // 4 mA
const int8_t SINE[256] = { // 68 counts = 0.5 mA
0,   2,   3,   5,   7,   8,   10,  12,  13,  15,  17,  18,  20,  21,  23,  25,
26,  28,  29,  31,  32,  34,  35,  37,  38,  39,  41,  42,  43,  45,  46,  47,
48,  49,  51,  52,  53,  54,  55,  56,  57,  58,  59,  59,  60,  61,  62,  62,
63,  64,  64,  65,  65,  66,  66,  67,  67,  67,  68,  68,  68,  68,  68,  68,
68,  68,  68,  68,  68,  68,  68,  67,  67,  67,  66,  66,  65,  65,  64,  64,
63,  62,  62,   61,  60,  60,  59,  58,  57,  56,  55,  54,  53,  52,  51,  49,
48,  47,  46,  45,  43,  42,  41,  39,  38,  37,  35,  34,  32,  31,  29,  28,
26,  25,  23,  21,  20,  18,  17,  15,  14,  12,  10,  8,   7,   5,   3,   2, 
0,   -2,  -3,  -5,  -7,  -8,  -10, -12, -13, -15, -17, -18, -20, -21, -23, -25,
-26, -28, -29, -31, -32, -34, -35, -37, -38, -39, -41, -42, -43, -45, -46, -47,
-48, -49, -51, -52, -53, -54, -55, -56, -57, -58, -59, -59, -60, -61, -62, -62,
-63, -64, -64, -65, -65, -66, -66, -67, -67, -67, -68, -68, -68, -68, -68, -68,
-68, -68, -68, -68, -68, -68, -68, -67, -67, -67, -66, -66, -65, -65, -64, -64,
-63, -62, -62, -61, -60, -60, -59, -58, -57, -56, -55, -54, -53, -52, -51, -49,
-48, -47, -46, -45, -43, -42, -41, -39, -38, -37, -35, -34, -32, -31, -29, -28,
-26, -25, -23, -21, -20, -18, -17, -15, -14, -12, -10, -8,  -7,  -5,  -3,  -2
};

// begin static variables
static int32_t prev_pll, pll;
static bool prev_bit;
static bool sending;
static int32_t x_prev, x_curr; // x[n-1], x[n]
static int32_t y_prev, y_curr; // y[n-1], y[n]
//end static variables

// begin private variables
uint16_t recv_buff[1024]; // массив для приема данных от АЦП при помощи DMA
uint16_t level_ADC_hart; // вычисленный средний уровень сигнала АЦП
IntFifo sig_fifo; // 
IntFifo fifo_for_level; // буффер для вычисления среднего уровня АЦП
uint16_t buff_send[32]; // массив для передачи данных в модуль ЦАП при помощи DMA
ByteFifo bit_fifo_dac; // FIF0 для для хранения бит для передачи в ЦАП
uint8_t bit_value=2;// характеризует бит для передачи в ЦАП (0 - false, 1  - true, 2 - mark)
uint32_t phase; // смещение фазы сигнала ЦАП
uint32_t GetLevelFromFifo(IntFifo*, uint16_t);// прототип функции получения среднего сигнала ЦАП
uint16_t value_for_dac;// общий уровень ЦАП
// end private variables

// begin extern variables
extern DAC_HandleTypeDef hdac1; // структура данных ЦАП
extern TIM_HandleTypeDef htim6; // структура данных таймера тактирования ЦАП
extern TIM_HandleTypeDef htim2; // структура данных таймера тактирования АЦП
extern ADC_HandleTypeDef hadc; // структура данных АЦП
extern ByteFifo rcv_bits; // FIFO для хранения демодулированных битов
extern uint8_t hart_address; // адрес для хранения Hurt-адреса сигнала
extern uint8_t analog_input;// MCU работает как модуль аналогового ввода если ==1, иначе как модуль вывода
// end extern variables


void FSK_Init(void) // функция инициадизации, выполняется один раз при старте
{	
	HAL_TIM_Base_Start(&htim6);// старт таймера ЦАП
	HAL_DAC_Start_DMA (&hdac1, DAC_CHANNEL_1,(uint32_t*)buff_send,32,DAC_ALIGN_12B_R);// старт передачи данных в модуль ЦАП при помощи DMA
	prev_pll = 0; // предыдущая фаза ФАПЧ
	pll = 0;// текущая фаза ФАПЧ
	prev_bit = false;		
	for (uint8_t i = 0; i < 4; i++) { 
		IntFifoPush(&sig_fifo,0);
	}
	HAL_ADCEx_Calibration_Start(&hadc);// калибровка АЦП
	HAL_TIM_Base_Start(&htim2); // старт таймера АЦП	
	HAL_ADC_Start_DMA(&hadc, (uint32_t*) &recv_buff, 1024);// старт передачи данных из АЦП при помощи DMA
	
	
}
void FillBuffer(uint8_t start_index, uint8_t finish_index)// Функция заполнения буффера для отправки на ЦАП
{
	uint32_t phase_shift = 0; // фазовое смещение
	uint8_t index = 0;
	if(hart_address==0) 
	{
		level = 544 + value_for_dac; // если устройство работает в режиме "точка-точка", то аналоговый выходной сигнал не фиксируется
	}
	else
	{
		level = 544; // срений упровень ЦАП  == 4mA
	}
	switch(bit_value)// 
	{
		case 0: // если нужно модулировать бит == false
			phase_shift = DPH_0; 
		SendLedOn(); // включаем светодиод
		break;
		case 1:
			phase_shift = DPH_1;// если нужно модулировать бит == true
			SendLedOn(); // включаем светодиод
		break;
		default:
			phase_shift = 0; // не нужно ничего модулировать
			phase = 0;	
			SendLedOff();		// выключаем светодиод
	}
	for(uint8_t i=start_index; i<finish_index;i++)
	{
		if(bit_value<2)
		{
			phase+=phase_shift; // увеличиваем смещение
			index = phase >> 24; // высчитываем индекс
			if(analog_input==1)buff_send[i]=(uint16_t)(SINE[index]*5+level); // если модуль работает как аналоговый вход, уыеличиваем амплитуду модулированного сигнала в 4 раза
			else buff_send[i]=(uint16_t)(SINE[index])+level;// заполнение массива для отправки на ЦАП
		}
		else buff_send[i]=level;		// если не нужно ничего передавать, просто отправляем среднее значение 	ЦАП
	}	
}
void AFSK_decode_sig(uint16_t *sig_buff, uint32_t len) { // функция демодулирования
	
	int32_t sig = 0; // выход фильтра 
	for (uint32_t i = 0; i < len; i++) {
		// simplified demodulation and 1200Hz IIR filter, designed for 9600Hz sample rate
		level_ADC_hart = GetLevelFromFifo(&fifo_for_level, sig_buff[i]);// определяем средний уровень сигнала
		sig = sig_buff[i] - level_ADC_hart;// амплитуда синусоиды		
		x_prev = x_curr;
		x_curr = (IntFifoPop(&sig_fifo) * sig) >> 2; // x[0]*0.29289322, precalculating
		y_prev = y_curr;
		y_curr = x_prev + x_curr + (y_prev >> 1); // x[0]*0.29289322 + x[-1]*0.29289322 + y[-1]*0.41421356
		bool bit = y_curr < 0; 
		IntFifoPush(&sig_fifo, sig); // ложим значение амлитуды в FIFO, чтобы через 4 шага еще воспользоваться им

		// PLL, sample
		if (bit != prev_bit) { // если уровень отфильтрованного сигнала поменялся, то уменьшаем фазу ФАПЧ
			pll >>= 1; // divide by 2 to nudge to 0
		}
		prev_pll = pll;
		pll += 1 << 29; // увеличиваем фазу ФАПЧ
		if (pll < 0 && prev_pll > 0) { // если произощло переполнение INT, ложим бит в очередь демодулированных битов
			ByteFifoPush(&rcv_bits, bit);
		}
		prev_bit = bit;
	}
}
uint32_t GetLevelFromFifo(IntFifo* fifo, uint16_t adc_value)// получаем средний уровень сигнала
{
	uint32_t max = 0;
	uint32_t min = 65535;
	if(fifo->count>=4)IntFifoPop(fifo);
	IntFifoPush(fifo, adc_value);	
	for(uint8_t i = 0; i<4;i++)
	{
		if(max<fifo->arr[i]) max = fifo->arr[i];
		if(min>fifo->arr[i]) min = fifo->arr[i];
	}
	return (max + min)/2;	
}
void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac) // прерывание от модуля ЦАП(полностбю отправлен буффер на ЦАП)
{
	if(bit_fifo_dac.count>0) 
	{
		bit_value = ByteFifoPop(&bit_fifo_dac); // производим выборку из FIFO
    sending = true;		
	}
	else
	{
		bit_value = 2;
		phase = 0;
		sending = false;
	}
	FillBuffer(0,16);
}
void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac) // прерывание от модуля ЦАП (выполнено преобразование половины буфера)
{
	FillBuffer(16,32);	
}
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) { // прерывание от модуля АЦП (заполнена половина буфера)
	if(!sending)AFSK_decode_sig(recv_buff, sizeof(recv_buff)>>2);// обработка второй половины буфера
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {// прерывание от модуля АЦП (заполнен весь буфер)
	if(!sending)AFSK_decode_sig(recv_buff + (sizeof(recv_buff)>>2), sizeof(recv_buff)>>2);	// обработка первой половины буфера
}


