/*
 * Embedded 1: Lecture 11 - Interrupts
 * 
 */

#include <xc.h>
#include <stdio.h> // To use the printf() statement

void Input_Detected_1(void);
void Input_Detected_2(void);

// To allow regular console printing within MPLAB's Simulator (override the putch function)
// For full info go here: http://microchipdeveloper.com/xc8:console-printing
void putch(unsigned char data) 
{
    while(!PIR1bits.TXIF) // wait until the transmitter is ready
        continue;
    
    TXREG = data;   // send one character
}

// HIGH Priority Interrupt Service Routine
// Can take away control from LOW Priority Interrupts
void interrupt My_ISR_High(void)
{
    // Was the triggered interrupt EXTERNAL INTERRUPT 0? (pin B0)
    // If the interrupt IS ENABLED and its FLAG IS SET
    if((INTCONbits.INT0E == 1) && (INTCONbits.INT0F == 1))
    {
        Input_Detected_1();
        INTCONbits.INT0IF = 0;  // Clear INT0 Flag bit ("reset" it)
    }
}

// LOW Priority Interrupt Service Routine
// Control can be taken away (paused,resumed) by the HIGH Priority Interrupts
void interrupt low_priority My_ISR_Low(void)
{
    // Was the triggered interrupt EXTERNAL INTERRUPT 1? (pin B1)
    // If the interrupt IS ENABLED and its FLAG IS SET
    if((INTCON3bits.INT1E == 1) && (INTCON3bits.INT1F == 1))
    {
        Input_Detected_2();
        INTCON3bits.INT1IF = 0;  // Clear INT1 Flag bit ("reset" it)
    }
}

void main(void) 
{
    // Needed for printing in MPLAB's UART Console window of the Simulator
    TXSTAbits.TXEN = 1; // enable transmitter
    RCSTAbits.SPEN = 1; // enable serial port
    
    TRISB = 0b00000011;  // input for INT0 and INT1 (pins B0 and B1)
    TRISD = 0b00000000;  // output for PORTD pins, just random placeholder code
    
    // Interrupt settings
    INTCONbits.INT0E = 1;   // External INT0 enabled (pin B0)
    INTCON3bits.INT1IE = 1; // External INT1 enabled (pin B1)
    INTCON3bits.INT1IP = 0; // External INT1 is LOW priority
    
    RCONbits.IPEN = 1;      // Allow HIGH and LOW priority interrupts (instead of all HIGH)
    INTCONbits.PEIE = 1;    // Enable for LOW interrupts
    INTCONbits.GIE = 1;     // Enable (release the hold) for HIGH interrupts (that have been indiv. enabled)
    
    while(1)
    {
        printf("test");
        PORTD = 0b10101010;
        PORTD = 0b01010101;
        PORTD = 0b10101010;
        PORTD = 0b01010101;
        PORTD = 0b10101010;
        PORTD = 0b01010101;
        PORTD = 0b10101010;
        PORTD = 0b01010101;
        
    }
}

void Input_Detected_1()
{
    printf("Pin B0 is 1 !!!\n");
}

void Input_Detected_2()
{
    printf("Pin B1 is 1 !!!\n");
}
