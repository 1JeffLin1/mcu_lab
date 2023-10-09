// This is a program for practicing operation mode of Timer (Time 2)
// In this program,  Compare Match, PWM, Single Pulse, and input capture are the main lecture topics
//　　Actually, the main function is to use the PWM function to adjust light intensity of LED.
//    Change tm2c0:[6:4] to change the clock frequency of TM2 and then observe LED status.
//    Change _tm2rp value and observe LED status when tm2c0:[6:4]=000.
//
// Interconnect lines on the Practice Board : 
//   (1) LEDs --> MCU : PC0~PC7-->PC0~PC7, really only PC3 and PC4 are used
//   (2) keyboard(Buttons) --> MCU :  PA3 --> PA3
//                             using jump to short one pair of pins
//   push the "key F" to make a single pulse
// You can set _PC4(or _PC3)=0 or reset _PC4=1(or _PC3) 
//     to observe the lighting status of the PC3 LED and the PC4 LED
#include "HT66F50.h"

void delay (unsigned int n)
{
	unsigned int idy1, idy2, idy3 ;

	for(idy1 = 0; idy1 < n ; idy1++)   // 
		{
		   	GCC_NOP();
		   	GCC_NOP();	   		   
		   	for(idy2=0; idy2<200 ; idy2++)
		   	{
		  		for(idy3=0; idy3<4 ; idy3++)
		   		{
		  			GCC_NOP();
		  			GCC_NOP();
					GCC_NOP();	
					GCC_NOP();
		   		}					    	   
			}		   		   
		}
}		
		
void main(void) // 主函式
{
//	unsigned int j, i, char_no ;
//	unsigned char k;  // don't use unsigned int ;
//	_acerl =0;  //POR:0b11111111, 
//The ACERH and ACERL control registers contain the ACER11~ACER0 bits 
//which determine which pins on PF1, PF0, PE1, PE0, and PORTA are used 
//as analog inputs for the A/D converter input and which pins are not
//to be used as the A/D converter input. Setting the corresponding bit
//bit high will select the A/D input function, clearing
//the bit to zero will select either the I/O or other
//pin-shared function.
//*** set system clock ***
	_smod=0b10000001; 	// [7:5]=100->fH/16,110-->fH/4; [4]:fsten; [3]:LTO; [2]:HTO; [1]:IDLEN; [0]=1-->fH/? or fL
//	_smod=0b00000000; 	// [7:5]=000,001->fL,[0]=1-->fH, [1]=0:SLEEP	
//	_wdtc=0b01111010; //[3:0]=1010:default-->disable WDT timer
//************************
//
/*multifunctional I/O pins: must close high-priority functions to assign a desired function to a I/O pin */
/* the rightmost function is the highest-priority function                                               */ 
//  PA0/C0X/TP0_0/AN0 ; PC5/[INT1]/TP0_1/TP1B_2/[PCK]  ; _t1bcp2 default=0;
//  PC2/TCK2/PCK/C1+ ;  PC3/PINT/TP2_0/C1- ; PC4/[INT0]/[PINT]/TCK3/TP2_1
//	
	_c1sel=0;  //set PC3/C1- as I/O
	_pcc3=0;	// 設定 PC3 埠為輸出
	_pcc4=0;	// 設定 PC4 埠為輸出
//********************************************************************************************************	
//
//*** _tm2c0 register setting *** 
//***_tm2c0: [7]=T2PAU; [6:4]=TnCK2~TnCK0; [3]=T2ON; [2:0]=T2RP2~T2RP0(TM0.TM3)/unused(TM2)		
	_tm2c0=0b01000000; //[6:4]=000:fsys/4; 100:ftbc(fL), [2:0]=unused 
//	_tm2c0=0b00000000; //[6:4]=000:fsys/4; 100:ftbc, [2:0]=unused 	
//********************************
//	
//*** _tm2c1 register setting ***
//[7:6]=T2M1,T2M0; [5:4]=T2IO1,T2IO0; [3]=T2OC; [2]=T2POL; [1]=T2DPX; [0]=T2CCLR		
//(1)[7:6]=00:comp match mode, [5:4]=11:toggle, 01:low, 10:high, [3]=0:init low, [0]=1:comp_A clr	
//(2)[7:6]=01:Capture Input Mode(STM) /or undefined(CTM), 
//   [5:4]=00:rising/01:falling/10:rising&falling/11:input capture disable   
//   when capturing(input rising or falling)-->on-going counting value is stored in CCRA
//(3)[7:6]=10:PWM/single pulse, [5:4]=10:PWM output/11:single pulse/00:PWM output inactive state/01:PWM output active state 
//   [3]=0/1:active low/high, [2]=0/1:noninvert/invert, [1]=0:CCRP/per(CCPA/duty), [0] not used	
// single pulse: T2ON:0->1 active --- T2ON:1->0 or match CCRA finish single pulse
//   [5:4]=T2IO1,T2IO0=11 for single pulse              
//(4)[7:6]=11:Timer/Counter, [5:4]=unused, [3:0] not used	
//In the Timer/Counter Mode, the TM output pin control must be disabled.
//  TM output pin control resgister: TMPC0:T0CP1,T0CP0; TMPC1:T3CP1,T3CP0,T2CP1,T2CP0
//*******************************
//	_tm2c1=0b00110001;  //[7:6]=00:comp match mode, [5:4]=11:toggle
//	_tm2c1=0b01000001; 	//[7:6]=01:Capture Input Mode, [5:4]=00:rising	
//	_tm2c1=0b10100010; 	//[7:6]=10:PWM/single pulse, [5:4]=10:PWM output, [1]=1:CCRA/period(CCPP/duty)
	_tm2c1=0b10111001; 	//[7:6]=10:PWM/single pulse, [5:4]=11:single pulse,	
// 	_tm2c1=0b10110001;	
//      make the instructions about single pulse output executable
//	_tm2c1=0b11110001; 	//[7:6]=11:Timer/Counter,		
	_tm2al=0;
	_tm2ah=50;	
	_tm2rp=10;  //
	_t2cp0=1;  //t2cp0:pc3, t2cp1:pc4; default(POR):_t2cp0=1
	_t2cp1=0;  //t2cp0:pc3, t2cp1:pc4; default(POR):_t2cp1=0	
	_t2on=1;
//  port default output = 1
	_pc4=0;		//0:Normal output for TP2_1; default output value = "1";
  	_pc3=0;		//(default), _pc4=0 --> observe the pc3 and pc4 LED lighting	
	
/* TM0
	_tm0c0=0b01000000; //[7]=T2PAU, [6:4]=000:fsys/4; 100:ftbc, [3]=T2ON, [2:0]=000(CRP 3bits):overflow; 001:128 
	_tm0c1=0b10100001; 	//[7:6]=10:PWM
	_tm0al=0;
	_tm0ah=0x05;	
	_t0cp0=0;  //t0cp0:pa0, t0cp1:pc5
	_t0cp1=1;  //t0cp0:pa0, t0cp1:pc5	
	_pcc5=0;
	_pc5=0;		//0:Normal output for TP0_1; default output value = "1";
	_t0on=1;
*/	
//
    _acerl = 0b11110111;// 設定PA3不為ADC輸入埠
    //_acerl = _acerl & 0b11110111;
	_c0sel = 0;  	// _cp0c[7]=_c0sel=0->設定PA3,PA2不為比較器C0的輸入c0-,C0+
	_papu3 = 1;		//設定PA3有pull-up電阻
	_pac3 = 1;		//設定PA3為輸入, 喚醒sleep或idel中的CPU
//	
//		
	while(1)
	{
//		
		delay(250);
/*
		_t2on=0;
		GCC_NOP();  		
		_t2on=1;
*/
//      *** Single PLUSE OUTPUT to LED ***
		//
		if (_pa3 == 0)
		{		
			_t2on=0;
			_tm2al=0;
			_tm2ah=80;
			_t2on=1;
		}
		//
//		
	}	
}
