// This is a LED sweeping program。
// In this program, memory, system clock and port are the main lecture topic
// Array declaration with/without "const" maps to the usage of ROM/RAM memory
// Interconnect lines on the Practice Board : 
//   (1) CdS light sensor --> MCU : PA2-->PF0, V_D-->VDD, G_D-->VSS
//   (2) LEDs --> MCU :  PC --> PC
//   Under no bright light, LEDs runs by the LEDTab1[1] pattern
//   under bright light, LEDs runs by the LEDTab1[2] pattern
#include "HT66F50.h"
#define LedPort _pc // pe 埠

// sweeping x axis; At every x-axix point, show y-axix LEDs for a short time
void delay (unsigned int n)	//(2+(4+1+2)*4+1+1+2)*n+1=34n+1 
{
	unsigned int idy1, idy2;
	for(idy1 = 0; idy1 < n ; idy1++)   // 
		{
		   	GCC_NOP();	//no operation, delay a instruction period
		   	GCC_NOP();	   		   
		   	for(idy2=0; idy2<4 ; idy2++)
		   	{
		  		GCC_NOP();
		  		GCC_NOP();
				GCC_NOP();		    	   
			}		   		   
		}
}
/* 
void delay_us (unsigned int n) //2+(1+1+2)*n+1=4n+3
{
     unsigned int idy1;

     for(idy1 = 0; idy1 < n ; idy1++)   // 
     {
		GCC_NOP();		   		   
	}
}
*/		
		
void main(void) // 主函式
{
	unsigned int delay_no ;
/* System Operation Mode: _smod */
//	_smod=0b11000000; 	// [7:5]=100-->fH/16,110-->fH/4; [0]=1-->fH, 0--fH/2~fH/64 or fL
// 		 [0]= 	1: fH		0: fH/2~fH/64 or fL, chosen by [7:5] 
//       [7:5]= 000: fL (fLXT or fLIRC) 	001: fL (fLXT or fLIRC) 
//              010: fH/64;	011: fH/32;	100: fH/16;	101: fH/8;	110: fH/4;	111: fH/2 
	_smod=0b00000000; 	// [7:5]=100-->fH/16,110-->fH/4,000,001->fL,[0]=1-->fH	

	_pcc = 0;	// 設定 PE 埠為輸出, 0:輸出; 1:輸入
	_pfc0 = 1 ; // 設定 PF0接腳為輸出, 0:輸出; 1:輸入
/*multifunctional I/O pins: must close high-priority functions to assign a desired function to a I/O pin */
/* the rightmost function is the highest-priority function                                               */ 
//  PA0/C0X/TP0_0/AN0 ; PC5/[INT1]/TP0_1/TP1B_2/[PCK]  ; _t1bcp2 default=0;
//  PC2/TCK2/PCK/C1+ ;  PC3/PINT/TP2_0/C1- ; PC4/[INT0]/[PINT]/TCK3/TP2_1
	_c1sel=0;  //set PC3/C1- as I/O
	_t0cp1=0;  //t0cp0:pa0, t0cp1:pc5	
	
	LedPort = 0x00 ; 	// LEDs port
	delay_no1 = 10;
	delay_no2 = 20;
	
	while(1)
	{
		if (_pf0 == 0)
		{	LedPort =0x01;
			delay(delay_no1);
			LedPort =0x02;
			delay(delay_no1);
			LedPort =0x04;
			delay(delay_no1);
			LedPort =0x08;
			delay(delay_no1);
			LedPort =0x10;
			delay(delay_no1);			
			LedPort =0x20;
			delay(delay_no1);
			LedPort =0x40;
			delay(delay_no1);
			LedPort =0x80;
			delay(delay_no1);
			LedPort =0x40;				
			delay(delay_no1);
			LedPort =0x20;
			delay(delay_no1);
			LedPort =0x10;
			delay(delay_no1);
			LedPort =0x08;
			delay(delay_no1);
			LedPort =0x04;
			delay(delay_no1);
			LedPort =0x02;
			delay(delay_no1);
			LedPort =0x01;
			delay(delay_no1);	
			LedPort =0x00;
			delay(delay_no1);																							
		}		
		else
		{	LedPort =0x81;
			delay(delay_no2);
			LedPort =0x42;
			delay(delay_no2);
			LedPort =0x24;
			delay(delay_no2);						
			LedPort =0x18;
			delay(delay_no2);
			LedPort =0x24;
			delay(delay_no2);
			LedPort =0x42;
			delay(delay_no2);
			LedPort =0x81;
			delay(delay_no2);
			LedPort =0xc3;
			delay(delay_no2);
			LedPort =0xe7;
			delay(delay_no2);
			LedPort =0xff;
			delay(delay_no2);
			LedPort =0x7e;
			delay(delay_no2);	
			LedPort =0x3c;
			delay(delay_no2);	
			LedPort =0x18;
			delay(delay_no2);
			LedPort =0x24;
			delay(delay_no2);
			LedPort =0x42;
			delay(delay_no2);
			LedPort =0x81;
			delay(delay_no2);
			LedPort =0x00;
			delay(delay_no2);																															
		}
	}
}
