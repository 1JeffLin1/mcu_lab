// This is a program for practicing the Capture Input mode of Timer2 (Time 2).
// In this program, Input capture is the main lecture topic by using an ultrasonic sensing module.
// The Timer2 module is set to operate in the Capture Input mode to capture the rising and falling edges 
//    of the input signal coning from TP2_0, i.e., PC3. 
// The interruption function of the Timer2 module is enabled. In the interruption subroutine, 
//    the _tm2ah and _tm2al are read at rising and falling edges and the differnce of two sampling time, i.e., 
//    TM2 counting value, are calculated.
//　　Actually, the main function is to generate a single pulse with the pule width equal to or larger than 
//              10us on the pin PF0 to trigger the ulrasonic module.
//	  Then, get the pulse width on the pin "echo" that is conneted to the TP2_0 pin of the Timer2 module
//              and is used as the input signal for the Capture Input mode.
//	  Calculate the distance of the object away from the ultrasonic sensor and then diaplay it.
//		  fclk_tm2=8Mhz/16=0.5MHz --> 2us; counting value = 20 --> 40us
//        Assuming that the environmental temperature is 27 degree Celsius
//        The ultrasonic speed = 331.5+0.607*t = 331.5+0.607*27 = 347.89 m/sec = 0.0348 cm/us
//        distance= 0.0348 * (echo positive pulsewidth / 2) 
//				  = 0.0348 cm/us * (TM2 counting value * 2us / 2) 
//				  = 0.0348 * (TM2 counting value) cm
//				  =  (TM2 counting value) / 29
//
// System clock is 8MHz by setting "_smod=0b10000001;" and usning the HIRC@8MHz oscillator as the system clock
// Get the 8MHz/16 clock freqency by setting "_tm2c0=0b00100000;" //[6:4]=000:fsys/4; 001:fSYS; 010:fH/16; 011:fH/64; 100:ftbc 
//
// Interconnect lines on the Practice Board : 
//   (1) LEDs --> MCU : PC6,PC7-->PC6,PC7
//   (2) Ultrasonic --> MCU : TRIG --> PF0, ECHO --> PC3
//   (3) 7-segment-LEDs 4 digit display --> MCU : PD-->PD, PE-->PE 
//
#include "HT66F50.h"

#define Led7_com _pe // pd 埠
#define Led7_seg _pd // pe 埠

const unsigned char led7com[4] = { 0x08, 0x04, 0x02, 0x01 } ; //{LSB, .. ,MSB}: PE0,PE1,PE2,PE3-->MSB~~LSB 
const unsigned char led7seg[11]= { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 
                     		0x6d, 0x7d, 0x07, 0x7f, 0x67, 0x00 } ;
                     		
unsigned char dig_bcd[4];
//unsigned char echo_no;
unsigned int  echo_pos, echo_neg, duration;


/*
void delay (unsigned int n) //(2+(4+1+2)*4+1+1+2)*n+1=34n+1 
{
	unsigned int idy1, idy2 ;

	for(idy1 = 0; idy1 < n ; idy1++)   // 
		{
		   	//GCC_NOP();
		   	//GCC_NOP();	   		   
		   	for(idy2=0; idy2<4 ; idy2++)
		   	{
		  		GCC_NOP();
		  		GCC_NOP();
				GCC_NOP();	
				GCC_NOP();  //	    	   
			}		   		   
		}
}		
*/
void delay_us (unsigned int n) //2+(1+1+2)*n+1=4n+3
{
	unsigned int idy1;

	for(idy1 = 0; idy1 < n ; idy1++)   // 
		{
		   	GCC_NOP();		   		   
		}
}

void distance_to_4byte (unsigned int dist)   // tranform 2 bytes of BCD codes into 4 bytes
{
	unsigned int val1,val2;
	dig_bcd[3]=dist/1000;
	val1=(dist%1000);			
	dig_bcd[2]=val1/100;	
	val2=(val1%100);	
	dig_bcd[1]=val2/10;
	dig_bcd[0]=val2%10;		//high nibble of Hbyte_bcd
	if (dist<1000){
		dig_bcd[3]=10;
		if (dist<100){
			dig_bcd[2]=10;
			if (dist<10){
				dig_bcd[1]=10;
			} 
		}
	}
}	
		
void main(void) // 主函式
{
	//unsigned char t2d_hi,t2d_lo;
	unsigned char t2d_hi_pos,t2d_lo_pos,t2d_hi_neg,t2d_lo_neg;	
	unsigned int distance;
	unsigned char dig_pos;
	unsigned char meas_delay;
//The ACERH and ACERL control registers contain the ACER11~ACER0 bits 
//which determine which pins on PF1, PF0, PE1, PE0, and PORTA are used 
//as analog inputs for the A/D converter input and which pins are not
//to be used as the A/D converter input. Setting the corresponding bit
//bit high will select the A/D input function, clearing
//the bit to zero will select either the I/O or other
//pin-shared function.
	_smod=0b10000001; 	// [7:5]=100->fH/16,101->fH/8,110-->fH/4; [4]:fsten; [3]:LTO; [2]:HTO; [1]:IDLEN; [0]=1-->fH, 0-->fH/? or fL
//	_smod=0b00000000; 	// [7:5]=000,001->fL,[0]=1-->fH, [1]=0:SLEEP	
//	_wdtc=0b01111010; //[3:0]=1010:default-->disable WDT timer

	_pdc = 0 ; 	// 設定 PD 埠為輸出; seven-segment LEDs
	_pec = 0 ; 	// 設定 PE 埠為輸出; PE[3:0]=com port
	Led7_com = led7com[0] ; 	// initial common port
    Led7_seg = led7seg[0];	// initial LED port	

//Time2:____                         ____
//  PC3/PINT/TP2_0/C1- ; PC4/[INT0]/[PINT]/TCK3/TP2_1
//	
	_c1sel=0;  //set PC3/C1- as I/O after assigning _c1sel=0
	_pcc3=1;	// 設定 PC3 埠為輸入
	_pcc4=0;	// 設定 PC4 埠為輸出
//fclk_tm2=ftbc (LXT:32.768kHz, LIRC:32kHz)
//ultrasonic speed = 348 m/sec
//Measured Distance = 348*(echo count value)/32000 
//Measured Distance = 348*(echo count value)/32000
//	_tm2c0=0b01000000; //[6:4]=000:fsys/4; 001:fSYS; 010:fH/16; 011:fH/64; 100:ftbc, [2:0]=unused 	 
//[7]=T2PAU; [6:4]=TnCK2~TnCK0; [3]=T2ON; [2:0]=T2RP2~T2RP0(TM0.TM3)/unused(TM2)		
//  _tm2c1 register setting 
//[7:6]=T2M1,T2M0; [5:4]=T2IO1,T2IO0; [3]=T2OC; [2]=T2POL; [1]=T2DPX; [0]=T2CCLR		
//[7:6]=00:comp match mode, [5:4]=11:toggle, 01:low, 10:high, [3]=0:init low, [0]=1:comp_A clr	
//[7:6]=01:Capture Input Mode(STM) /or undefined(CTM), 
//   [5:4]=00:rising/01:falling/10:rising&falling/11:input capture disable   
//   when capturing(input rising or falling)-->on-going counting value is stored in CCRA
//[7:6]=10:PWM/single pulse, [5:4]=10:PWM output/11:single pulse/00:PWM output inactive state/01:PWM output active state 
//   [3]=0/1:active low/high, [2]=0/1:noninvert/invert, [1]=0:CCRP/per(CCPA/duty), [0] not used	
// single pulse: T2ON:0->1 active --- T2ON:1->0 or match CCRA finish single pulse
//   [5:4]=T2IO1,T2IO0=11 for single pulse              
//[7:6]=11:Timer/Counter, [5:4]=unused, [3:0] not used	
//In the Timer/Counter Mode, the TM output pin control must be disabled.
//  TM output pin control resgister: TMPC0:T0CP1,T0CP0; TMPC1:T3CP1,T3CP0,T2CP1,T2CP0
//	_tm2c1=0b00110001;  //[7:6]=00:comp match mode, [5:4]=11:toggle
// 	_tm2c1=0b01000001; 	//[7:6]=01:Capture Input Mode, [5:4]=00:rising	
//	_tm2c1=0b10100001; 	//[7:6]=10:PWM/single pulse, [5:4]=10:PWM output,
//	_tm2c1=0b10111001; 	//[7:6]=10:PWM/single pulse, [5:4]=11:single pulse,	
// 	_tm2c1=0b10110001;	
//      make the instructions about single pulse output executable
//	_tm2c1=0b11110001; 	//[7:6]=11:Timer/Counter,
//fclk_tm2=8Mhz/16=0.5MHz --> 2us; counting value = 20 --> 40us
	_tm2c0=0b00100000; //[6:4]=000:fsys/4; 001:fSYS; 010:fH/16; 011:fH/64; 100:ftbc, [2:0]=unused 
	//_tm2c1=0b10111001; 	//[7:6]=10:PWM/single pulse, [5:4]=11:single pulse,	
 	//_tm2c1=0b01100001; 	//[7:6]=01:Capture Input Mode, [5:4]=00:rising/01:falling/10:rising&falling/11:input capture disable 
 	_tm2c1=0b11110001; 	//[7:6]=11:Timer/Counter,
	//_tm2al=0;
	//_tm2ah=30;	
	//_tm2rp=0;  //2^16=65536 overflow
	_t2cp0=0;  //t2cp0:pc3, t2cp1:pc4
	_t2cp1=0;  //t2cp0:pc3, t2cp1:pc4	
	_t2on=0;
		
	_pfc0 = 0;		//Triiger Ultrasonic, 1:輸入, 0:輸出 
	//_pfc1 = 1;		//Echo Ultrasonic, 1:輸入, 0:輸出
	_pcc7 = 0;
	_pcc6 = 0;
	_acerl = 0b00000000;// 設定PA3不為ADC輸入埠
	_c1os = 1;
	_pac &= 0b00011111;
	
	
//TRIG-->PF0, ECHO-->PC3	
//		
	while(1)
	{		
		//Trigger Ultrasonic 0->1->0
		_pf0=0;
		delay_us(5);
		_pf0=1;
		delay_us(20);			
		_pf0=0;									
//			GCC_NOP();
		//echo_no=0;

		_t2on=1;
//		//wait for the postive edge of the echoed pulse
		while(_pc3==0)
		{
			//t2d_hi=_tm2dh;
			//t2d_lo=_tm2dl;
			//echo_pos=t2a_hi*256+t2a_lo;
			_pc7=1;
		}
		t2d_hi_pos=_tm2dh;
		t2d_lo_pos=_tm2dl;
		//echo_pos=t2d_hi_pos*256+t2d_lo_pos;	
		//wait for the negative edge of the echoed pulse	
		while(_pc3==1)
		{
			//t2d_hi=_tm2dh;
			//t2d_lo=_tm2dl;
			//echo_neg=t2a_hi*256+t2a_lo;
			_pc6=1;
		}	
		t2d_hi_neg=_tm2dh;
		t2d_lo_neg=_tm2dl;
		echo_pos=t2d_hi_pos*256+t2d_lo_pos;
		echo_neg=t2d_hi_neg*256+t2d_lo_neg;		
		
		duration=echo_neg-echo_pos;	
//

		//wait for the response of the ultrasonic module	
		delay_us(20);							
//
		_t2on=0;			
		distance=duration/29;	//0.0348*echo=echo/29-->0~2281cm
		distance_to_4byte(distance);	

//		Sweeping 7-seg 4 digit display within (1/32000) second
//		Execution time of the Nested for-loop occupies the large portion of the whole while(1)-loop
//   	  to make sure that the LED lighting brightness of the 7-segment display is strong.
		for(meas_delay=0; meas_delay<50; meas_delay++)
		{
			for( dig_pos = 0 ; dig_pos < 4 ; dig_pos++ )
			{	
				Led7_com = 0x00 ; 	// 8x8 LED X-axis (PD) OFF
//				delay(1);
				Led7_seg = led7seg[dig_bcd[dig_pos]];	// 8x8 LED Y-axis (PE) ON
				Led7_com = led7com[dig_pos] ; 	// 8x8 LED X-axis (PD) OFF
				if (dig_pos<3)
   					delay_us(160);	   //adjust the brightness of LEDs of the 7-segment display 
			}
			if(distance < 10){  // red
				_pac5 = 0;
				_pac6 = 1;
				_pac7 = 1;
			}
			else if(distance <= 25 & distance >= 10){ // green
				_pac5 = 1;
				_pac6 = 0;
				_pac7 = 1;
				}
			else{                       // blue
				_pac5 = 1;
				_pac6 = 1;
				_pac7 = 0;
				} 

				
		}	
	}	
}