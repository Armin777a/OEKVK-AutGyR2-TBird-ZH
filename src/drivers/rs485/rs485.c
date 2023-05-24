// Include header file
#include "rs485.h"



// ===============================================
// Global variables
// ===============================================

uint8_t rs485_buffer[RS485_BUFFER_SIZE][RS485_FRAME_LENGTH] = {0};
uint8_t rs485_buffer_head = 0;
uint8_t rs485_buffer_tail = 0;

uint8_t rs485_frame[RS485_FRAME_LENGTH] = {0};
uint8_t rs485_frame_send[RS485_FRAME_LENGTH] = {0};

uint8_t ledBlink = 0x01;

uint8_t isMaster = 1;


// ===============================================
// Interrupts
// ===============================================

// UART1 interrupt routine
ISR(USART1_RX_vect) {
    uint8_t data = UDR1;                            // Get the data from the UART1 data register

    if (isMaster == 1) {
        return;
    }

    if (data == 0x03) {                             // Check if the data is the start of frame
        //ledBlink ^= 0x01;
        //LedOut(ledBlink);

        for (uint8_t i = 0; i < RS485_FRAME_LENGTH; i++) {
            rs485_frame[i] = 0;                     //   Clear the frame buffer
        }

        rs485_frame[FRAME_SOF] = data;              //   If so put the data in the frame buffer
        
   
    } else if (data == 0xF4) {                      // Check if the data is the end of frame
        rs485_frame[FRAME_EOF] = data;              //   If so put the data in the frame buffer
        RsFifoPut(rs485_frame);                     //   Put the frame in the fifo

        /*
        RsReturnData(text);
	    LCD_SendStringToLine(text, lcd_line);
	    lcd_line++;

        if (lcd_line > 3) {
            lcd_line = 0;
        }
        */

    } else {                                        // If the data is not the start or end of frame
        for (uint8_t i = 0; i < RS485_FRAME_LENGTH; i++) {
            if (rs485_frame[i] == 0) {              // Check if the frame buffer is empty
                rs485_frame[i] = data;              //   If so put the data in the frame buffer
                break;                              //   Break out of the for loop
            }
        }
    }
}



// ===============================================
// RS485
// ===============================================

void RsCreateFrame(char* id, char* command, char* data) {

    char id_U = id[0];
    char id_L = id[1];
    char command_U = command[0];
    char command_L = command[1];
    char data_U = data[0];
    char data_L = data[1];

    rs485_frame_send[FRAME_SOF] = 0x03;
    rs485_frame_send[FRAME_ID] = (uint8_t)id_U;
    rs485_frame_send[FRAME_ID+1] = (uint8_t)id_L;
    rs485_frame_send[FRAME_COMMAND] = (uint8_t)command_U;
    rs485_frame_send[FRAME_COMMAND+1] = (uint8_t)command_L;
    rs485_frame_send[FRAME_DATA] = (uint8_t)data_U;
    rs485_frame_send[FRAME_DATA+1] = (uint8_t)data_L;
    rs485_frame_send[FRAME_CRC] = RsGetCrc(rs485_frame_send);
    rs485_frame_send[FRAME_EOF] = 0xF4;
}

// If isMaster is 1, the device is a master and can send data
void RsSendFrame() {
    if (!isMaster) {
        return;
    }

    RsWriteEnable();
    _delay_ms(1);

    for (uint8_t i = 0; i < RS485_FRAME_LENGTH; i++) {
        Usart1Transmit((char)rs485_frame_send[i]);
    }

    _delay_ms(1);
    RsReadEnable();
}

uint8_t RsGetCrc(uint8_t* data) {
    uint8_t crc = 0x00;

    for (uint8_t i = 0; i < FRAME_DATA+1; i++) {
        crc ^= data[i];
    }

    return crc;
}

void RsReturnData(char* data) {
    uint8_t i = 0;

    for (i = 0; i < FRAME_DATA; i++) {
        data[i] = rs485_frame[FRAME_ID+i];
    }
}

void RsReadEnable() {
    RsSetRE(0);
    RsSetDE(0);
}

void RsWriteEnable() {
    RsSetRE(1);
    RsSetDE(1);
}

void RsSetRE(uint8_t state) {
    if (state) {
        PORTC |= (1 << PC7);
    } else {
        PORTC &= ~(1 << PC7);
    }
}

void RsSetDE(uint8_t state) {
    if (state) {
        PORTE |= (1 << PC2);
    } else {
        PORTE &= ~(1 << PC2);
    }
}



// ===============================================
// Fifo
// ===============================================

// Put data in the rs485_buffer
uint8_t RsFifoPut(uint8_t *data) {
    uint8_t next = rs485_buffer_head + 1;               // Get the next head

    if (next >= RS485_BUFFER_SIZE) {                    // Check if the next head is out of bounds
        next = 0;                                       //   If so move the head to the start of the buffer
    }

    if (next == rs485_buffer_tail) {                    // Check if the next head is the same as the tail
        return 0;                                       //   If so the buffer is full, return 0 to indicate failure
    }

    for (uint8_t i = 0; i < RS485_FRAME_LENGTH; i++) {
        rs485_buffer[rs485_buffer_head][i] = data[i];   // Put the data in the buffer
    }
    rs485_buffer_head = next;                           // Set the head to the next head

    return 1;                                           // Return 1 to indicate success
}


// Get data from the rs485_buffer
uint8_t RsFifoGet(uint8_t *data) {
    if (rs485_buffer_head == rs485_buffer_tail) {       // Check if the head is the same as the tail
        return 0;                                       //  If so the buffer is empty, return 0 to indicate failure
    }

    for (uint8_t i = 0; i < RS485_FRAME_LENGTH; i++) {
        data[i] = rs485_buffer[rs485_buffer_tail][i];   // Get the data from the buffer
    }
    
    rs485_buffer_tail++;                                // Increment the tail

    if (rs485_buffer_tail >= RS485_BUFFER_SIZE) {       // Check if the tail is out of bounds
        rs485_buffer_tail = 0;                          //  If so move the tail to the start of the buffer
    }

    return 1;                                           // Return the data
}


// Clear the rs485_buffer
void RsFifoClear(void) {
    rs485_buffer_head = 0;                       // Set the head to the start of the buffer
    rs485_buffer_tail = 0;                       // Set the tail to the start of the buffer
}


// Check if the rs485_buffer is full
uint8_t RsFifoIsFull(void) {
    uint8_t next = rs485_buffer_head + 1;       // Get the next head

    if (next >= RS485_BUFFER_SIZE) {            // Check if the next head is out of bounds
        next = 0;                               //   If so move the head to the start of the buffer
    }

    if (next == rs485_buffer_tail) {            // Check if the next head is the same as the tail
        return 1;                               //  If so the buffer is full, return 1 to indicate full
    }

    return 0;                                   // Return 0 to indicate not full
}


// Return the number of frames in the rs485_buffer
uint8_t RsFifoCount(void) {
    if (rs485_buffer_head >= rs485_buffer_tail) {                       // Check if the head is ahead of the tail
        return rs485_buffer_head - rs485_buffer_tail;                   //  If so return the difference
    }

    return RS485_BUFFER_SIZE - rs485_buffer_tail + rs485_buffer_head;   // Return the difference between the head and the tail
}
