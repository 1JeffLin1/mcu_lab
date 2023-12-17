// This is a traffic-light control program using Timer Module Timer/Counter : 
// VR1: 0-->midnight standby / yellow flashing; 1--> normal operation
// VR1 is replaced by eleclock
// VR0: 0-->manual control by pressing "F" to change direction; 1--> normal operation
/// Interconnect lines on the Practice Board : 
//   (1) VR --> MCU :  PA0 --> PA0,  PA1 --> PA1
//	 (2) 4*4KEY --> MCU : PA3 --> PA3 ("F"); NOTE: GND JUMP
//   (3) 7-SEG LEDs --> MCU : PD --> PD, PE --> PE
//   (4) 8 LEDs --> MCU : PC --> PC
//	 PA3@MPU: PA3/INT0/C0-/AN3
#include "HT66F50.h"

#define LedPort _pc // pc 埠
//#define LedPort8x8 _pd // pd 埠
#define Led7_com _pe // pd 埠
#define Led7_seg _pd // pe 埠
//#define divf_no 32767 // 65536
#define divf_no 32768/8  // clock freq =32.768 kHz
const unsigned char t2_int_H = divf_no/1/256;  //16 
const unsigned char t2_int_L = divf_no/1%256;  //255

const unsigned char led7com[4] = { 0x08, 0x04, 0x02, 0x01} ;
const unsigned char led7seg[11]= { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 
                     		0x6d, 0x7d, 0x07, 0x7f, 0x67, 0x00} ;
//
//const unsigned char t2_int_H = divf_no/256;  //127 
//const unsigned char t2_int_L = divf_no%256;  //255
unsigned char t2_dL, t2_dH, p5sec;
unsigned char dig_bcd[4], dig_bcd_r1[2], dig_bcd_r2[2], bin2bcd[2];
unsigned char dig_pos, bcd_L, flash_yellow, sign_mode, flash_yy, standby ;  //k, num, byte_H, byte_L
unsigned char t_g2r1, t_y2r1, t_r2sr1, t_r2g1, t_r2y1, t_r2r1s ;  
unsigned char t_d1, t_d2, t_d3, t_d4, t_d5, t_d6 ;  
unsigned char t_dsp_r1, t_dsp_r2, t_dsp_now ; 
//for electronic clock
unsigned char bcd_S, bcd_M, bcd_Hr, bcdcode;	//for electronic clock

void tm2_timer (void) ;
//enter the TM2 interrupt vector per 0.5 second
void __attribute((interrupt(0x14))) isr_TM2(void)
{	
	tm2_timer();		
//	_mf0f=0;
	_t2af=0;
}

void delay (unsigned char n)   // delay function
{
	unsigned char idy1, idy2 ;

	for(idy1 = 0; idy1 < n ; idy1++)   // 
		{
		   	GCC_NOP();	   		   
		   	for(idy2=0; idy2<16 ; idy2++)
		   	{
		  		GCC_NOP();   	   
			}		   		   
		}
}	


void bin_to_bcd (unsigned char bin_code)   // tranform 2 bytes of BCD codes into 4 bytes
{
   
	bin2bcd[0]=bin_code % 10;		
	bin2bcd[1]=bin_code / 10;
	if (bin2bcd[1]==0)
		bin2bcd[1]=10;

}
void bin_to_bcd_main (unsigned char bin_code)   // tranform 2 bytes of BCD codes into 4 bytes
{
   
	bin2bcd[0]=bin_code % 10;		
	bin2bcd[1]=bin_code / 10;
	if (bin2bcd[1]==0)
		bin2bcd[1]=10;

}

// bcd_S, bcd_M, bcd_Hr, bcdcode;
void eclock(void);
void tm2_timer (void)   // delay function
{
	p5sec=p5sec^0b00000001;	//p5sec: inverted once per 0.5 second
	if(p5sec==0)
		eclock();
	//if (_pa1 == 0) 	//midnight standby: flashing yellow light for two directions
	//00:00:00 ~ 00:01:59為深夜雙向道都閃黃燈，請更改????內容
	if (bcd_Hr == 0 && (bcd_M < 2)) 	//midnight standby: flashing yellow light for two directions
	{
		flash_yellow = flash_yellow ^ 0b00100010;	//on/off per 0.5 second
		flash_yy=0x00;	//let sign_mode=0
		//go on running timing procedure of traffic lights until entering the yellow light status
		if ((bcd_L != t_d2)	&& (bcd_L != t_d5))
    	{
    		if(p5sec==0)
    		{
    			t_dsp_now = t_dsp_now - 1;
				bin_to_bcd(t_dsp_now);
				if (bcd_L==t_d6)
				{
					bcd_L=0x01;
				}		
				else
				{
					bcd_L++;  
				}
    		}
			standby=0;	//normal 7-segment LED display 
    	}
    	else
   		{
			dig_bcd[1] = 10;
	    	dig_bcd[0] = 10;
			dig_bcd[3] = 10;
	    	dig_bcd[2] = 10; 
			standby=1;	//totally turn off 7-segment LED	    		 	
    	}  		 	
	}
	else if (_pa0 == 0) //manually control traffic lights by pishing "F" button to switch lights
	{
		//dig_bcd[1] = 10;
	    //dig_bcd[0] = 10;
		//dig_bcd[3] = 10;
	    //dig_bcd[2] = 10; 
		standby=1;
		flash_yy=0xff;	//normal signal_mode
		//push "F" and then bcd_L jumps to yellow-light status				
		if(_pa3==0)	 
		{
			if(bcd_L<t_d2 && bcd_L>t_d1)
			{
				bcd_L=t_d2;
				t_dsp_now=t_y2r1;
			}
			else if(bcd_L<t_d5 && bcd_L>t_d4)
			{
				bcd_L=t_d5;
				t_dsp_now=t_r2y1;
			}
		}
		//yellow/red light-->all red light-->red/green light
		//-->jump to last time for red/green light-->remain the status until "F" button is pressed 	
		else
		{
		if(bcd_L<t_d2 && bcd_L>t_d1)
		{
			bcd_L=t_d2-1;
			t_dsp_now=t_y2r1;
		}
		else if(bcd_L<t_d5 && bcd_L>t_d4)
		{
			bcd_L=t_d5-1;
			t_dsp_now=t_r2y1;
		}
		else
		{
    		if(p5sec==0)
    		{
    			t_dsp_now = t_dsp_now - 1;
				bin_to_bcd(t_dsp_now);
				if (bcd_L==t_d6)
				{
					bcd_L=0x01;
				}		
				else
				{
					bcd_L++;  
				}
    		}				
		}
		}
		bin_to_bcd(t_dsp_now);
	}
	//noram mode, i.e. not midnight standy and not manual traffic-light control		
	else
	{
		standby=0;
		flash_yy=0xff;	
		if(p5sec==0)
		{
			t_dsp_now = t_dsp_now - 1;
			bin_to_bcd(t_dsp_now);	
			if (bcd_L==t_d6)
			{
				bcd_L=0x01;
			}		
			else
			{
				bcd_L++;  
			}
		}					  		
	} 
//
}
void eclock(void)
{
 	if (bcd_S==0x59)
	{
		bcd_S=0x00;
		if (bcd_M ==0x59)
		{
			bcd_M =0x00;
		    if (bcd_Hr == 0x23)
				bcd_Hr = 0x00;
			else
				bcd_Hr ++ ;
	    }
		else
		{
			bcdcode=bcd_M & 0x0f;
			if (bcdcode==0x09)
			   bcd_M =bcd_M +0x07;
			else
			   bcd_M ++;        
		}
	}		
	else
	{
		bcdcode=bcd_S & 0x0f;	
		if (bcdcode==0x09)
		   bcd_S=bcd_S+0x07;
		else
		   bcd_S++;  
	}				   					
}
//
			
void main(void) // 主函式
{
//	unsigned int  j, i ;
//	unsigned char dig_pos, bcd_L, flash_yellow, sign_mode, flash_yy, standby ;  //k, num, byte_H, byte_L
//	unsigned char t_g2r1, t_y2r1, t_r2sr1, t_r2g1, t_r2y1, t_r2r1s ;  
//	unsigned char t_d1, t_d2, t_d3, t_d4, t_d5, t_d6 ;  
//	unsigned char t_dsp_r1, t_dsp_r2, t_dsp_now ; 
	
	t_r2r1s = 5;	//mode 1		
	t_g2r1 = 20;	//mode 2
	t_y2r1 =  4; 	//mode 3
	t_r2sr1 = 3;	//mode 4
	t_r2g1 = 35;	//mode 5
	t_r2y1 =  6;	//mode 6
//	mode 0   y2y1	

	t_d1 = t_r2r1s ;
	t_d2 = t_d1 + t_g2r1 ;
	t_d3 = t_d2 + t_y2r1 ;	
	t_d4 = t_d3 + t_r2sr1 ;		
	t_d5 = t_d4 + t_r2g1 ;
	t_d6 = t_d5 + t_r2y1 ;
	t_dsp_r1 = t_g2r1 + t_y2r1 + t_r2r1s;
	bin_to_bcd_main(t_dsp_r1);
	dig_bcd_r1[1] = bin2bcd[1];
	dig_bcd_r1[0] = bin2bcd[0];
	t_dsp_r2 = t_r2g1 + t_r2y1 + t_r2sr1;
	bin_to_bcd_main(t_dsp_r2);
	dig_bcd_r2[1] = bin2bcd[1];
	dig_bcd_r2[0] = bin2bcd[0];	
		
//	_acerl =0;  //POR:0b11111111, 
//The ACERH and ACERL control registers contain the ACER11~ACER0 bits 
//which determine which pins on PF1, PF0, PE1, PE0, and PORTA are used 
//as analog inputs for the A/D converter input and which pins are not
//to be used as the A/D converter input. Setting the corresponding bit
//bit high will select the A/D input function, clearing
//the bit to zero will select either the I/O or other
//pin-shared function.
	_smod=0b11100000;	// [7:5]=100->fH/16,110-->fH/4; [0]=1-->fH/? or fL
//	_smod=0b00000000; 	// [7:5]=000,001->fL,[0]=1-->fH, [1]=0:SLEEP	
	_pdc = 0 ; 	// 設定 PD 埠為輸出
	_pec = 0 ; 	// 設定 PE 埠為輸出
//	Led7_com = led7com[0] ; 	// 8x8 LED X-axis (PD) OFF
 //   Led7_seg = led7seg[0];	// 8x8 LED Y-axis (PE) ON	
//	LedPort = 0x00 ; 	// 8x8 LED X-axis (PD) OFF
//	_wdtc=0b01111010; //[3:0]=1010:default-->disable WDT timer
//	_pac0 = 1 ; 	// 設定 PA0 埠為輸入
//	_pac1 = 1 ; 	// 設定 PA1 埠為輸入
//	_pac = _pac & 0b00000000; //pA3~PA0為輸出
//
//	_pac1 = 1 ; 	// 設定 PA1 埠為輸入
//  PA0/C0X/TP0_0/AN0 ; PA1/TP1A/AN1 ;  PA2/TCK0/C0+/AN2 ; PA3/INT0/C0-/AN3 
	_pac = _pac | 0b00001111; //pA1為輸入
	_acerl = _acerl & 0b11110000;
    _c0os=0; // pa0 is disconnected to c0x and purly acts as an I/Opin
    _t0cp0=0; // pa0 is disconnected to TM0 and purly acts as an I/Opin
    _t1acp0=0; //pa1 is disconnected to TM1 
	_c0sel=0; // pa2 and pa3 are disconnected c0+ and c0-, respectively, and act as I/O pins
//	_c1os=1;  // pa5 is disconnected to c1x and purly acts as an I/Opin 
	_papu= _papu | 0b00001000;
//	_pawu=0b00000001;
//
	_pcc=0;	//PC7~PC0為輸出
	_t1bcp0=0; // pc0/TP1B_0 is disconnected to TM1
	_t1bcp1=0; // pc1/TP1B_1 is disconnected to TM1
	_c1sel=0;  //set PC2/C1+ and PC3/C1- and are disconnected c1+ and c1-
	_t2cp0=0;  // pc3/TP2_0 is disconnected to TM2 
	_t2cp1=0;  // pc4/TP2_1 is disconnected to TM2	
	_t1bcp2=0; // pc5/TP0_1/TP1B_2 is disconnected to TM1
	_t0cp1=0;  // pc5/TP0_1/TP1B_2 is disconnected to TM0

//	_pcc4=0;
//	_tm2c0=0b01000000; //[6:4]=100:ftbc, [2:0]=000:overflow 
//	_tm2c1=0b10101001; 
//[7:6]=00:comp match mode, [5:4]=11:toggle, [3]=0:init low, 
//          [2]=1:invert output, [0]=1:comp_A clr	
//[7:6]=10:PWM, [5:4]=10:PWM output, [3]=0/1:active low/high, 
//          [2]=1:invert output, [1]=0:CCRP/per(CCPA/duty), [0] not used	
//[7:6]=11:Timer/Counter, [5:4]=unused, [3:0] not used	
//	_tm2al=255;
//	_tm2ah=32;
//	_tm2rp=0;
//
//	_tm2c0=0b01000000; //[6:4]=100:ftbc, [2:0]=000:overflow 
//	_tm2c1=0b11000000; 
	_tm2c0=0b01000000; //[6:4]=100:ftbc, [2:0]=000:overflow 
	_tm2c1=0b00000001; //[7:6]=00: Compare Match Output Mode; [0]=1/0-->Comp_A/_P clear
	_tm2ah=t2_int_H; 
	_tm2al=t2_int_L;	
//	_tm2rp=0;	// overflow
//In the Timer/Counter Mode, the TM output pin control must be disabled. 
//	_t2cp0=0;  //t2cp0:pc3, t2cp1:pc4
//	_t2cp1=0;  //t2cp0:pc3, t2cp1:pc4	
//	_t2on=1;
//	bcd_H=0x00;
	bcd_L=0x00;
	
	t_dsp_now = t_dsp_r1;	
    dig_bcd[1] = dig_bcd_r1[1];
    dig_bcd[0] = dig_bcd_r1[0];   
    dig_bcd[3] = 10;
   	dig_bcd[2] = 10; 
	LedPort=0b00010001; //r2r1s, xgyrxgyr 	
//	LedPort8x8=0;
	_t2on=0;
	GCC_NOP();			
	_t2on=1;
	flash_yellow = 0b00100010;	
	sign_mode=1;
	flash_yy=0xff;	
	standby=0;
	p5sec=0;	
	_emi=1;
	_mf0e=1;
	_mf0f=0;
	_t2ae=1;
	_t2af=0;
	bcd_S=0x00;
	bcd_M=0x59;
	bcd_Hr=0x23;	

	while(1)
	{
//
		if ((bcd_L >= 0) && (bcd_L < t_d3))
		{
			dig_bcd[1] = bin2bcd[1];
			dig_bcd[0] = bin2bcd[0];
			dig_bcd[2] = 10;
		}
		else
		{
			dig_bcd[3] = bin2bcd[1];
			dig_bcd[2] = bin2bcd[0];
			dig_bcd[0] = 10;				
		}			
		if (bcd_L == t_d1)	
			sign_mode=2; //g2r1, xgyrxgyr 
		else if (bcd_L == t_d2)	
			sign_mode=3 & flash_yy; //y2r1, xgyrxgyr 
		else if (bcd_L == t_d3)	
		{	
			sign_mode=4; //r2sr1, xgyrxgyr 
			dig_bcd[1] = 10;
   			dig_bcd[0] = 0;
   			dig_bcd[3] = dig_bcd_r2[1];
   			dig_bcd[2] = dig_bcd_r2[0];
   			t_dsp_now = t_dsp_r2;    	
		}
		else if (bcd_L == t_d4)		
			sign_mode=5; //r2g1, xgyrxgyr 
		else if (bcd_L == t_d5)		
			sign_mode=6 & flash_yy; //r2y1, xgyrxgyr 
		else if (bcd_L == t_d6)	
		{	
			sign_mode=1; //r2r1s, xgyrxgyr 
   			dig_bcd[1] = dig_bcd_r1[1];
   			dig_bcd[0] = dig_bcd_r1[0];
   			dig_bcd[3] = 10;
   			dig_bcd[2] = 0;
   			t_dsp_now = t_dsp_r1;    	
		}		    				   				  	
//			select sign mode		  													
		if (sign_mode==1) 
			LedPort=0b00010001; //r2r1s, xgyrxgyr 
		else if (sign_mode==2)
			LedPort=0b01000001; //g2r1, xgyrxgyr 
		else if (sign_mode==3)	
			LedPort=0b00100001; //y2r1, xgyrxgyr 
		else if (sign_mode==4)	
			LedPort=0b00010001; //r2sr1, xgyrxgyr 
		else if (sign_mode==5)		
			LedPort=0b00010100; //r2g1, xgyrxgyr 
		else if (sign_mode==6)		
			LedPort=0b00010010; //r2y1, xgyrxgyr  
		else if (sign_mode==0)
			LedPort=flash_yellow;  //y2y1, xgyrxgyr		    		
//								
				
		for( dig_pos = 0 ; dig_pos < 4 ; dig_pos++ )
		{	
			Led7_com = 0x00 ; 	// 8x8 LED X-axis (PD) OFF
//			delay(1);
			Led7_seg = led7seg[dig_bcd[dig_pos]];	// 8x8 LED Y-axis (PE) ON
			if (standby==1)
				Led7_com = 0x00 ;
			else
				Led7_com = Led7_com | led7com[dig_pos] ; 	// 8x8 LED X-axis (PD) OFF
   			delay(1);	   
		}
	}	
}
