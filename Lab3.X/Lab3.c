/*******************************
 * Name: William Wallace
 * Student ID#: 1001139276
 * Lab Day: 2-27-18
 * CSE 3442/5442 - Embedded Systems 1
 * Lab 3: GPIO
 ********************************/

#include <xc.h> // For the XC8 Compiler (automatically finds/includes the specific PIC18F452.h header file)

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

const char Clear1[] = {0x80,' ',' ',' ',' ',' ',' ',' ',' ',0};  // Clear first line of LCD
const char Clear2[] = {0xC0,' ',' ',' ',' ',' ',' ',' ',' ',0};  // Clear second line of LCD


/*******************************
 * Function prototypes
 ********************************/
void Initialize_PIC(void);
void Initialize_LCD(void);
void Print_To_LCD(const char *);
void Toggle_LEDs(void);

void ADD(void);
void SUB(void);
void AND(void);
void NOT(void);
void MULT(void);

int sum, minus, tens, neg, tens_n, rem_n, and, not, mult, hund;
unsigned int Bdecimal, Cdecimal, rem;

char add[] = {0x80,' ',' ',0x42,0x2B,0x43,' ',' ',' ',0};//B+C
char addlow[] = {0xC0,' ',' ',' ',' ',' ',' ',' ',' ',0};
char sub[] = {0x80,' ',' ',0x42,0x2D,0x43,' ',' ',' ',0};//B-C
char sublow[] = {0xC0,' ',' ',' ',' ',' ',' ',' ',' ',0};
char and_lcd[] = {0x80,' ',0x42,0x41,0x4E,0x44,0x43,' ',' ',0};//BANDC
char andlow[] = {0xC0,' ',' ',' ',' ',' ',' ',' ',' ',0};
char not_lcd[] = {0x80,' ',0x4E,0x4F,0x54,0x28,0x42,0x29,' ',0};//NOT(B)
char notlow[] = {0xC0,' ',' ',' ',' ',' ',' ',' ',' ',0};
char mult_lcd[] = {0x80,' ',' ',0x42,0x2A,0x43,' ',' ',' ',0};//B*C
char multlow[] = {0xC0,' ',' ',' ',' ',' ',' ',' ',' ',0};
unsigned char SWA,SWB,B3,B2,B1,B0,C3,C2,C1,C0;

void main(void)
{
    Initialize_PIC();       //Initialize all settings required for general QwikFlash and LCD operation
	Print_To_LCD(Clear1);   //Clear the LCD line 1 to start your program
	Print_To_LCD(Clear2);   //Clear the LCD line 2 to start your program

 
    TRISC = 0b11111111;
    TRISB = 0b11111111;
    TRISD = 0b00000000;
    
    
    while(1)
    {
        SWA = PORTBbits.RB4;
        SWB = PORTBbits.RB5;
        
        B3 = PORTBbits.RB3;
        B2 = PORTBbits.RB2;
        B1 = PORTBbits.RB1;
        B0 = PORTBbits.RB0;
    
        C3 = PORTCbits.RC3;
        C2 = PORTCbits.RC2;
        C1 = PORTCbits.RC1;
        C0 = PORTCbits.RC0;
        
        Bdecimal = (B3*8) + (B2*4) + (B1*2) + (B0*1);
        Cdecimal = (C3*8) + (C2*4) + (C1*2) + (C0*1);
       
        
        if(SWA == 0 && SWB == 0)//B+C
        {
            //ADD();
            MULT();
            
            
        }
        else if(SWA == 0 && SWB == 1)//B-C
        {
            SUB();
           
        }
        else if(SWA == 1 && SWB == 0)//B AND C
        {
            AND();
            
        }
        else// NOT(B)
        {
            NOT();
            
        }
    }
	// Your personal PORT/TRIS/ADCON/etc settings or configurations can go here 
	// Or make your own function and call it
            
    // You can use the two delay functions defined in pic18.h (automatically included from xc.h):
    //      __delay_us(x);  // to delay in microsecond increments
    //                      // x is an unsigned long (0 - 4294967295)
    //
    //      __delay_ms(x);  // to delay in millisecond increments
    //                      // x is an unsigned long (0 - 4294967295)
    
    //Main routine, forever in an infinite while(true) loop
    
}
void ADD(void)
{
    sum = Bdecimal + Cdecimal;
            
    tens = sum / 10;    //Get tens place
    rem = sum % 10;     //Get ones place
            
    tens += 48;         //ascii conversion
    rem += 48;
            
    addlow[3] = 0x2B;   //LCD placement
    addlow[4] = tens;
    addlow[5] = rem;
         
    Print_To_LCD(add);  //print to LCD
    Print_To_LCD(addlow);
    
}
void SUB(void)
{
    minus = Bdecimal - Cdecimal;    //SUB operation
    neg = minus*-1;                 //Takes into account negative result
            
    tens = minus / 10;              //Separate tens and ones
    rem = minus % 10;
            
    tens_n = neg / 10;
    rem_n = neg % 10;
            
    tens_n += 48;
    rem_n += 48;
            
    tens += 48;
    rem += 48;
            
    if(minus < 0)   //if negative print out minus sign
    {
                
        sublow[3] = 0x2D;
        sublow[4] = tens_n;
        sublow[5] = rem_n;
    }
    else
    {
        sublow[3] = 0x2B;
        sublow[4] = tens;
        sublow[5] = rem;
    }
            
    Print_To_LCD(sub);
    Print_To_LCD(sublow);
}
void AND(void)
{
    B3 = B3 & C3;   //AND each bit
    B2 = B2 & C2;
    B1 = B1 & C1;
    B0 = B0 & C0;
            
    B3 += 48;       //Convert to ascii
    B2 += 48;
    B1 += 48;
    B0 += 48;
            
    addlow[2] = B3; //Assign to LCD
    addlow[3] = B2; 
    addlow[4] = B1;
    addlow[5] = B0;
            
    Print_To_LCD(and_lcd);  //Print to LCD
    Print_To_LCD(addlow);
    
}
void NOT(void)
{
    B3 ^= 1;    //Flip each bit
    B2 ^= 1;
    B1 ^= 1;
    B0 ^= 1;
            
    B3 += 48;   //Convert to ascii
    B2 += 48;
    B1 += 48;
    B0 += 48;
            
    notlow[2] = B3; //Assign to LCD
    notlow[3] = B2; 
    notlow[4] = B1;
    notlow[5] = B0;
            
    Print_To_LCD(not_lcd);  //Print to LCD
    Print_To_LCD(notlow);   
}
void MULT(void)
{
    mult = Bdecimal * Cdecimal;
            
    hund = mult / 100;
    mult -= hund*100;
    tens = mult / 10;
    rem = mult % 10;
            
    hund += 48;
    tens += 48;
    rem += 48;
            
    multlow[2] = 0x2B;
    multlow[3] = hund;
    multlow[4] = tens;
    multlow[5] = rem;
            
    Print_To_LCD(mult_lcd);
    Print_To_LCD(multlow);
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


