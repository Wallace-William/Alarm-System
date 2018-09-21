/*******************************
 * Name: William Wallace
 * Student ID#: 1001139276
 * CSE 3442/5442 - Embedded Systems 1
 * Homework 2
 * 
 * Homework 2 Description:
 *  	Use PIC18F452 and XC8 for this assignment
 * 
 *  	Use CCP1's Capture Mode and Interrupts
 *  	to determine the frequency of a signal
 *  	coming through PIN RC2 (CCP1)
 * 
 *  	printf() the calculated frequency continuously
 * 
 *  	Use Clock Stimulus to generate the signal
 *  	within MPLAB's Simulator (50Hz to 500Hz)
 ********************************/ 

// PIC18F452 Configuration Bit Settings
#pragma config OSC = HS         // Oscillator Selection bits (HS oscillator)
#pragma config OSCS = OFF       // Oscillator System Clock Switch Enable bit (Oscillator system clock switch option is disabled (main oscillator is source))
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOR = OFF        // Brown-out Reset Enable bit (Brown-out Reset disabled)
#pragma config BORV = 20        // Brown-out Reset Voltage bits (VBOR set to 2.0V)
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 128      // Watchdog Timer Postscale Select bits (1:128)
#pragma config CCP2MUX = ON     // CCP2 Mux bit (CCP2 input/output is multiplexed with RC1)
#pragma config STVR = OFF       // Stack Full/Underflow Reset Enable bit (Stack Full/Underflow will not cause RESET)
#pragma config LVP = OFF        // Low Voltage ICSP Enable bit (Low Voltage ICSP disabled)

#include <xc.h>
#include <stdio.h> // To use the printf() statement for Simulator's USART Output Window
                   // printf XC8 details: http://microchipdeveloper.com/tls2101:printf

#define _XTAL_FREQ 4000000 // Fosc = 4MHz, required for __delay_ms() and __delay_us() functions

// To allow regular console printing within MPLAB's Simulator (override the putch function)
// Go to File > Project Properties > Simulator > Option Categories > Uart1 IO Options > Enable Uart IO (check the box)
// For full info go here: http://microchipdeveloper.com/xc8:console-printing
int high, low, current, prev=0, dif, freq;
void timer1(void);

void putch(unsigned char data) 
{
    while(!PIR1bits.TXIF) // wait until the transmitter is ready
        continue;
    
    TXREG = data;   // send one character
}

void interrupt capture(void) //when high priority interrupt is triggered
{
    
    //if rising edge interrupt is triggered and CCP1 interrupt enabled
        timer1();             //Frequency routine
        
        //printf("interrupt\n");
        PIR1bits.CCP1IF = 0;  //Reset flag
    
    
}

void timer1(void)
{
    //printf("timer1");
    high = CCPR1H << 8;  //Save capture
    low = CCPR1L;
    
    current = high + low;
    dif = current - prev;
    prev = current;
    
    freq = 1000000/dif;
}

void main(void) 
{
    
    // Needed for printing in MPLAB's UART Console window of the Simulator
    TXSTAbits.TXEN = 1; // enable transmitter
    RCSTAbits.SPEN = 1; // enable serial port
    
    //printf("test");
   
    //Input (RC2)
    TRISC = 0b11111111;
   
    
    CCP1CON = 0b00000101; //Capture mode every rising edge
    PIE1bits.CCP1IE = 1;
    //TMR3H = 0x48;
    //TMR3L = 0xE5;
    //RC2 CCP1IE PEIE GIE
   
    //T1CONbits.TMR1ON = 1; 
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;//Enable all unmasked interrupts
    T1CONbits.TMR1ON = 1; 
    
    //T1CONbits.TMR1ON = 1; 
    //Turn TMR1 ON
    
    while(1)
    {
        printf("%d Hz\n", freq);
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
