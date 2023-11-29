// This is a control program for TM2 and external interrupt 0 
// TM2 works as a electronic clock (Eclock). 
// Seven-segment LEDs are used to display minute and second of TM2 eleclock. 
// Normally, the Seven-segment LEDs display minute and second of TM2 eleclock. 
// PA3 is the INT0 pin. When PA3 change from 1 to 0, the INT0 interrupt subroutine is run
//  and minute and second of the elecock is reset
//      PA3/INT0/C0-/AN3
// PD7~PD0(a~h) and PE3~PE0(com0~3) are used to control Seven-segment LEDs

#include "HT66F50.h" 

#define Led7_com _pe // pe 埠
#define Led7_seg _pd // pd 埠
//  In Holtek C, char, int(16), short(16) --> 8 bits, long(32) --> 16 bits
#define divf_no 32768  // clock freq =32.768 kHz
#define speedup 16		//normally 1 
#define t2_int_H (divf_no/speedup)/256  //16 
#define t2_int_L (divf_no/speedup)%256  //255

unsigned char led7com[4] = { 0x08, 0x04, 0x02, 0x01 } ;
unsigned char led7seg[11]= { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 
                     		0x6d, 0x7d, 0x07, 0x7f, 0x67,
                     		0x00 } ;

unsigned char dig_bcd[4];  
unsigned char bcd_H, bcd_L, bcdcode;
//             		
//
void delay (unsigned int n)   // delay function
{
	unsigned long  idy1;
	unsigned short idy2 ;

	for(idy1 = 0; idy1 < n ; idy1++)   // 
		{
		   	GCC_NOP();
		   	GCC_NOP();	   		   
		   	for(idy2=0; idy2<4 ; idy2++)
		   	{
		  		GCC_NOP();
		  		GCC_NOP();
				GCC_NOP();		    	   
			}		   		   
		}
}
void tm2_timer (void)   // delay function
{

    //*** increase one second ***/
	//*** Here, Please copy and revise the program code of the elecock in the previous lab ***//
	if (bcd_L==0x59)
	{
	    ?????????????? 
	
	}		
	else
	{
         ????????????? 

	}							   			
	dig_bcd[0]=bcd_L & 0x0f;		
	dig_bcd[1]=bcd_L/16;
	dig_bcd[2]=bcd_H & 0x0f;
	dig_bcd[3]=bcd_H/16;
	//when minute changes from 59 to 0, let beep on for 3 seconds.
	if ((bcd_H==???) && ((bcd_L<=???) && (bcd_L>=???)))
		_t0on=1;
	else
		_t0on=0;

}	
//
//
//
void __attribute((interrupt(0x04))) isr_int0(void)
{
	//*** clear bcd_L and bcd_H ***//
	bcd_L = ???;
	bcd_H = ???;
	
	//_int0f=0;  automatiocaaly reset, the instruction is not necessary.
}
//
void __attribute((interrupt(0x14))) isr_TM2(void)
{
	
	tm2_timer();
		
	//_mf0f=0;  automatiocaaly reset, the instruction is not necessary
	
	//*** reset tm2 interrupt flag ***//
	_t2af=???;
}
/*
void __attribute((interrupt(0x1c))) isr_adc(void)
{
	unsigned char int_acc, int_status;
//	int_acc=_acc;
//	int_status=_status;
	adc0h=_adrh;
	adc0l=_adrl;
//	adc_val0=adc0h*256+adc0l;	
//	bin2bcd(adc_val0);
//	_adf=0;
//	_acc=int_acc;
//	_status=int_status;	
}
*/		
void main(void) // 主函式
 {
//  In Holtek C, char, int(16), short(16) --> 8 bits, long(32) --> 16 bits
	unsigned char dig_pos;	//position of 4-digit seven-segment LEDs
//	unsigned long adc_val0, adc_val1, adc_val2;
//	unsigned short 	adc0h, adc0l, adc1h, adc1l, adc2h, adc2l;	
//	_acerl =0;  //POR:0b11111111, 
//The ACERH and ACERL control registers contain the ACER11~ACER0 bits 
//which determine which pins on PF1, PF0, PE1, PE0, and PORTA are used 
//as analog inputs for the A/D converter input and which pins are not
//to be used as the A/D converter input. Setting the corresponding bit
//bit high will select the A/D input function, clearing
//the bit to zero will select either the I/O or other
//pin-shared function.
	_smod=0b11010000;	
// [7:5]=100->fH/16,110-->fH/4; [4]=1:FSTEN, fast wakeup; [1]=1/0:IDLEN-->IDEL/SLEEP;[0]=1-->fH/? or fL
//	_smod=0b11000000;	// [7:5]=100->fH/16,110-->fH/4; [0]=1-->fH/? or fL
//	_smod=0b00000000; 	// [7:5]=000,001->fL,[0]=1-->fH, [1]=0:SLEEP	
	_pdc = 0 ; 	// 設定 PD 埠為輸出; seven-segment LEDs
	_pec = 0 ; 	// 設定 PE 埠為輸出; PE[3:0]=com port PE[7:4]:keyboard col
	Led7_com = led7com[0] ; 	// initial common port
    Led7_seg = led7seg[0];	// initial LED port	
//
//	initial 4-digit seven-segment LEDs : 0000
	dig_bcd[0]=0;
	dig_bcd[1]=0;
	dig_bcd[2]=0;
	dig_bcd[3]=0;
//
//_tm2c1 register: 
//[7:6]=00:comp match mode, [5:4]=11:toggle, [3]=0:init low, 
//          [2]=1:invert output, [0]=1:comp_A clr	
//[7:6]=10:PWM, [5:4]=10:PWM output, [3]=0/1:active low/high, 
//          [2]=1:invert output, [1]=0:CCRP/per(CCPA/duty), [0] not used	
//[7:6]=11:Timer/Counter, [5:4]=unused, [3:0] not used	
//In the Timer/Counter Mode, the TM output pin control must be disabled.
//	_t2cp0=0;  //t2cp0:pc3, t2cp1:pc4
//	_t2cp1=0;  //t2cp0:pc3, t2cp1:pc4
// Timer 2
	_tm2al=t2_int_L;
	_tm2ah=t2_int_H;
//	_tm2rp=0;
	_tm2c0=0b01000000; //[6:4]=100:ftbc, [2:0] not used for TM2
	_tm2c1=0b00000001; //[7:6]=00:comp match mode, [5:4]=11:toggle, [3]=0:init low, [2]=1:invert output, [0]=1:comp_A clr	
//	_tm2rp=0;	// overflow
	
	_t2on=1;
	bcd_H=0x59;		//0x00;
	bcd_L=0x00;		//0x00;
	_acerl =0b00000000;  //POR:0b11111111, "0":I/O, "1":AN
	_pac |= 0b00001000;  // "1":input, "0":output 
	_papu |= 0b00001000; // pull up PA3;  PA3/INT0/C0-/AN3
	_c0sel=0;  //set PA3/C0-, PA2/C0+ as I/O rather than C0-, C0+
	//_c1sel=0;  //set PC3/C1-, PC2/C1+ as I/O rather than C1-, C`+
	_integ=0b00000101;  // INT0-->[1:0]=10-->falling edge;=01-->rising edge
	_int0e=1;
	_int0f=0;
	_mf0e=1;	//multi-function interrupt 0 
	_t2ae=1;
	_mf0f=0;
	_t2af=0;

	_emi=1;  //INTC0[0]:global interrupt	
	
	while(1)
	{
			
// scanning display of 4-digit seven-segment LEDs				
		for( dig_pos = 0 ; dig_pos < 4 ; dig_pos++ )
		{	
			Led7_com = 0x00 ; 	// common-cathode pins off; 0-->off, 1-->on
   			Led7_seg = led7seg[dig_bcd[dig_pos]];	// change the LED pattern
			Led7_com = led7com[dig_pos] ; 	// change the seleted digit position
   			delay(20-(dig_pos/3)*10);  //longer delay --> higher LED brightness	   
		}	
			
	}	
}
