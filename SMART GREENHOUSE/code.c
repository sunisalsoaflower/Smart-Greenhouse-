#include<reg51.h>
#include<stdio.h>

//INPUTS

#define input_port P1  //ADC
sbit ADDA=P3^0;  //Address pins for selecting input channels.
sbit ADDB=P3^1;
sbit ADDC=P3^2;
sbit ale=P3^3;  
sbit oe=P3^6; 
sbit sc=P3^4; 
sbit eoc=P3^5;  
sbit clk=P3^7;  

sbit manual = P2^7;

// OUTPUTS

#define lcdport P0  //LCD
sbit rs=P0^0;
sbit rw=P0^2;
sbit en=P0^1;

sbit cooler = P2^0;   
sbit heater = P2^1;
sbit UV = P2^2;
sbit motor = P2^3;


int result[3],number;

void timer0() interrupt 1  // Function to generate clock of frequency 500KHZ using Timer 0 interrupt.
{
clk=~clk;
}

void delay(unsigned int count)    // Delay Function
{
int i,j;
for(i=0;i<count;i++)
  for(j=0;j<100;j++);
}

void daten()   // Function to send data to lcd and print
{
    rs=1;
    rw=0;
    en=1;
    delay(1);
    en=0;
}

void lcd_data(unsigned char ch)   // LCD data processed 
{
    lcdport=ch & 0xF0;
    daten();
    lcdport=ch<<4 & 0xF0;
    daten();
}

void cmden(void)   // LCD commands sent to lcd
{
    rs=0;
    en=1;
    delay(1);
    en=0;
}

void lcdcmd(unsigned char ch)  // LCD commands processed
{

    lcdport=ch & 0xf0;
    cmden();
    lcdport=ch<<4 & 0xF0;
    cmden();
}

lcdprint(unsigned char *str)  //Function to send string data to LCD.
{
    while(*str)
    {
        lcd_data(*str);
        str++;
    }
}

void lcd_ini()  //Function to inisialize the LCD
{
    lcdcmd(0x02);
    lcdcmd(0x28);
    lcdcmd(0x0e);
    lcdcmd(0x01);
}

void show()       // Function to convert value to string to be able to print
{ 
   sprintf(result,"%d",number);
   lcdprint(result);
   lcdprint("  ");
}

void read_adc()    // To read adc value, and save it
{
   number=0;
   ale=1;
   sc=1;
   delay(1);
   ale=0;
   sc=0;
   while(eoc==1);
   while(eoc==0);
   oe=1;
   number=input_port;
   delay(1);
   oe=0;
}

void adc(int i)  //Function to drive ADC
{
switch(i)
  {
   case 0:
    ADDC=0;  // Selecting input channel IN0 (Temperature) using address lines
    ADDB=0;
    ADDA=0;
    lcdcmd(0xc0);
    read_adc();
	number = number *2;
	if (number > 20 ) {cooler = 1; heater = 0;}  // If else conditions to bring about change
	if (number < 15 ) {heater = 1; cooler = 0;}
	else {heater = 0; cooler = 0;}
    show();
    break;

   case 1:
    ADDC=0;  // Selecting input channel IN1 (Light) using address lines
    ADDB=0;
    ADDA=1;
    lcdcmd(0xc6);
    read_adc();
	number = 100 - number/10;
	if (number < 80 ) {UV = 1;}  // If else conditions to bring about change
	else {UV = 0;}
    show();
   break;

   case 2:
    ADDC=0;  // Selecting input channel IN2 (Soil Moisture) using address lines
    ADDB=1;
    ADDA=0;
    lcdcmd(0xcc);
    read_adc();
    if (number < 100 ) {motor = 1; delay(100); motor =0;} // If else conditions to bring about change
    show();
    break;
  }
}

void main()
{
 int i=0;
 P2 = 0;
 eoc=1;
 ale=0;
 oe=0;
 sc=0;
 TMOD=0x02;
 TH0=0xFD;
lcd_ini();
lcdprint(" ADC 0808  ");
lcdcmd(192);
lcdprint("  Interfacing   ");
delay(500);
lcdcmd(1);
lcdprint("Circuit Digest ");
lcdcmd(192);
lcdprint("System Ready...   ");
delay(500);
lcdcmd(1);
lcdprint("Ch1   Ch2   Ch3 ");
 IE=0x82;
 TR0=1;
while(1)
{
if ( manual == 1 ) {motor = 1; delay(100); motor =0;}   // To manually turn on motor
   for(i=0;i<3;i++)
   {
     adc(i);
     number=0;
   }
}
}