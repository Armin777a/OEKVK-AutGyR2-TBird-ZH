#include <avr/io.h>
#include "drivers/drivers.h"
#include <util/delay.h>


int main () {

	// Initialize ports
	InitPorts();

	// Initialize timer
	InitTimer();

	// Initialize USART0
	Usart0Init(MYUBRR);
	Usart0CursorBlinkOff();
	Usart0ClearTerminal();

	// Initialize LCD
	LCD_Initialization();
	LCD_DisplayClear();
	LCD_CursorOff();
	LCD_SendString("Hello World!");

	while (1) {

	}

	return 0;
}