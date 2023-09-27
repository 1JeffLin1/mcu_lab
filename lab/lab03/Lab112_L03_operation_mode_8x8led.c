// This is a 8x8 dot-matrix LED sweeping program for practicing operation mode of CPU
// In this program, operation mode, watch-dog and wake-up function are the main lecture topics
// fsys: fL, fH/64, fH/32, fH/16, ..., fH/2, fH
// It consumes 4 system clock periods for every single instruction operation under the pipeline architecture
// Normal mode : 	_smod=0b11000000; or _smod=0b11000001;	// [0]=0 & [7:5]=100->fH/16,110-->fH/4, [0]=1->fH
// slow mode : 		_smod=0b00000000; 	// [0]=0 & [7:5]=000/001->fL, [0]=1->fH
// IDEL0 mode : 	_smod=0b11000010; 	// [1]=1->IDEL, _wdtc[7](FSYSON)=0-->IDEL0
// IDEL1 mode : 	_smod=0b11000010; 	// [1]=1->IDEL, _wdtc[7](FSYSON)=1-->IDEL1
// SLEEP0 mode : 	_smod=0b11000000; 	// [1]=0, WDT & LVD off; _wdtc[3:0]=1010->off; _lvdc[4]=0->LVD off
// SLEEP1 mode : 	_smod=0b11000000; 	// [1]=0, WDT or LVD on; _wdtc[3:0]=not 1010->on; _lvdc[4](LVDEN)=1->LVD on
// LVD Control register LVDC : the setting of the default LVD is off
// Time Base Control register TBC : the setting of the default TB is off
//
// WatchDog: tool/configuration option: choose one instruction for watchdog clear 
// Interconnect lines on the Practice Board : 
//   (1) 8x8 LEDs --> MCU : PE-->PE, PD-->PD
//   (2) keyboard(Buttons) --> MCU :  PA3 --> PA3, PA2 --> PA2, 
//                             using jump to short one pair of pins
//   push the "key B" to make the CPU into the sleep or idel status
//   push the "key F" to wake up the CPU from the sleep or idel status
//*** Notes for the program ***// 
//Disable the "Watchdog Timer Fuction" in the "Configuration Options"
//Within the setting section of the "IDLE1" operation mode, set wdtc[3:0]="1010" or "0101" to disable or enable WDT
//  and then obseve whether the patterns of 8x8LEDs run correctly.
//When wdtc[3:0]="0101", i.e. enableing WDT, oberve whether the patterns of 8x8LEDs run correctly 
//  with or without the execution of the "GCC_CLRWDT()" in the for-loop.
//	_wdtc=0b11111010;   // [7](FSYSON)=0(IDEL0), 1(IDEL1); [3:0]=1010->WDT off, other values->WDT on
//	_wdtc=0b11110101;   // [7](FSYSON)=0(IDEL0), 1(IDEL1); [3:0]=1010->WDT off, other values->WDT on
//****************************//
#include "HT66F50.h"
#define LedPortx _pd // pa 埠
#define LedPorty _pe // pa 埠

//const array : 總長度不超過一個32K; 一般的array: 不超過一個rambank
const unsigned char LedTablex[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 } ;
const unsigned char LedTabley[32] = { 0x00, 0x00, 0x00, 0x7f, 0x02, 0x08, 0x20, 0x7f, 
                       0x00, 0x00, 0x00, 0x7f, 0x08, 0x14, 0x22, 0x41,
                       0x00, 0x00, 0x00, 0x7f, 0x02, 0x08, 0x20, 0x7f,
                       0x00, 0x00, 0x00, 0x3f, 0x40, 0x40, 0x40, 0x3f} ;  
// sweeping x axis; At every x-axix point, show y-axix LEDs for a short time
/*
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
*/ 
void delay (unsigned int n)
{
	char idy1, idy2;
	for(idy1 = 0; idy1 < n ; idy1++)   //    
		{
		   	GCC_NOP();
//		   	GCC_NOP();	  		   
		   	for(idy2=0; idy2<3 ; idy2++)
		   	{
		  		GCC_NOP();
//		  		GCC_NOP();
//				GCC_NOP();		    	   
			}		   		   
		}
}		
		
void main(void) // 主函式
  {
	int k, j, i, char_no, swp_no ;
//	_acerl =0;  //POR:0b11111111, 
//The ACERH and ACERL control registers contain the ACER11~ACER0 bits 
//which determine which pins on PF1, PF0, PE1, PE0, and PORTA are used 
//as analog inputs for the A/D converter input and which pins are not
//to be used as the A/D converter input. Setting the corresponding bit
//bit high will select the A/D input function, clearing
//the bit to zero will select either the I/O or other
//pin-shared function.
	swp_no = 200;
// Normal
//	_smod=0b11000000; 	// [7:5]=100->fH/16,110->fH/4; [1]=0->No IDEL
//	_wdtc=0b01111010;   // [7](FSYSON)=0(IDEL0), 1(IDEL1); [3:0]=1010->WDT off, other values->WDT on
// SLOW
//	_smod=0b00000000; 	// [7:5]=000->fL; [1]=0->No IDEL
//	_wdtc=0b01111010;   // [7](FSYSON)=0(IDEL0), 1(IDEL1); [3:0]=1010->WDT off, other values->WDT on
//	swp_no = 5;
// IDEL0	
//	_smod=0b11000010; 	// [7:5]=100->fH/16,110->fH/4; [1]=1->IDEL
//	_wdtc=0b01111010;   // [7](FSYSON)=0(IDEL0), 1(IDEL1); [3:0]=1010->WDT off, other values->WDT on
//	_wdtc=0b01110101;   // [7](FSYSON)=0(IDEL0), 1(IDEL1); [3:0]=1010->WDT off, other values->WDT on
// IDEL1: watchdog on: push "B" to enter idel_1 and then automatically wake up after watchdog counter overflows 	
	_smod=0b11000010; 	// [7:5]=100->fH/16,110->fH/4; [1]=1->IDEL
	_wdtc=0b11111010;   // [7](FSYSON)=0(IDEL0), 1(IDEL1); [3:0]=1010->WDT off, other values->WDT on
//	_wdtc=0b11110101;   // [7](FSYSON)=0(IDEL0), 1(IDEL1); [3:0]=1010->WDT off, other values->WDT on
// SLEEP0	
//	_smod=0b11000000; 	// [7:5]=100->fH/16,110->fH/4; [1]=0->No IDEL, into sleep mode
//	_wdtc=0b11111010;   // [3:0]=1010->WDT off, other values->WDT on
// SLEEP1: 必須將WDT的clk設為fsys/4, 否則在sleep1期間，WDT照常計數，overflow會啟動wake-up	
//	_smod=0b11000000; 	// [7:5]=100->fH/16,110->fH/4; [1]=1->No IDEL
//	_wdtc=0b11111000;   // [3:0]=1010->WDT off, other values->WDT on	
// 在工具/配置選項/WDT clock source fs選擇fsub或fsys/4
				
	_pdc = 0 ; 	// 設定 PD 埠為輸出
	LedPortx = 0x00 ; 	// 8x8 LED X-axis (PD) OFF
	_pec=0; 			// 設定 PE 埠為輸出
    LedPorty = 0x00;	// 8x8 LED Y-axis (PE) ON
//  PA3/INT0/C0-/AN3  ;   PA2/TCK0/C0+/AN2    
    _acerl = 0b11110011;// 設定PA3, PA2不為ADC輸入埠
	_c0sel = 0;  	// _cp0c[7]=c0sel=0->設定PA3,PA2不為比較器C0的輸入c0-,C0+
	_pawu3 = 1;		//設定PA3有wake-up功能, 喚醒sleep或idel中的CPU 
//    _pawu = _pawu | 0b00001000;
	_papu3 = 1;		//設定PA3有pull-up電阻
	_pac3 = 1;		//設定PA3為輸入, 喚醒sleep或idel中的CPU
	_papu2 = 1;		//設定PA2有pull-up電阻
	_pac2 = 1;		//設定PA2為輸入, 設定CPU進入sleep或idel	
//	_pac = _pac | 0b00001100;
//	_papu = _papu | 0b00001100;	
	
	while(1)
	{
		for(char_no = 0 ; char_no < 4 ; char_no++)
		{
			for( j = 0 ; j < swp_no ; j++ )
			{
				for( k = 0 ; k < 8 ; k++ )
				{
					i=char_no*8+k;
					LedPortx = 0x00 ; // 輸出 LED 控制碼, 關滅 LEDx
					LedPorty = 0x00 ; // 輸出 LED 控制碼, 關滅 LEDx						
					LedPortx = LedTablex[k] ; // 輸出 LED 控制碼, 點亮 LEDx
					LedPorty = LedTabley[i] ; // 輸出 LED 控制碼, 點亮 LEDx		
					delay(5);	
//					GCC_CLRWDT();  														 
				}
//				GCC_CLRWDT();					
			}
			if (_pa2==0)
			{
				LedPortx = 0x00 ; // 輸出 LED 控制碼, 關滅 LEDx -->省電 
				LedPorty = 0x00 ; // 輸出 LED 控制碼, 關滅 LEDx	-->省電				
				GCC_HALT();
				//while (_lto == 0 )
				//while (_hto == 0 )				
				//{  }
			}			
		}
/*		if (_pa2==0)
		{
			_smod=0b00000000; 	// [7:5]=000->fL
			swp_no = 5;
			while (_lto == 0 )
			{  }
		}
		else
		{	
			_smod=0b11000000; 	// [7:5]=000->fL
			swp_no = 100;
			while (_hto == 0 )
			{  }			
		}
*/			
	}
}
