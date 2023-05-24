#include "uart_menu.h"

uint8_t counter = 0;
uint8_t leds = 0x00;
unsigned char betu = 0;
unsigned char szoveg[10] = {0};
uint8_t szoveg_i = 0;
uint8_t match_count = 0;
unsigned char *p = 0;
uint8_t equals = 0;

unsigned char* menu[7] = {
	"LED7 ON",
	"LED7 OFF",
	"LED6 ON",
	"LED6 OFF",
	"LED5 ON",
	"LED5 OFF",
	"HELP"
};

uint8_t rgb_leds[3] = {0};




// ============== UART MENU FUNCTIONS ==============

// Uart 0 interrupt
ISR(USART0_RX_vect) {
    char betu = UDR0;

    uint8_t i = 0;

    if(betu != '\r' && betu != '\b') {
        Usart0Transmit(betu);
        szoveg[szoveg_i] = betu;
        szoveg_i++;
    }
    
    if(szoveg_i >= 10) {
        Usart0TransmitString("\r\n\nROSSZ MUVELET!\r\n\n");
        for(i=0; i<10; i++){
            szoveg[i] = 0;
        }
        szoveg_i = 0;
        betu = 0;
    }
    
    if(betu == '\b') {
        if(szoveg_i) {
            szoveg_i--;
            szoveg[szoveg_i] = 0;
            Usart0TransmitString("\b \b");
        }
    }
    
    if(betu == '\r') {
        szoveg[szoveg_i] = '\0';
        
        for(i=0; i<7; i++) {
            p = menu[i];			
            if(MenuStringComp(p, szoveg)){
                equals = i + 1;
                Usart0TransmitString("\r\n\nSIKERES MUVELET!\r\n\n");
                MoveCursorBack();
            }
        }
        
        if(!equals) {
            Usart0TransmitString("\r\n\nROSSZ MUVELET!\r\n\n");
            MoveCursorBack();
        }
        
        switch(equals) {
            case 1:
                leds |= (1 << 7);
                break;
            case 2:
                leds &= ~(1 << 7);
                break;
            case 3:
                leds |= (1 << 6);
                break;
            case 4:
                leds &= ~(1 << 6);
                break;
            case 5:
                leds |= (1 << 5);
                break;
            case 6:
                leds &= ~(1 << 5);
                break;
            case 7:
                Usart0ClearTerminal();
                MenuPrint();
                break;
        }
        
        LedOut(leds);

        for(i=0; i<10; i++) {
            szoveg[i] = 0;
        }
        szoveg_i = 0;
        betu = 0;
        equals = 0;
    }
    
    RgbLed(rgb_leds[0], rgb_leds[1], rgb_leds[2]);
}



// Print out the menu
void MenuPrint() {
    Usart0TransmitString("Valasszon muveletet:\r\n");
	
    uint8_t i = 0;

	while(i<7) {
		Usart0Transmit(i + '1');
		Usart0TransmitString(" - ");
		Usart0TransmitString(menu[i]);
		Usart0Transmit('\r');
		Usart0Transmit('\n');
		i++;
	}
	
	Usart0TransmitString("\r\nIrja be a muveletet:\r\n\n");
}



// Compare two strings
uint8_t MenuStringComp(unsigned char* p_menu, unsigned char* p_text) {
	
	while(1) {
		
		if (*p_menu != *p_text) {
			return 0;
		}
		
		if (!(*p_menu)) {
			if(*p_text) {
				return 0;
			} else {
				return 1;
			}
		}
		
		p_menu++;
		p_text++;
	}
}

void MoveCursorBack() {
    return;
}