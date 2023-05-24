#include <avr/io.h>
#include "drivers/drivers.h"
#include "uart_menu.h"

// Function declarations
void SwPWM();
void IncBrightness();
void DecBrightness();
void PrintBar(uint8_t volume);

uint8_t pwm_counter = 0;
uint8_t pwm_trigger = 255;
uint8_t pwm_step = 16;

uint8_t rs485_TMPframe[RS485_FRAME_LENGTH] = {0};
uint8_t AdcResult;
uint8_t temperature = 26;

uint8_t led2 = 0;


int main () {

	// Initialize ports
	InitPorts();
	LedOut(0x00);

	// Initialize timer
	InitTimer();

	// Initialize ADC
	ADC_Init(0x00, 0x07);
	ADC_EnableInterrupts();

	// Initialize USART0
	Usart0Init(MYUBRR);
	Usart0ClearTerminal();
	Usart0CursorBlinkOn();

	// Initialize RS485
	Usart1Init(MYUBRR);

	// Initialize LCD
	LCD_Initialization();
	LCD_DisplayClear();
	LCD_CursorOff();

	// Initialize push buttons
	InitButton(1, IncBrightness);
	InitButton(2, DecBrightness);

	PrintBar( (pwm_trigger * 16) / 255 );



	// Print out the menu
	MenuPrint();

	while (1) {

	}

	return 0;
}

// Timer0 interrupt
ISR(TIMER0_OVF_vect) {
	sei();
	SwPWM();
}

// Timer2 interrupt
ISR(TIMER2_OVF_vect) {
	ButtonManagerFunctionCall();
}

// Timer 1 interrupt
ISR(TIMER1_COMPA_vect) {
	sei();

	ADC_ConvStart(0x00);

	// Convert temp to string
	char temp_str[2] = {"00"};
	char lcd_temp_str[16] = {"  C             "};

	temp_str[0] = (temperature / 10) % 10 + '0';
	lcd_temp_str[0] = (temperature / 10) % 10 + '0';

	temp_str[1] = temperature % 10 + '0';
	lcd_temp_str[1] = temperature % 10 + '0';

	RsCreateFrame("AR", "TM", temp_str);
	RsSendFrame();

	LCD_SendStringToLine(lcd_temp_str, 3);
}

ISR(ADC_vect) {
	sei();

	AdcResult = ADC;
	AdcResult = AdcResult >> 2;
	AdcResult = AdcResult/10;
	temperature = AdcResult >> 2;

	ADCSRA &= ~(1<<ADIF);
}


// SoftwarePWM
void SwPWM() {
	sei();

	if (pwm_counter <= pwm_trigger) {
		RgbLed(0, 0 ,1);
	}
	else {
		RgbLed(0, 0, 0);
	}
	
	pwm_counter++;
}

// Increment brightness
void IncBrightness() {
	if (pwm_trigger < 255-16) {
		pwm_trigger += pwm_step;
	} else {
		pwm_trigger = 255;
	}

	PrintBar( (pwm_trigger * 16) / 255 );
}

// Decrement brightness
void DecBrightness() {
	if (pwm_trigger > 0+16) {
		pwm_trigger -= pwm_step;
	} else {
		pwm_trigger = 0;
	}

	PrintBar( (pwm_trigger * 16) / 255 );
}

void PrintBar(uint8_t volume) {
	uint8_t i = 0;
	char bar[16] = "                ";		// 16 spaces

	// Clear the bar
	for (i = 0; i < 16; i++) {
		bar[i] = ' ';
	}

	// Fill the bar with #
	for (i = 0; i < volume; i++) {
		bar[i] = '#';
	}

	// Print out the bar to the LCD
	LCD_SendStringToLine(bar, 0);

	// Convert the volume into a percentage string
	uint8_t percentage = (volume * 100) / 16;

	char volume_str[16] = {"                "};
	volume_str[0] = percentage / 100 + '0';
	volume_str[1] = (percentage / 10) % 10 + '0';
	volume_str[2] = percentage % 10 + '0';
	volume_str[3] = '%';

	LCD_SendStringToLine(volume_str, 1);
}