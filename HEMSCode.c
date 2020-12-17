/*
 * Author: Harsh Dubey
 * Lab partner: Lin Zeng
 * Code: Tea kettle Project Updated
 * Created on Feb 2, 2018
*/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> 
#include <xc.h>
#include "newfile.h"

//Configurating bits
#define _XTAL_FREQ  8000000
#pragma config PBADEN = OFF//PORTB as digital Inputs
#pragma config FOSC = HSMP
#pragma config FCMEN = OFF
#pragma config WDTEN = OFF

#define HEATandCOOL PORTE  
#define HEATorCOOL PORTBbits.RB0 
#define ON PORTBbits.RB1

//0 = Heater and 1 = Cooler

int HEATRATE[25] = {1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6};
int COOLRATE[25] = {1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6};
int hour[25] = {30,11,30,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11};
//Change Inputs Here
int state = 11110;
int Current;
int TheH,TheL;
unsigned long int tempval; //Reserve For ADC number conversion
//Variables
int i;
int uart = 0;
//Temp Profile
int Desired[26] = {30,40,40,55,35,85,55,50,65,85,65,85,65,85,65,85,65,85,65,85,65,85,65,85};
int DesiredTemp[50];
int SetPoint[25] = {1,2,3,4,5,6,7,8,9,1,2,3,4,5,6,7,8,9,1,2,3,4,5,6};
int SetTime[25];
int SetTrigger = 0;
int HEMS[26];


//Function Prototype
void PRICEDISPLAY(int HEMS[26]);
void Break();
void ADC_READING();
void DECIDE(int HEMS[26]);
void LCDINIT();
void ESUARTINITILAIZE();
void interrput();

void Oscillator_Initialize()
{
    OSCCON = 0x30;
    OSCCON2 = 0x04;
    OSCTUNE = 0x00;
}

//Interrupt Service Routine
void interrupt ISR()
{
    if(INTCONbits.TMR0IF==1)
    {
    interrput();
    
    INTCONbits.TMR0IF = 0;
}   else
    {
        INTCONbits.TMR0IF = 0;
    return;
    }
}


void main()
{
    //Intializing LCD, So that message can be displayed
    LCDINIT();
    lcdCommand(0X01);
    
    //Set Time Calculation for different state
    for (int r = 0; r<24;r++)
    {
        if(Desired[r]<Desired[r+1])
        {
            SetTime[r] =(((((Desired[r+1])-Desired[r])+SetPoint[r])/HEATRATE[r])*60)/8.35;
        }
        if((Desired[r]>Desired[r+1]))
        {
            SetTime[r] = (((((Desired[r])-Desired[r+1])+SetPoint[r])/COOLRATE[r])*60)/8.35;
        }
    }
    
    //Start the state machine
    if(state = 11110&&ON)
    {
    
    lcdChar('W');
    lcdChar('A');
    lcdChar('I');
    lcdChar('T');
    lcdChar('I');
    lcdChar('N');
    lcdChar('G');
    lcdChar(' ');
    lcdChar('F');
    lcdChar('O');
    lcdChar('R');
    lcdChar(' ');
    
    lcdGoTo(0x40);
    lcdChar('E');
    lcdChar('N');
    lcdChar('E');
    lcdChar('R');
    lcdChar('G');
    lcdChar('Y');
    lcdChar(' ');
    lcdChar('P');
    lcdChar('R');
    lcdChar('I');
    lcdChar('C');
    lcdChar('E');
    lcdChar('S');
    lcdChar('.');
    lcdChar('.');
    lcdChar('.');
    state = 11111;
        
    }
    else if(!ON)
    {
        lcdCommand(0X01);
    }
     //ADC CONTROL Bits setting
	ANSA0=1;  // select RA0 as analog input 
    ADCON2 =0xAD;  // left justified, 12TAC
    ADCON1 =0x00;  //reference voltage vcc,vss
	ADCON0 =0x01;  //RA0 as analog input to ADC
    //ADC ends
    
    //I/O FOR THE TIMER
    TRISDbits.RD0 = 0;
    TRISDbits.RD1 = 0;
    TRISDbits.RD2 = 0;
    TRISDbits.RD3 = 0;
    TRISDbits.RD4 = 0;
    TRISDbits.RD5 = 0;
    PORTDbits.RD0 = 0;
    PORTDbits.RD1 = 0;
    PORTDbits.RD2 = 0;
    PORTDbits.RD3 = 0;
    PORTDbits.RD4 = 0;
    PORTDbits.RD5 = 0;
    //ADC
    TRISAbits.RA0 = 1;
    
    //LOAD
    TRISEbits.RE1 = 0;
    TRISEbits.RE0 = 0;
    TRISBbits.RB0 = 1;
    HEATandCOOL = 0x00;
    
    //ON/RESET
    TRISBbits.RB1 = 1;
    
    //INTERRPUT CONTROL
    INTCONbits.GIE = 1;
    INTCONbits.TMR0IE = 1;
    
    //INTERRUPT FLAG
    INTCONbits.TMR0IF = 0;
    
    //TIMER SETTINGS
    TMR0H = 0X00;
    TMR0L = 0X00;
    T0CON = 0x07;
    
    //Break Desired Temp for display
    Break();
    
    //Calculate SetTime
    
    
    //Start the State Machine
    if(ON)
    {
    if(state == 11111&&ON)
    {
        //Initialize ESUART and overwrite LCD
    ESUARTINITILAIZE();
    RCSTA1bits.CREN=1;
    while(uart!=24&&ON)
    {while(PIR1bits.RC1IF==0);
    HEMS[uart] = RCREG1;
    PRICEDISPLAY(HEMS);
    uart++;
    if(uart==24)
    {
    state = 00001;
    RCSTA1bits.CREN=0;
    }
    }
        }
    if(state == 00001&&ON)//1
    {
    PORTD = 0XFF;
    //Re-initialize LCD and overwrite ESUART
    LCDINIT();
    HEATandCOOL = 0x0;
    SetTrigger = 0;
    i=0;
    lcdCommand(0X01);
    T0CONbits.TMR0ON = 1;
    
        while(state == 00001&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('1');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[0]+48);
    lcdChar(DesiredTemp[1]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    
    if(state == 00010&&ON)//2
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=1;
        lcdCommand(0X01);
        while(state == 00010&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('2');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[2]+48);
    lcdChar(DesiredTemp[3]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    
    if(state == 00011&&ON)//3
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=2;
        lcdCommand(0X01);
        while(state == 00011&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
   
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('3');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[4]+48);
    lcdChar(DesiredTemp[5]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    
    if(state == 00100&&ON)//4
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=3;
        lcdCommand(0X01);
        while(state == 00100&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('4');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[6]+48);
    lcdChar(DesiredTemp[7]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    
    if(state == 00101&&ON)//5
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=4;
        lcdCommand(0X01);
        while(state == 00101&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('5');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[8]+48);
    lcdChar(DesiredTemp[9]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    
    if(state == 00110&&ON)//6
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=5;
        lcdCommand(0X01);
        while(state == 00110&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('6');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[10]+48);
    lcdChar(DesiredTemp[11]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
        
    }if(state == 00111&&ON)//7
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=6;
        lcdCommand(0X01);
        while(state == 00111&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('7');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[12]+48);
    lcdChar(DesiredTemp[13]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }if(state == 01000&&ON)//8
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=7;
        lcdCommand(0X01);
        while(state == 01000&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('8');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[14]+48);
    lcdChar(DesiredTemp[15]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }if(state == 01001&&ON)//9
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=8;
        lcdCommand(0X01);
        while(state == 01001&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('9');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[16]+48);
    lcdChar(DesiredTemp[17]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    if(state == 01010&&ON)//10
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=9;
        lcdCommand(0X01);
        while(state == 01010&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('1');
    lcdChar('0');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[18]+48);
    lcdChar(DesiredTemp[19]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    
    if(state == 01100&&ON)//11
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=10;
        lcdCommand(0X01);
        while(state == 01100&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('1');
    lcdChar('1');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[20]+48);
    lcdChar(DesiredTemp[21]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    
    if(state == 01110&&ON)//12
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=11;
        lcdCommand(0X01);
        while(state == 01110&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('1');
    lcdChar('2');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[22]+48);
    lcdChar(DesiredTemp[23]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    
    if(state == 01111&&ON)//13
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=12;
        lcdCommand(0X01);
        while(state == 01111&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('1');
    lcdChar('3');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[24]+48);
    lcdChar(DesiredTemp[25]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    
    if(state == 10000&&ON)//14
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=13;
        lcdCommand(0X01);
        while(state == 10000&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('1');
    lcdChar('4');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[26]+48);
    lcdChar(DesiredTemp[27]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    
    if(state == 10001&&ON)//15
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=14;
        lcdCommand(0X01);
        while(state == 10001&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('1');
    lcdChar('5');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[28]+48);
    lcdChar(DesiredTemp[29]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    
     if(state == 10010&&ON)//16
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=15;
        lcdCommand(0X01);
        while(state == 10010&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('1');
    lcdChar('6');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[30]+48);
    lcdChar(DesiredTemp[31]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    
    if(state == 10011&&ON)//17
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=16;
        lcdCommand(0X01);
        while(state == 10011&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('1');
    lcdChar('7');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[32]+48);
    lcdChar(DesiredTemp[33]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    if(state == 10100&&ON)//18
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=17;
        lcdCommand(0X01);
        while(state == 10100&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('1');
    lcdChar('8');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[34]+48);
    lcdChar(DesiredTemp[35]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    
    if(state == 10101&&ON)//19
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=18;
        lcdCommand(0X01);
        while(state == 10101&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('1');
    lcdChar('9');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[36]+48);
    lcdChar(DesiredTemp[37]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    if(state == 10110&&ON)//20
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=19;
        lcdCommand(0X01);
        while(state == 10110&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('2');
    lcdChar('0');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[38]+48);
    lcdChar(DesiredTemp[39]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    if(state == 10111&&ON)//21
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=20;
        lcdCommand(0X01);
        while(state == 10111&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('2');
    lcdChar('1');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[40]+48);
    lcdChar(DesiredTemp[41]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    if(state == 11000&&ON)//22
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=21;
        lcdCommand(0X01);
        while(state == 11000&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('2');
    lcdChar('2');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[42]+48);
    lcdChar(DesiredTemp[43]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    
    if(state == 11001&&ON)//23
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=22;
        lcdCommand(0X01);
        while(state == 11001&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('2');
    lcdChar('3');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[44]+48);
    lcdChar(DesiredTemp[45]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    
    if(state == 11010&&ON)//24
    {
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=23;
        lcdCommand(0X01);
        while(state == 11010&&ON)
    {
    
    ADC_READING();
    DECIDE(HEMS);
    
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar(' ');
    lcdChar('H');
    lcdChar('O');
    lcdChar('U');
    lcdChar('R');
    lcdChar(':');
    lcdChar('2');
    lcdChar('4');
    lcdChar(' ');
    lcdGoTo(0x40);
    lcdChar('D');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(DesiredTemp[46]+48);
    lcdChar(DesiredTemp[47]+48);
    lcdChar(' ');
    lcdChar('C');
    lcdChar('T');
    lcdChar(':');
    lcdChar(' ');
    lcdChar(TheH+48);
    lcdChar(TheL+48);
    lcdChar(' ');
    lcdChar(' ');
    }
    }
    }
    else if(!ON)
    {
        lcdCommand(0X01);
        state = 11111;
        HEATandCOOL = 0x0;
        SetTrigger = 0;
        i=0;
        uart = 0;
        
    }
         
}

void Break()
{
    //Break DT
    DesiredTemp[0] = Desired[0]/10;
    DesiredTemp[1] = Desired[0]%10;
    DesiredTemp[2] = Desired[1]/10;
    DesiredTemp[3] = Desired[1]%10;
    DesiredTemp[4] = Desired[2]/10;
    DesiredTemp[5] = Desired[2]%10;
    DesiredTemp[6] = Desired[3]/10;
    DesiredTemp[7] = Desired[3]%10;
    DesiredTemp[8] = Desired[4]/10;
    DesiredTemp[9] = Desired[4]%10;
    DesiredTemp[10] = Desired[5]/10;
    DesiredTemp[11] = Desired[5]%10;
    DesiredTemp[12] = Desired[6]/10;
    DesiredTemp[13] = Desired[6]%10;
    DesiredTemp[14] = Desired[7]/10;
    DesiredTemp[15] = Desired[7]%10;
    DesiredTemp[16] = Desired[8]/10;
    DesiredTemp[17] = Desired[8]%10;
    DesiredTemp[18] = Desired[9]/10;
    DesiredTemp[19] = Desired[9]%10;
    DesiredTemp[20] = Desired[10]/10;
    DesiredTemp[21] = Desired[10]%10;
    DesiredTemp[22] = Desired[11]/10;
    DesiredTemp[23] = Desired[11]%10;
    DesiredTemp[24] = Desired[12]/10;
    DesiredTemp[25] = Desired[12]%10;
    DesiredTemp[26] = Desired[13]/10;
    DesiredTemp[27] = Desired[13]%10;
    DesiredTemp[28] = Desired[14]/10;
    DesiredTemp[29] = Desired[14]%10;
    DesiredTemp[30] = Desired[15]/10;
    DesiredTemp[31] = Desired[15]%10;
    DesiredTemp[32] = Desired[16]/10;
    DesiredTemp[33] = Desired[16]%10;
    DesiredTemp[34] = Desired[17]/10;
    DesiredTemp[35] = Desired[17]%10;
    DesiredTemp[36] = Desired[18]/10;
    DesiredTemp[37] = Desired[18]%10;
    DesiredTemp[38] = Desired[19]/10;
    DesiredTemp[39] = Desired[19]%10;
    DesiredTemp[40] = Desired[20]/10;
    DesiredTemp[41] = Desired[20]%10;
    DesiredTemp[42] = Desired[21]/10;
    DesiredTemp[43] = Desired[21]%10;
    DesiredTemp[44] = Desired[22]/10;
    DesiredTemp[45] = Desired[22]%10;
    DesiredTemp[46] = Desired[23]/10;
    DesiredTemp[47] = Desired[23]%10;
    DesiredTemp[48] = Desired[24]/10;
    DesiredTemp[49] = Desired[24]%10;
    //ENDS
    return;
}

void ADC_READING()
{
    ADCON0bits.GO = 1;
    Current=ADRES;
    tempval=Current;
    Current = (Current*0.004887585533-0.00000000017141)*25.76398684-20.86850024;
    TheH = Current/10;
    TheL = Current%10;          //Decomposing in Decimal //val3=val2%10;//Since, C does a better job
    ADCON0bits.GO = 0;
    return;
}

void DECIDE(int HEMS[26])
{
    if(((HEMS[i])<(HEMS[i+1]))&&(Current<(Desired[i+1]+(SetPoint[i])))&&(HEATorCOOL == 0))
    {
    if(SetTrigger == 0)
    { 
    if(Current<Desired[i]-2)
    {
       if(HEATorCOOL == 0)
       {HEATandCOOL = 0x2;}
       else 
       {HEATandCOOL = 0x0;}
    }
    if (Current>Desired[i]+2)
    {
       if(HEATorCOOL == 0)
       {HEATandCOOL = 0x0;}
       else 
       {HEATandCOOL = 0x1;}
    }
    }
    
    else if(SetTrigger == 1)
    {
     HEATandCOOL = 0x02;
    }
    }
    else if (((HEMS[i]<(HEMS[i+1]))&&(Current>(Desired[i+1]-(SetPoint[i])))&&(HEATorCOOL == 1)))
    {
    if(SetTrigger == 0)
    { 
    if(Current<Desired[i]-2)
    {
       if(HEATorCOOL == 0)
       {HEATandCOOL = 0x2;}
       else 
       {HEATandCOOL = 0x0;}
    }
    if (Current>Desired[i]+2)
    {
       if(HEATorCOOL == 0)
       {HEATandCOOL = 0x0;}
       else 
       {HEATandCOOL = 0x1;}
    }
    }
    
    else if(SetTrigger == 1)
    {
     HEATandCOOL = 0x01;
    }
    }
    else
    { 
    if(Current<Desired[i]-2)
    {
       if(HEATorCOOL == 0)
       {HEATandCOOL = 0x2;}
       else 
       {HEATandCOOL = 0x0;}
    }
    if (Current>Desired[i]+2)
    {
       if(HEATorCOOL == 0)
       {HEATandCOOL = 0x0;}
       else 
       {HEATandCOOL = 0x1;}
    }
    }
    
}
void ESUARTINITILAIZE()
{
    TRISD = 0X00;
    PORTD = 0X00;
    TRISCbits.RC6=1;	
	ANSELCbits.ANSC6=0;
	TRISCbits.RC7=1;	
	ANSELCbits.ANSC7=0;
	TXSTA1bits.SYNC =0;
	TXSTA1bits.BRGH=0;
	TXSTA1bits.TX9=0;
	SPBRG1 =12;
	RCSTA1bits.SPEN=1;
	
}

void LCDINIT()
{
    //Needed for LCD
    LATC = 0x00;
    TRISC = 0xD7;
    ANSELC = 0xC4;//PORTC is used by the LCD//Do not use it anywhere else
    SPI1_Initialize();
    Oscillator_Initialize();
    lcdinit();
    //All six Lines of Codes
}

void PRICEDISPLAY(int HEMS[26])
{
    if(HEMS[uart]==48)
    {PORTD = 0X5F;}
    if(HEMS[uart]==49)
    {PORTD = 0X06;}
    if(HEMS[uart]==50)
    {PORTD = 0X3B;}
    if(HEMS[uart]==51)
    {PORTD = 0X2F;}
    if(HEMS[uart]==52)
    {PORTD = 0X66;}
    if(HEMS[uart]==53)
    {PORTD = 0X6D;}
    if(HEMS[uart]==54)
    {PORTD = 0X7D;}
    if(HEMS[uart]==55)
    {PORTD = 0X07;}
    if(HEMS[uart]==56)
    {PORTD = 0XFF;}
    if(HEMS[uart]==57)
    {PORTD = 0X6F;}
}

void interrput()
{
   if(state == 00001)//1
    {
    hour[0]--;
    if(hour[0]==0)
    {
    state = 00010;
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[0]==SetTime[0])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }else
    {
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
    
    if(state == 00010)//2
    {
    hour[1]--;
    if(hour[1]==0)
    {
    state = 00011;
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[1]==SetTime[1])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }else
    {
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
    
    if(state == 00011)//3
    {
    hour[2]--;
    if(hour[2]==0)
    {
    state = 00100;
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[2]==SetTime[2])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }else
    {
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
    
    if(state == 00100)//4
    {
    hour[3]--;
    if(hour[3]==0)
    {
    state = 00101;
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[3]==SetTime[3])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }else
    {
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
    
    if(state == 00101)//5
    {
    hour[4]--;
    if(hour[4]==0)
    {
    state = 00110;
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[4]==SetTime[4])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }else
    {
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
    
    if(state == 00110)//6
    {
    hour[5]--;
    if(hour[5]==0)
    {
    state = 00111;
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[5]==SetTime[5])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
    
    if(state == 00111)//7
    {
    hour[6]--;
    if(hour[6]==0)
    {
    state = 01000;
    
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[6]==SetTime[6])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
    if(state == 01000)//8
    {
    hour[7]--;
    if(hour[7]==0)
    {
    state = 01001;
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[7]==SetTime[7])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
   
   if(state == 01001)//9
    {
    hour[8]--;
    if(hour[8]==0)
    {
    state = 01010;
    
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[8]==SetTime[8])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
   
   if(state == 01010)//10
    {
    hour[9]--;
    if(hour[9]==0)
    {
    state = 01100;
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[9]==SetTime[9])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
   
   if(state == 01100)//11
    {
    hour[10]--;
    if(hour[10]==0)
    {
    state = 01110;
    
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[10]==SetTime[10])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }
   }
    
    if(state == 01110)//12
    {
    hour[11]--;
    if(hour[11]==0)
    {
    state = 01111;
    
    
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[11]==SetTime[11])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
   
   if(state == 01111)//13
    {
    hour[12]--;
    if(hour[12]==0)
    {
    state = 10000;
    
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[12]==SetTime[12])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
   
    if(state == 10000)//14
    {
    hour[13]--;
    if(hour[13]==0)
    {
    state = 10001;
    
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[13]==SetTime[13])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
   
   if(state == 10001)//15
    {
    hour[14]--;
    if(hour[14]==0)
    {
    state = 10010;
    
    
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[14]==SetTime[14])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
   
   if(state == 10010)//16
    {
    hour[15]--;
    if(hour[15]==0)
    {
    state = 10011;
    
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[15]==SetTime[15])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
   
   if(state == 10011)//17
    {
    hour[16]--;
    if(hour[16]==0)
    {
    state = 10100;

    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[16]==SetTime[16])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
   
   if(state == 10100)//18
    {
    hour[17]--;
    if(hour[17]==0)
    {
    state = 10101;
    
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[17]==SetTime[17])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
   
   if(state == 10101)//19
    {
    hour[18]--;
    if(hour[18]==0)
    {
    state = 10110;
    
    
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[18]==SetTime[18])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
   
   if(state == 10110)//20
    {
    hour[19]--;
    if(hour[19]==0)
    {
    state = 10111;
    
    
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[19]==SetTime[19])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
   
   if(state == 10111)//21
    {
    hour[20]--;
    if(hour[20]==0)
    {
    state = 11000;    
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[20]==SetTime[20])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
   
   if(state == 11000)//22
    {
    hour[21]--;
    if(hour[21]==0)
    {
    state = 11001;
    
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[21]==SetTime[21])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
   
   if(state == 11001)//23
    {
    hour[22]--;
    if(hour[22]==0)
    {
    state = 11010;
   
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[22]==SetTime[22])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }
    }if(state == 11010)//24
    {
    hour[23]--;
    if(hour[23]==0)
    {
    state = 11110;
    hour[0] = 1;
    hour[1] = 2;
    hour[2] = 2;
    hour[3] = 2;
    hour[4] = 2;
    hour[5] = 2;
    hour[6] = 2;
    hour[7] = 2;
    hour[8] = 2;
    hour[9] = 2;
    hour[10] = 2;
    hour[11] = 2;
    hour[12] = 2;
    hour[13] = 2;
    hour[14] = 2;
    hour[15] = 2;
    hour[16] = 2;
    hour[17] = 2;
    hour[18] = 2;
    hour[19] = 2;
    hour[20] = 2;
    hour[21] = 2;
    hour[22] = 2;
    hour[23] = 2;
    
    INTCONbits.TMR0IF = 0;
    return;
    }
    else if(hour[23]==SetTime[23])
    {
        SetTrigger = 1;
        INTCONbits.TMR0IF = 0;
    return;
    }
    }
    } 

