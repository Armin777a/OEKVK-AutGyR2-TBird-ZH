#pragma once
#ifndef RS485_H_
#define RS485_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include "../clockfreq.h"
#include "../uart/uart.h"
#include "../led/led.h"
#include "../lcd/lcd.h"

// Defines
#define RS485_BUFFER_SIZE   20
#define RS485_FRAME_LENGTH  9

#define FRAME_SOF           0       // Start of frame 0x03
#define FRAME_ID            1       // 2 x ASCII
#define FRAME_COMMAND       3       // 2 x ASCII
#define FRAME_DATA          5       // 2 x ASCII
#define FRAME_CRC           7       // XOR 0-5 bytes
#define FRAME_EOF           8       // End of frame 0xF4


// Rs485 functions
void RsReadEnable();
void RsWriteEnable();
void RsSetRE(uint8_t state);
void RsSetDE(uint8_t state);


// Fifo functions
uint8_t RsFifoPut(uint8_t *data);
uint8_t RsFifoGet(uint8_t *data);
void RsFifoClear(void);
uint8_t RsFifoIsFull(void);
uint8_t RsFifoCount(void);

void RsReturnData(char* data);

// For sending data
void RsCreateFrame(char* id, char* command, char* data);
void RsSendFrame();
uint8_t RsGetCrc(uint8_t* data);



#endif