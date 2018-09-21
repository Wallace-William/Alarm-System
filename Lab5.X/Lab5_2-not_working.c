/*******************************
 * Name: William Wallace
 * Student ID#: 1001139276
 * Lab Day: 3-20-18
 * CSE 3442/5442 - Embedded Systems 1
 * Lab 2: Analog to Digital Conversion (ADC)
 ********************************/

#include <xc.h> // For the XC8 Compiler (automatically finds/includes the specific PIC18F452.h header file)
#include <math.h>

#define _XTAL_FREQ  10000000     // Running at 10MHz (external oscillator/crystal), REQUIRED for delay functions

#define LED_LEFT    PORTAbits.RA3  // QwikFlash red LED (left) to toggle
#define LED_CENTER  PORTAbits.RA2  // QwikFlash red LED (center) to toggle
#define LED_RIGHT   PORTAbits.RA1  // QwikFlash red LED (right) to toggle

// PIC18F452 Configuration Bit Settings
#pragma config OSC     = HS     // Oscillator Selection bits (HS oscillator)
#pragma config OSCS    = OFF    // Oscillator System Clock Switch Enable bit (Oscillator system clock switch option is disabled (main oscillator is source))
#pragma config PWRT    = OFF    // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOR     = OFF    // Brown-out Reset Enable bit (Brown-out Reset disabled)
#pragma config WDT     = OFF    // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config CCP2MUX = ON     // CCP2 Mux bit (CCP2 input/output is multiplexed with RC1)
#pragma config STVR    = ON     // Stack Full/Underflow Reset Enable bit (Stack Full/Underflow will cause RESET)
#pragma config LVP     = OFF    // Low Voltage ICSP Enable bit (Low Voltage ICSP disabled)

/*******************************
 * Global Vars
 * 
 * Strings for LCD Initialization and general use
 * For stability reasons, each array must have 10 total elements (no less, no more)
 ********************************/
const char LCDstr[]  = {0x33,0x32,0x28,0x01,0x0c,0x06,0x00,0x00}; // LCD Initialization string (do not change)

//Never change element [0] or [9] of the following char arrays
//You may only change the middle 8 elements for displaying on the LCD
char Str_1[] = {0x80,' ',' ',' ',' ',' ',' ',' ',' ',0};    // First line of LCD
char Str_2[] = {0xC0,' ',' ',' ',' ',' ',' ',' ',' ',0};    // Second line of LCD
char Str_Test[] = {0x80,'I','t',' ','W','o','r','k','s',0};

const char Clear1[] = {0x80,' ',' ',' ',' ',' ',' ',' ',' ',0};  // Clear first line of LCD
const char Clear2[] = {0xC0,' ',' ',' ',' ',' ',' ',' ',' ',0};  // Clear second line of LCD

unsigned int adLow = 0;             //Low/High bits from output
unsigned int adHigh = 0;
unsigned int adLow_5k = 0;          //Output for built in potentiometer
unsigned int adHigh_5k = 0;

unsigned char controlByte = 0;
unsigned char dataByte = 0;

int fullAD = 0;
long decimal, decimal_5k, voltage, voltage_ext, voltage_int, voltage_5k, tens, rem, hund, tens_1, rem_1;
double time;


/*******************************
 * Function prototypes
 ********************************/
void Initialize_PIC(void);
void Initialize_LCD(void);
void Print_To_LCD(const char *);
void Toggle_LEDs(void);
void DAC(void);

void main(void)
{
    Initialize_PIC();       //Initialize all settings required for general QwikFlash and LCD operation
	Print_To_LCD(Clear1);   //Clear the LCD line 1 to start your program
	Print_To_LCD(Clear2);   //Clear the LCD line 2 to start your program

    
    TRISEbits.RE2 = 1;
    TRISAbits.TRISA5 = 1;
    
    TRISCbits.RC5 = 0;
    TRISCbits.RC0 = 0;
    
    
	// Your personal PORT/TRIS/ADCON/etc settings or configurations can go here 
	// Or make your own function and call it
            
    // You can use the two delay functions defined in pic18.h (automatically included from xc.h):
    //      __delay_us(x);  // to delay in microsecond increments
    //                      // x is an unsigned long (0 - 4294967295)
    //
    //      __delay_ms(x);  // to delay in millisecond increments
    //                      // x is an unsigned long (0 - 4294967295)
    
    //Main routine, forever in an infinite while(true) loop
    while(1)
    {      
        DAC();
    }
}

void DAC(void)
{     
    
   SSPSTAT = 0b11000000;
   SSPCON1 = 0b00100000;
    
   ADCON0 = 0b00100001;        
   ADCON1 = 0b11000000;
        
  ADCON0 = 0b00100001;        //Settings previously used in Part 1 for external input
        ADCON1 = 0b11000000;
        
        __delay_ms(10);
        ADCON0bits.GO = 1;
        __delay_ms(10);             //Same as Part 1 ...
        
        while(ADCON0bits.DONE == 1);
        
        adLow = ADRESL;
        adHigh = ADRESH;
        
        adHigh = adHigh << 8;
        
        decimal = adLow + adHigh;
        
        voltage = (decimal*50)/1023;
        
        tens = voltage / 10;
        rem = voltage % 10;
        
        tens += 48;
        rem += 48;
        
        Str_2[3] = tens;
        Str_2[4] = 0x2E;
        Str_2[5] = rem;
        Str_2[6] = 0x56;
         
        ADCON1 = 0b10001110;
        Print_To_LCD(Str_2);            
        //__delay_ms(2000);
        __delay_ms(10);
        
        //Built in Potentiometer
        ADCON0 = 0b00111001;        //Set osc/4, AN4, !GO, ON
        ADCON1 = 0b11000010;        //Set RIGHT, osc/4, AN4, Vref +, Vref -
        
        __delay_ms(10);
        ADCON0bits.GO = 1;
        __delay_ms(10);
    
        while(ADCON0bits.DONE == 1);
        
        adLow_5k = ADRESL;
        adHigh_5k = ADRESH;
        
        adHigh_5k = adHigh_5k << 8;
        
        decimal_5k = adLow_5k + adHigh_5k;      //Same procedure from previous part
        
        voltage_5k = (decimal_5k*50)/1023;
        tens = voltage_5k / 10;
        rem = voltage_5k % 10;
        
        tens += 48;
        rem += 48;
        
        Str_1[3] = tens;
        Str_1[4] = 0x2E;
        Str_1[5] = rem;
        Str_1[6] = 0x56;
         
        ADCON1 = 0b10001110;
        Print_To_LCD(Str_1);
    
   for (time = 0; time<=1; time+=0.001)
   {
      PORTCbits.RC0 = 0;
      PORTCbits.RC5 = 1;
      
      dataByte = voltage_ext*sin(2*3.14159*voltage_int*time);
      
      controlByte = 0b00100001;
      
      SSPBUF = controlByte;
      while(SSPSTATbits.BF == 0); 
      
      //scale sin wave
      SSPBUF = dataByte;
      while(SSPSTATbits.BF == 0);
      
      PORTCbits.RC0 = 1;
      PORTCbits.RC5 = 0;
   }
        
    
}

/*******************************
 * Initialize_PIC(void)
 *
 * This function performs all initializations of variables and registers
 * for the PIC18F452 when specifically on the QwikFlash board.
 *
 * DO NOT CHANGE ANYTHING IN THIS FUNCTION
 ********************************/
void Initialize_PIC(void)
{
    // Reference the QwikFlash schematic (pdf) to understand analog/digital IO decisions
    
    ADCON1 = 0b10001110;    // Enable PORTA & PORTE digital I/O pins
    TRISA  = 0b11100001;    // Set I/O for PORTA
    TRISB  = 0b11011100;    // Set I/O for PORTB
    TRISC  = 0b11010000;    // Set I/0 for PORTC
    TRISD  = 0b00001111;    // Set I/O for PORTD
    TRISE  = 0b00000000;    // Set I/O for PORTE
    PORTA  = 0b00010000;    // Turn off all four LEDs driven from PORTA    
    LED_LEFT    = 0;        // All LEDs initially OFF
    LED_CENTER  = 0;        // All LEDs initially OFF
    LED_RIGHT   = 0;        // All LEDs initially OFF
    Initialize_LCD();       // Initialize LCD
}

/*******************************
 * Initialize_LCD(void)
 *
 * Initialize the Optrex 8x2 character LCD.
 * First wait for 0.1 second, to get past the displays power-on reset time.
 *
 * DO NOT CHANGE ANYTHING IN THIS FUNCTION
 *******************************/
void Initialize_LCD(void)
{
    char currentChar;
    char *tempPtr;

    __delay_ms(100);                // wait 0.1 sec (100 ms)

    PORTEbits.RE0 = 0;              // RS=0 for command
    tempPtr = LCDstr;

    while (*tempPtr)                // if the byte is not zero (end of string)
    {
        currentChar = *tempPtr;
        PORTEbits.RE1 = 1;          // Drive E pin high
        PORTD = currentChar;        // Send upper nibble
        PORTEbits.RE1 = 0;          // Drive E pin low so LCD will accept nibble          
        __delay_ms(10);             // wait 10 ms
        currentChar <<= 4;          // Shift lower nibble to upper nibble
        PORTEbits.RE1 = 1;          // Drive E pin high again
        PORTD = currentChar;        // Write lower nibble
        PORTEbits.RE1 = 0;          // Drive E pin low so LCD will process byte        
        __delay_ms(10);             // wait 10 ms	
        tempPtr++;                  // Increment pointer to next character
    }
}

/*******************************
 * Print_To_LCD(const char * tempPtr) 
 *
 * This function is called with the passing in of an array of a constant
 * display string.  It sends the bytes of the string to the LCD.  The first
 * byte sets the cursor position.  The remaining bytes are displayed, beginning
 * at that position.
 * This function expects a normal one-byte cursor-positioning code, 0xhh, or
 * an occasionally used two-byte cursor-positioning code of the form 0x00hh.
 *
 * DO NOT CHANGE ANYTHING IN THIS FUNCTION
 ********************************/
void Print_To_LCD(const char * tempPtr)
{
	char currentChar;
    PORTEbits.RE0 = 0;          // Drive RS pin low for cursor-positioning code

    while (*tempPtr)            // if the byte is not zero (end of string)
    {
        currentChar = *tempPtr;
        PORTEbits.RE1 = 1;      // Drive E pin high
        PORTD = currentChar;    // Send upper nibble
        PORTEbits.RE1 = 0;      // Drive E pin low so LCD will accept nibble
        currentChar <<= 4;      // Shift lower nibble to upper nibble
        PORTEbits.RE1 = 1;      // Drive E pin high again
        PORTD = currentChar;    // Write lower nibble
        PORTEbits.RE1 = 0;      // Drive E pin low so LCD will process byte
        __delay_ms(10);         // wait 10 ms	
        PORTEbits.RE0 = 1;      // Drive RS pin high for displayable characters
        tempPtr++;              // Increment pointerto next character
    }
}

/*******************************
 * Toggle_LEDs(void)
 *
 * This function simply toggles the QwikFlash's red LEDs in a simple sequence
 * The LED_X defines are at the top of this .c file
 * 
 * You may alter this function if you like or directly manipulate LEDs in other functions
 ********************************/
void Toggle_LEDs(void)
{
    LED_LEFT ^= 1;      // regardless of the bit's previous state, this flips it to 1 or 0
    __delay_ms(100);
    
    LED_CENTER ^= 1;
    __delay_ms(100);
    
    LED_RIGHT ^= 1;
    __delay_ms(100);
}

// Always have at least 1 blank line at the end of the .c file
