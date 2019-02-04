/*******************************
 * Name: William Wallace
 * Student ID#: 1001139276
 * CSE 3442/5442 - Embedded Systems 1
 * Lab 7 (ABET): Building a PIC18F4520 Standalone Alarm System with EUSART Communication 
 ********************************/

 // CONFIG1H
#pragma config OSC = HS    // Oscillator Selection bits (Internal oscillator block, port function on RA6 and RA7)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF  // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 3         // Brown Out Reset Voltage bits (Minimum setting)

// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config CCP2MX = PORTC   // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = OFF     // PORTB A/D Enable bit (PORTB<4:0> pins are configured as digital I/O on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config MCLRE = ON      // MCLR Pin Enable bit (RE3 input pin enabled; MCLR disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-001FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (002000-003FFFh) not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (004000-005FFFh) not code-protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (006000-007FFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000800-001FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (002000-003FFFh) not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (004000-005FFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (006000-007FFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-001FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (002000-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (004000-005FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (006000-007FFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block (000000-0007FFh) not protected from table reads executed in other blocks)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.
 
#include <xc.h> 
#include <stdio.h>
#include <string.h>
#define _XTAL_FREQ 20000000 //External Crystal of 20 MHz

//Variables
int choice,check = 0,test;
long adLow = 0,adHigh = 0;
long voltage, decimal;
char pass[4];
char pass_check;
int triggered = 0;
//EEPROM saved Data
int alarm_set;
int temp_set;
int temp_threshold;
char temp_threshold_read[2];
char temp_threshold_input[2];
char saved_pass[4];
char temp;
int system_active = 0; //Turn to 1 after initial password saved
int keypad_active = 0; //Default keypad is OFF
//EEPROM addresses
char pass_loc = 0x00;
char alarm_loc = 0x10;
char temp_loc = 0x20;
char threshold_loc = 0x30; 
char system_active_loc = 0x40;
char keypad_active_loc = 0x50;
//Functions
void password_options();
void alarm_options();
void temp_options();
void input_options();
void Clear();
void password();
void password_change();
int keypad();
void ADC();

void putch(unsigned char data)//print
{
    while(!PIR1bits.TXIF)
        continue;
    TXREG = data;
}
void Clear()//Clear terminal window
{
    printf("\033[2J");      
    printf("\033[0;0H");
}
void write_eeprom(unsigned char address, unsigned char data)//Function to write to EEPROM
{
    EEADR = address;
    EEDATA = data;
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.WREN = 1;
    INTCONbits.GIE = 0;
    EECON2 = 0x55;
    EECON2 = 0x0aa;
    EECON1bits.WR = 1;
    __delay_ms(30);
    INTCONbits.GIE = 1;
    EECON1bits.WREN = 0;
}

void interrupt alarm(void)//high priority interrupt for motion sensor
{
    Clear();
    printf("\n\r\n\r\n\rALARM TRIGGERED\n\r\n\r\n\r");
    printf("\n\rENTER PASSWORD TO CONTINUE\n\r");
    PORTBbits.RB2 = 1;       //Turn on Red LED
    triggered = 1;
    password();              //Enter Pass to reset alarm
    
    if((voltage>=temp_threshold) && temp_set == 1){
        printf("\n\rENTER PASSWORD TO RESET TEMP ALARM\n\r");
    }
    else{
        printf("\n\rPress Any Button to return to Menu\n\r");
    }
    INTCON3bits.INT1IF = 0;  //Reset flag to zero
    PORTBbits.RB2 = 0;       //Turn off LED
    
}
void interrupt low_priority temp_sensor(void)//low priority interrupt for temp sensor
{
    if(INTCONbits.TMR0IF == 1) //Check if both TMR0 interrupted
    {
        PORTBbits.RB5 = 1; //Turn on Yellow every ADC cycle
        __delay_ms(10);
        ADCON0bits.GO = 1; 
        __delay_ms(10);
        if(PIR1bits.ADIF == 1) //Check if ADC interrupted (flag set)
        {
            ADC();//Do ACD calculation
        }
        temp_set = eeprom_read(temp_loc);
        temp_threshold = temp_threshold_read[0]*10 + temp_threshold_read[1];

        if((voltage>=temp_threshold) && temp_set == 1)//check if temp_threshold was surpassed  
        {
            Clear();
            triggered = 1;
            PORTBbits.RB5 = 1;   //Leave on Yellow LED while alarm triggered
            printf("\n\r\n\r\n\rTEMP THRESHOLD OF %d%d EXCEEDED\n\r\n\r\n\r",temp_threshold_read[0],temp_threshold_read[1]);
            printf("\n\rENTER PASSWORD TO CONTINUE\n\r");
            T0CONbits.TMR0ON = 0;//Turn Timer off
            //voltage = 0;         //Reset Temp Value
            password();          //Enter Pass to reset alarm
            printf("\n\rDo you want to change the temp threshold?\n\r1)Yes\n\rANY)No");
            if(PORTBbits.RB4 == 1)
            {
                choice = keypad();
                PORTBbits.RB4 = 0;
                __delay_ms(250);
                PORTBbits.RB4 = 1;
            }
            else
            {
                while(PIR1bits.RCIF == 0);
                choice = RCREG;
            }
            
            if(choice == 49){//Change Temp Threshold if 1 is entered
                __delay_ms(250);
                printf("\n\rEnter new Temperature Threshold: ");
                threshold_loc = 0x30;
                if(PORTBbits.RB4 == 1)//If Keypad input is ON
                {
                    for(int i = 0;i < 2;i++){
                        temp_threshold_input[i] = keypad();
                        temp_threshold_input[i] -= 48; //Get actual number
                        if(temp_threshold_input[i] < 0 || temp_threshold_input[i] > 9)
                        {
                            Clear();
                            printf("\n\rONLY 0-9 CAN BE USED FOR TEMP\n\r\n\r");
                            __delay_ms(1000);
                            return;
                        }
                        write_eeprom(threshold_loc,temp_threshold_input[i]);
                        threshold_loc += 1;
                        
                        printf("%d", temp_threshold_input[i]);
                        PORTBbits.RB4 = 0;
                        __delay_ms(500);
                        PORTBbits.RB4 = 1;
                    }
                }
                else
                {
                    for(int i = 0;i < 2;i++){
                        while(PIR1bits.RCIF == 0);
                        temp_threshold_input[i] = RCREG;
                        temp_threshold_input[i] -= 48; //Get actual number
                        
                        if(temp_threshold_input[i] < 0 || temp_threshold_input[i] > 9)
                        {
                            Clear();
                            printf("\n\rONLY 0-9 CAN BE USED IN PASSWORD\n\r\n\r");
                            __delay_ms(1000);
                            return;
                        }
                        printf("%d", temp_threshold_input[i]);
                        write_eeprom(threshold_loc,temp_threshold_input[i]);
                        threshold_loc += 1;
                    }
                }
                temp_threshold_input[0] = temp_threshold_input[0]*10;
                temp_threshold = temp_threshold_input[0]+temp_threshold_input[1];
                printf("\n\r%d\n\r", temp_threshold);
                printf("\n\rPress Any Button to return to Menu\n\r");
            }
            else{//Return to Menu
                printf("\n\rPress Any Button to return to Menu\n\r");
            }
            T0CONbits.TMR0ON = 1;//Turn Timer Back on
        }
        
    }
    
    PIR1bits.ADIF = 0;     //Reset ADC flag
    INTCONbits.TMR0IF = 0; //Reset TMR0 flag
    PORTBbits.RB5 = 0;     //Turn off Yellow LED
}
void ADC()//ADC Logic
{
    adLow = ADRESL;         //Store in registers
    adHigh = ADRESH;
    adHigh = adHigh << 8;   //Get last 2 bits from adHigh
    decimal = adLow + adHigh;
    voltage = decimal*500;  //10mV/C Scaling
    voltage = voltage/1023; //10bit ADC
    voltage = voltage - 50; //0.5V Offset
    
    voltage = voltage*1.8;  //Celsius to F
    voltage += 32;
}
void main()
{
    //PRINTING
    TXSTAbits.TXEN = 1;
    RCSTAbits.SPEN = 1;
    
    TRISB = 0b00000010; //PORTB All output
    
    PORTBbits.RB2 = 0; //RED
    PORTBbits.RB3 = 0; //GREEN
    PORTBbits.RB4 = 0; //BLUE
    PORTBbits.RB5 = 0; //YELLOW
    
    //COMMUNICATION
    TRISCbits.RC7 = 1;  //RX is input
    TRISCbits.RC6 = 0;  //TX is output
    SPBRG = 31;         //Low Speed: 9,600 BAUD for Fosc = 20MHz
    TXSTAbits.SYNC = 0; //Asynchronous mode
    TXSTAbits.BRGH = 0; //Low speed BAUD rate
    RCSTAbits.RX9 = 0;  //8-bit transmission
    RCSTAbits.SPEN = 1; //Serial port enabled (RX and TX active)
    RCSTAbits.CREN = 1; //Enable continuous receiver (ON)
    
    //INTERRUPTS
    INTCONbits.GIE = 0;  //Disable all unmasked interrupts
    INTCONbits.PEIE = 0; //Disable all unmasked peripheral interrupts
    RCONbits.IPEN = 1;   //Enable priority levels on interrupts
    //ADC INTERRUPT
    PIR1bits.ADIF = 0;       //Clear ADC complete bit
    PIE1bits.ADIE = 1;       //Enable ADC interrupt
    IPR1bits.ADIP = 0;       //ADC low priority
    //Motion Alarm INTERRUPT
    INTCON3bits.INT1IP = 1;  //Set Alarm as high priority
    INTCON3bits.INT1IE = 0;  //Disable INT1 external interrupt -DEFAULT- (Alarm)//ENABLE WHEN SETTING IS SET
    INTCON3bits.INT1IF = 0;  //Initialize INT1 flag to zero
    //ADC
    ADCON0bits.CHS3 = 0; //Channel 0 (AN0)
    ADCON0bits.CHS2 = 0;
    ADCON0bits.CHS1 = 0;
    ADCON0bits.CHS0 = 0;
    
    ADCON0bits.GO = 0;    //Conversion status OFF
    ADCON0bits.ADON = 1;  //ADC ON
    ADCON1bits.VCFG1 = 0; //Use Vss
    ADCON1bits.VCFG0 = 0; //Use Vdd
    
    ADCON1bits.PCFG3 = 1; //Use all Digital except AN0 (MESSED UP ALARM)
    ADCON1bits.PCFG2 = 1;
    ADCON1bits.PCFG1 = 1;
    ADCON1bits.PCFG0 = 0;
    
    ADCON2bits.ADFM = 1;  //Right justified
    
    ADCON2bits.ACQT2 = 1; //8 TAD
    ADCON2bits.ACQT2 = 0;
    ADCON2bits.ACQT2 = 0;
    
    ADCON2bits.ADCS2 = 1; //Fosc/4
    ADCON2bits.ADCS1 = 0;
    ADCON2bits.ADCS0 = 0;
    //TMR0 SETTINGS
    T0CONbits.TMR0ON = 0; //Stop TMR0
    T0CONbits.T08BIT = 0; //16bit
    T0CONbits.T0CS = 0;   //Internal Clock
    T0CONbits.PSA = 0;    //Pre-scaler assigned
    
    T0CONbits.T0PS2 = 1;  //Pre-scaler 1:64
    T0CONbits.T0PS1 = 0;
    T0CONbits.T0PS0 = 1;
    //TMR0 INTERRUPT 
    INTCONbits.TMR0IE = 1;  //Enable TMR0 overflow interrupt
    INTCONbits.TMR0IF = 0;  //Initialize flag to zero  
    INTCON2bits.TMR0IP = 0; //TMR0 low priority
    
    INTCONbits.GIE = 1;  //Enable all unmasked interrupts
    INTCONbits.PEIE = 1; //Enable all unmasked peripheral interrupts 
    //Load pre-scalar
    TMR0H = 0x67;
    TMR0L = 0x6A;
    
    Clear();
    keypad_active = eeprom_read(keypad_active_loc);
    system_active = eeprom_read(system_active_loc);
    if(keypad_active == 1 || keypad_active == 2){//Set LED based on saved input status data
        PORTBbits.RB4 = 1; //Turn ON Blue LED
    }
    else{
        PORTBbits.RB4 = 0; //Leave Blue LED OFF
    }
    if(system_active == 1)//Check if system_active == 1 (not first time powered ON)
    {
        if(keypad_active == 1){     //keypad ON (input option 1)
            printf("INPUT: Keypad\n\r\n\r");   
            password();
        }
        else if(keypad_active == 2){//Both keypad and keyboard active (input option 3)
            printf("INPUT: Keypad and Keyboard\n\r\n\r");
            password();
        }
        else{                       //keypad OFF (input option 2)
            printf("INPUT: Keyboard\n\r\n\r");
            password();
        }
        threshold_loc = 0x30;
        for(int i = 0;i < 2;i++)
        {
            temp_threshold_read[i] = eeprom_read(threshold_loc);
            threshold_loc += 1;
        }
        temp_threshold = temp_threshold_read[0]*10+temp_threshold_read[1];
    }
    else//Set initial Pass
    {
        //INITIAL PASSWORD - only set initial pass if first time powered up
        printf("SET PASSWORD:\t");
        for(int i = 0;i < 4;i++){
            while(PIR1bits.RCIF == 0);
            saved_pass[i] = RCREG;
            
            printf("*");
            
            if(saved_pass[i] < '0' || saved_pass[i] > '9')
            {
                printf("\n\rONLY 0-9 CAN BE USED IN PASSWORD\n\r");
                __delay_ms(1000);
                return;
            }
        } 
    
        //SAVE PASSWORD TO EEPROM (Saved ascii value)
        for(int i=0;i<4;i++)
        {
            write_eeprom(pass_loc,saved_pass[i]);//saved 1 as 48
            pass_loc+=1;
        }
    
        printf("\n\rPassword SET\n\r\n\r");
        //The Initial Password was set
        write_eeprom(system_active_loc,0x01);
        //Set Default/Initial Temp Threshold
        write_eeprom(threshold_loc,7);
        write_eeprom(threshold_loc+1,5);
        //temp_threshold = 72;
        write_eeprom(alarm_loc,0);
        write_eeprom(temp_loc,0);
    }

        
	while(1)//Main Loop
	{
        PORTBbits.RB3 = 1; //GREEN (LED on when system is running)
        T0CONbits.TMR0ON = 1; //Turn on TMR0
        
        while(1)//Main Menu
        {   
            alarm_set = eeprom_read(alarm_loc);
            temp_set = eeprom_read(temp_loc);
            keypad_active = eeprom_read(keypad_active_loc);
            threshold_loc = 0x30;
            for(int i = 0; i < 2;i++)
            {
                temp_threshold_read[i] = eeprom_read(threshold_loc);
                threshold_loc += 1;
            }
            if(alarm_set != 1){
                printf("Alarm is OFF\n\r");
            }
            else
            {
                printf("Alarm is ON\n\r");
                INTCON3bits.INT1IE = 1; //Alarm is ON
            }
            if(temp_set != 1){
                printf("Temp Alarm is OFF\n\r");
            }
            else
            {
                printf("Temp Alarm is ON\n\r");
            }
            if(keypad_active == 1)
            {
                printf("INPUT: KEYPAD\n\r");
            }
            else if(keypad_active == 2)
            {
                printf("INPUT: KEYPAD and KEYBOARD\n\r");
            }
            else
            {
                printf("INPUT: KEYBOARD\n\r");
            }
            printf(
            "Temperature Threshold: %d%d\n\r"
            "Current Temp [F]: %ld"
            "\n\r\n\r"
            "Alarm System"
            "\n\r-----------------\n\r"
            "1)Password\n\r"
            "2)PIR Sensor\n\r"
            "3)Temp Sensor\n\r"
            "4)Input Method\n\r"
            "----------------\n\r",temp_threshold_read[0],temp_threshold_read[1],voltage
            );
            
            //Get user input for menu choice
            if(PORTBbits.RB4 == 1){                
                choice = keypad();    
                PORTBbits.RB4 = 0;
                __delay_ms(250);
                PORTBbits.RB4 = 1;
            }
            else{
                while(PIR1bits.RCIF == 0);
                choice = RCREG;
            }
            
            if(triggered == 1)
            {
                printf("\n\rReturning to Menu...\n\r");
                triggered = 0;
                Clear();
                break;
            }
            switch(choice){
                case '1'://Password Options
                    password_options();
                    break;
                case '2'://PIR Sensor Options
                    alarm_options();
                    break;
                case '3'://Temp Sensor Options
                    temp_options();
                    break;
                case '4'://Input Options
                    input_options();
                    break;
                default://Invalid Input
                    printf("\nERROR INVALID INPUT\n\r");
                    __delay_ms(500);
                    Clear();
                    break;
            }       
        }
	} //end of while(1)
} //end of void main()
void password_change()//Logic for changing password
{
    while(1)
    {
        Clear();
        __delay_ms(250);
        printf("Enter Current Password: ");
        if(PORTBbits.RB4 == 1)//If Keypad is active
        {
            for(int i = 0;i < 4;i++){
                pass[i] = keypad();
                printf("*");
                PORTBbits.RB4 = 0;
                __delay_ms(500);
                PORTBbits.RB4 = 1;
            }
        }
        else
        {
            for(int i = 0;i < 4;i++){
                while(PIR1bits.RCIF == 0);
                pass[i] = RCREG;
                printf("*");
                
            }
        }
        
        //Password Verification
        pass_loc = 0x00;
        for(int i = 0;i < 4;i++)
        {
            pass_check = eeprom_read(pass_loc);
            pass_loc+=1;
            if(pass_check != pass[i])
            {
                check = 1;
                break;
            }
            else
            {
                check = 0;
            }
            
        }
        
        if(check == 0)
        {
            Clear();
            __delay_ms(250);
            printf("Enter New Password: ");
            if(PORTBbits.RB4 == 1)//If Keypad is active
            {
                for(int i = 0;i < 4;i++){
                    saved_pass[i] = keypad();
                    printf("*");
                    PORTBbits.RB4 = 0;
                    __delay_ms(500);
                    PORTBbits.RB4 = 1;
                    if(saved_pass[i] < '0' || saved_pass[i] > '9')
                    {
                        Clear();
                        printf("\n\rONLY 0-9 CAN BE USED IN PASSWORD\n\r");
                        printf("Returning to Password Menu...\n\r");
                        __delay_ms(1000);
                        return;
                    }
                }
            }
            else
            {
                for(int i = 0;i < 4;i++){
                    while(PIR1bits.RCIF == 0);
                    saved_pass[i] = RCREG;
                    printf("*");
                    if(saved_pass[i] < '0' || saved_pass[i] > '9')
                    {
                        Clear();
                        printf("\n\rONLY 0-9 CAN BE USED IN PASSWORD\n\r");
                        printf("Returning to Password Menu...\n\r");
                        __delay_ms(1000);
                        return;
                    }
                }
            }
            pass_loc = 0x00;    //Set pass address back to 0x00
            for(int i=0;i<4;i++)//Write new password to EEPROM
            {
                write_eeprom(pass_loc,saved_pass[i]);//saved 1 as 48
                pass_loc += 1;
            }
            printf("\n\r\n\rPassword Changed\n\r\n\r");
            break;
        }
        else
        {
            printf("\n\rPassword Invalid. Going back to Password Menu....\n\r");
            __delay_ms(1000);
            Clear();
            break;
        }
    }
}
void password()//Logic for Entering/Verifying Password
{
    while(1)
        {
            __delay_ms(250);
            printf("PASSWORD:\t");
            if(PORTBbits.RB4 == 1)//If Keypad is active
            {
                for(int i = 0;i < 4;i++){
                    pass[i] = keypad();
                    printf("*");
                    PORTBbits.RB4 = 0;
                    __delay_ms(500);
                    PORTBbits.RB4 = 1;
                }
            }
            else
            {
                for(int i = 0;i < 4;i++){
                    while(PIR1bits.RCIF == 0);
                    pass[i] = RCREG;
                    printf("*");                
                }
            }
            pass_loc = 0x00;
            for(int i = 0;i < 4;i++)
            {
                pass_check = eeprom_read(pass_loc);
                pass_loc+=1;
                if(pass_check != pass[i])
                {
                    check = 1;
                    break;
                }
                else
                {
                    check = 0;
                }
            
            }
            
            if(check == 0)
            {
                printf("\n\rPASSWORD VALID\n\r\n\r");
                return;
            }
            else
            {
                printf("\n\rxxxxxxxINVALIDxxxxxxx\n\r\n\r");
            }
        }
}
void password_options()//Password Menu
{
    Clear();
    while(1)
    {
        alarm_set = eeprom_read(alarm_loc);
        temp_set = eeprom_read(temp_loc);
        keypad_active = eeprom_read(keypad_active_loc);
        threshold_loc = 0x30;
        for(int i = 0; i < 2;i++)
        {
            temp_threshold_read[i] = eeprom_read(threshold_loc);
            threshold_loc += 1;
        }
        if(alarm_set != 1){
            printf("Alarm is OFF\n\r");
        }
        else
        {
            printf("Alarm is ON\n\r");
        }
        if(temp_set != 1){
            printf("Temp Alarm is OFF\n\r");
        }
        else
        {
            printf("Temp Alarm is ON\n\r");
        }
        if(keypad_active == 1)
        {
            printf("INPUT: KEYPAD\n\r");
        }
        else if(keypad_active == 2)
        {
            printf("INPUT: KEYPAD and KEYBOARD\n\r");
        }
        else
        {
            printf("INPUT: KEYBOARD\n\r");
        }
        printf(
               "Temperature Threshold: %d%d\n\r"
               "Current Temp [F]: %ld"
               "\n\r\n\r"
               "PASSWORD OPTIONS\n\r"
               "1)Change Password\n\r"
               "2) Return to Menu\n\r",temp_threshold_read[0],temp_threshold_read[1], voltage);
        
        if(PORTBbits.RB4 == 1){                
            choice = keypad();   
            PORTBbits.RB4 = 0;
            __delay_ms(250);
            PORTBbits.RB4 = 1;
        }
        else{
            while(PIR1bits.RCIF == 0);
            choice = RCREG;
        }
        
        if(triggered == 1)
        {
            printf("\n\rReturning to Menu...\n\r");
            triggered = 0;
            Clear();
            break;
        }
        switch(choice){
            case '1'://Change Password
                printf("Changed Password\n\r\n\r");
                password_change();
                break;
            case '2'://Return to Menu
                Clear();
                return;
            default://Invalid Input
                printf("\nERROR INVALID INPUT\n\r");
                __delay_ms(500);
                Clear();
                break;
        }
    }
}
void alarm_options()//Alarm Menu
{
    Clear();
    while(1)
    {
        alarm_set = eeprom_read(alarm_loc);
        temp_set = eeprom_read(temp_loc);
        keypad_active = eeprom_read(keypad_active_loc);
        threshold_loc = 0x30;
        for(int i = 0; i < 2;i++)
        {
            temp_threshold_read[i] = eeprom_read(threshold_loc);
            threshold_loc += 1;
        }
        if(alarm_set != 1){
                printf("Alarm is OFF\n\r");
        }
        else
        {
            printf("Alarm is ON\n\r");
        }
        if(temp_set != 1){
            printf("Temp Alarm is OFF\n\r");
        }
        else
        {
            printf("Temp Alarm is ON\n\r");
        }
        if(keypad_active == 1)
        {
            printf("INPUT: KEYPAD\n\r");
        }
        else if(keypad_active == 2)
        {
            printf("INPUT: KEYPAD and KEYBOARD\n\r");
        }
        else
        {
            printf("INPUT: KEYBOARD\n\r");
        }
        printf("Temperature Threshold: %d%d\n\r"
               "Current Temp [F]: %ld"
               "\n\r\n\r"
               "PIR Sensor OPTIONS\n\r"
               "1) Enable/Disable Alarm\n\r"
               "2) Return to Menu\n\r",temp_threshold_read[0],temp_threshold_read[1],voltage);

        if(PORTBbits.RB4 == 1){                
            choice = keypad();
            PORTBbits.RB4 = 0;
            __delay_ms(250);
            PORTBbits.RB4 = 1;
        }
        else{
            while(PIR1bits.RCIF == 0);
            choice = RCREG;
        }
        
        if(triggered == 1)
        {
            printf("\n\rReturning to Menu...\n\r");
            triggered = 0;
            Clear();
            break;
        }
        switch(choice){
            case '1'://Change Password
                if(INTCON3bits.INT1IE == 0){
                    INTCON3bits.INT1IE = 1; //Enable Alarm
                    printf("Alarm Enabled\n\n\r\r");
                    //alarm_set = 1;
                    write_eeprom(alarm_loc,0x01);
                }
                else
                {
                    INTCON3bits.INT1IE = 0; //Disable Alarm
                    printf("Alarm Disabled\n\n\r\r");
                    //alarm_set = 0;
                    write_eeprom(alarm_loc,0x00);
                }
                __delay_ms(500);
                Clear();
                break;
            case '2'://Return to Menu
                Clear();
                return;
            default://Invalid Input
                printf("\nERROR INVALID INPUT\n\r");
                __delay_ms(500);
                Clear();
                break;
        }
    }
}
void temp_options()//Menu for Temperature Sensor
{
    Clear();
    while(1)
    {
        alarm_set = eeprom_read(alarm_loc);
        temp_set = eeprom_read(temp_loc);
        keypad_active = eeprom_read(keypad_active_loc);
        threshold_loc = 0x30;
        for(int i = 0; i < 2;i++)
        {
            temp_threshold_read[i] = eeprom_read(threshold_loc);
            threshold_loc += 1;
        }
        if(alarm_set != 1){
            printf("Alarm is OFF\n\r");
        }
        else
        {
            printf("Alarm is ON\n\r");
        }
        if(temp_set != 1){
            printf("Temp Alarm is OFF\n\r");
        }
        else
        {
            printf("Temp Alarm is ON\n\r");
        }
        if(keypad_active == 1)
        {
            printf("INPUT: KEYPAD\n\r");
        }
        else if(keypad_active == 2)
        {
            printf("INPUT: KEYPAD and KEYBOARD\n\r");
        }
        else
        {
            printf("INPUT: KEYBOARD\n\r");
        }
        printf("Temperature Threshold: %d%d\n\r"
               "Current Temp [F]: %ld"
               "\n\r\n\r"
               "Temperature Sensor OPTIONS\n\r"
               "1) Enable/Disable Alarm\n\r"
               "2) Temperature Threshold\n\r"
               "3) Return to Menu\n\r",temp_threshold_read[0],temp_threshold_read[1],voltage);
        
        if(PORTBbits.RB4 == 1){                
            choice = keypad();     
            PORTBbits.RB4 = 0;
            __delay_ms(250);
            PORTBbits.RB4 = 1;
        }
        else{
            while(PIR1bits.RCIF == 0);
            choice = RCREG;
        }
        
        if(triggered == 1)
        {
            printf("\n\rReturning to Menu...\n\r");
            triggered = 0;
            Clear();
            break;
        }
        switch(choice){
            case '1'://Disable/Enable Temp Alarm
                temp_set = eeprom_read(temp_loc);
                if(temp_set != 1)
                {
                    //temp_set = 1;
                    printf("\n\rTemperature Alarm ON\n\r");
                    write_eeprom(temp_loc,0x01);
                }
                else
                {
                    //temp_set = 0;
                    printf("\n\rTemperature Alarm OFF\n\r");
                    write_eeprom(temp_loc,0x00);
                }
                __delay_ms(500);
                Clear();
                break;
            case '2'://Change temperature threshold 
                __delay_ms(250);
                printf("\n\rEnter new Temperature Threshold: ");
                threshold_loc = 0x30;
                if(PORTBbits.RB4 == 1)//If Keypad input is ON
                {
                    for(int i = 0;i < 2;i++){
                        temp_threshold_input[i] = keypad();
                        temp_threshold_input[i] -= 48; //Get actual number
                        if(temp_threshold_input[i] < 0 || temp_threshold_input[i] > 9)
                        {
                            Clear();
                            printf("\n\rONLY 0-9 CAN BE USED FOR TEMP\n\r\n\r");
                            __delay_ms(1000);
                            return;
                        }
                        printf("%d", temp_threshold_input[i]);
                        write_eeprom(threshold_loc,temp_threshold_input[i]);
                        threshold_loc += 1;
                        
                        PORTBbits.RB4 = 0;
                        __delay_ms(500);
                        PORTBbits.RB4 = 1;
                    }
                }
                else
                {
                    for(int i = 0;i < 2;i++){
                        while(PIR1bits.RCIF == 0);
                        temp_threshold_input[i] = RCREG;
                        temp_threshold_input[i] -= 48; //Get actual number
                     
                        if(temp_threshold_input[i] < 0 || temp_threshold_input[i] > 9)
                        {
                            Clear();
                            printf("\n\rONLY 0-9 CAN BE USED FOR TEMP\n\r\n\r");
                            __delay_ms(1000);
                            return;
                        }
                        printf("%d", temp_threshold_input[i]);
                        write_eeprom(threshold_loc,temp_threshold_input[i]);
                        threshold_loc += 1;
                    }
                }
                temp_threshold_input[0] = temp_threshold_input[0]*10;
                temp_threshold = temp_threshold_input[0]+temp_threshold_input[1];
                printf("\n\r\n\rTemperature Threshold set to %d F", temp_threshold);
                __delay_ms(2000);
                Clear();
                break;
            case '3'://Return to Menu
                Clear();
                return;
            default://Invalid Input
                printf("\nERROR INVALID INPUT\n\r");
                __delay_ms(1000);
                Clear();
                break;
        }
    }
}
void input_options()//Menu for Input
{
    Clear();
    while(1)
    {
        alarm_set = eeprom_read(alarm_loc);
        temp_set = eeprom_read(temp_loc);
        keypad_active = eeprom_read(keypad_active_loc);
        threshold_loc = 0x30;
        for(int i = 0; i < 2;i++)
        {
            temp_threshold_read[i] = eeprom_read(threshold_loc);
            threshold_loc += 1;
        }
        Clear();
        if(alarm_set != 1){
            printf("Alarm is OFF\n\r");
        }
        else
        {
            printf("Alarm is ON\n\r");
        }
        if(temp_set != 1){
            printf("Temp Alarm is OFF\n\r");
        }
        else
        {
            printf("Temp Alarm is ON\n\r");
        }
        if(keypad_active == 1)
        {
            printf("INPUT: KEYPAD\n\r");
        }
        else if(keypad_active == 2)
        {
            printf("INPUT: KEYPAD and KEYBOARD\n\r");
        }
        else
        {
            printf("INPUT: KEYBOARD\n\r");
        }
        printf("Temperature Threshold: %d%d\n\r"
               "Current Temp [F]: %ld"
               "\n\r\n\r"
               "INPUT OPTIONS:\n\r"
               "1) Use Keypad\n\r"
               "2) Use Keyboard\n\r"
               "3) Use BOTH\n\r"
               "4) Return to Menu\n\r",temp_threshold_read[0], temp_threshold_read[1],voltage);
        
        if(PORTBbits.RB4 == 1){                
            choice = keypad();  
            PORTBbits.RB4 = 0;
            __delay_ms(250);
            PORTBbits.RB4 = 1;
        }
        else{
            while(PIR1bits.RCIF == 0);
            choice = RCREG;
        }
        
        if(triggered == 1)
        {
            printf("\n\rReturning to Menu...\n\r");
            triggered = 0;
            Clear();
            break;
        }
        switch(choice){
            case '1'://Use Keypad
                printf("\n\rUsing Keypad as Input\n\r");
                TRISCbits.RC7 = 0;  //Disable Keyboard Input
                PORTBbits.RB4 = 1;  //Turn on Blue LED
                write_eeprom(keypad_active_loc,0x01);//Save Turn ON keypad
                __delay_ms(500);
                break;
            case '2'://Use Keyboard
                printf("\n\rUsing Keyboard as Input\n\r");
                TRISCbits.RC7 = 1;  //Enable Keyboard Input
                PORTBbits.RB4 = 0;  //Turn off Blue LED
                write_eeprom(keypad_active_loc,0x00);//Save Turn OFF keypad
                __delay_ms(500);
                break;
            case '3'://Use both keypad and keyboard
                printf("\n\rBOTH\n\r");
                TRISCbits.RC7 = 1;  //Enable Keyboard Input
                PORTBbits.RB4 = 1;  //Turn on Blue LED
                write_eeprom(keypad_active_loc,0x02);//Save BOTH Keypad and Keyboard ON
                __delay_ms(500);
                break;
            case '4'://Return to main menu
                Clear();
                return;
            default://Invalid Input
                printf("\nERROR INVALID INPUT\n\r");
                __delay_ms(500);
                Clear();
                break;
        }
    }
}
int keypad()//Keypad Logic
{
    while(1)
    {
        //SET TRISD SETTINGS
        TRISDbits.RD0 = 0;
        TRISDbits.RD1 = 0;
        TRISDbits.RD2 = 0;
        TRISDbits.RD3 = 0;
        TRISDbits.RD4 = 1;
        TRISDbits.RD5 = 1;
        TRISDbits.RD6 = 1;
        TRISDbits.RD7 = 1;
        
        //POLL Keypad Buttons
        PORTDbits.RD0 = 1; //Set Row 1 High
        PORTDbits.RD1 = 0;
        PORTDbits.RD2 = 0;
        PORTDbits.RD3 = 0;
        if(PORTDbits.RD4 == 1 && PORTDbits.RD5 == 0 && PORTDbits.RD6 == 0 && PORTDbits.RD7 == 0)
        {
            return '1';   
        }
        if(PORTDbits.RD4 == 0 && PORTDbits.RD5 == 1 && PORTDbits.RD6 == 0 && PORTDbits.RD7 == 0)
        {
            return '2';
        }
        if(PORTDbits.RD4 == 0 && PORTDbits.RD5 == 0 && PORTDbits.RD6 == 1 && PORTDbits.RD7 == 0)
        {
            return '3';
        }
        if(PORTDbits.RD4 == 0 && PORTDbits.RD5 == 0 && PORTDbits.RD6 == 0 && PORTDbits.RD7 == 1)
        {
            return 'A';  
        }
        
        PORTDbits.RD0 = 0;
        PORTDbits.RD1 = 1; //Set Row 2 High
        PORTDbits.RD2 = 0;
        PORTDbits.RD3 = 0;
        if(PORTDbits.RD4 == 1 && PORTDbits.RD5 == 0 && PORTDbits.RD6 == 0 && PORTDbits.RD7 == 0)
        {
            return '4';   
        }
        if(PORTDbits.RD4 == 0 && PORTDbits.RD5 == 1 && PORTDbits.RD6 == 0 && PORTDbits.RD7 == 0)
        {
            return '5';
        }
        if(PORTDbits.RD4 == 0 && PORTDbits.RD5 == 0 && PORTDbits.RD6 == 1 && PORTDbits.RD7 == 0)
        {
            return '6';
        }
        if(PORTDbits.RD4 == 0 && PORTDbits.RD5 == 0 && PORTDbits.RD6 == 0 && PORTDbits.RD7 == 1)
        {
            return 'B';  
        }
        
        PORTDbits.RD0 = 0;
        PORTDbits.RD1 = 0;
        PORTDbits.RD2 = 1;//Set Row 3 High
        PORTDbits.RD3 = 0;
        if(PORTDbits.RD4 == 1 && PORTDbits.RD5 == 0 && PORTDbits.RD6 == 0 && PORTDbits.RD7 == 0)
        {
            return '7';   
        }
        if(PORTDbits.RD4 == 0 && PORTDbits.RD5 == 1 && PORTDbits.RD6 == 0 && PORTDbits.RD7 == 0)
        {
            return '8';
        }
        if(PORTDbits.RD4 == 0 && PORTDbits.RD5 == 0 && PORTDbits.RD6 == 1 && PORTDbits.RD7 == 0)
        {
            return '9';
        }
        if(PORTDbits.RD4 == 0 && PORTDbits.RD5 == 0 && PORTDbits.RD6 == 0 && PORTDbits.RD7 == 1)
        {
            return 'C';  
        }
        
        PORTDbits.RD0 = 0;
        PORTDbits.RD1 = 0;
        PORTDbits.RD2 = 0;
        PORTDbits.RD3 = 1;//Set Row 4 High
        if(PORTDbits.RD4 == 1 && PORTDbits.RD5 == 0 && PORTDbits.RD6 == 0 && PORTDbits.RD7 == 0)
        {
            return '*';   
        }
        if(PORTDbits.RD4 == 0 && PORTDbits.RD5 == 1 && PORTDbits.RD6 == 0 && PORTDbits.RD7 == 0)
        {
            return '0';
        }
        if(PORTDbits.RD4 == 0 && PORTDbits.RD5 == 0 && PORTDbits.RD6 == 1 && PORTDbits.RD7 == 0)
        {
            return '#';
        }
        if(PORTDbits.RD4 == 0 && PORTDbits.RD5 == 0 && PORTDbits.RD6 == 0 && PORTDbits.RD7 == 1)
        {
            return 'D';  
        }
    
    }
}
//Be sure to have a blank line at the end of your program
