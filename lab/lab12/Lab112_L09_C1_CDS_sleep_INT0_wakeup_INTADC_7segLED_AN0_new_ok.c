// This is a control program for ADC and external interrupt 0 
// Seven-segment LEDs are used to display values given from ADC. 
// Normally, the Seven-segment LEDs display the ADC values transferred from PA0 and PA3=1
// When light intensity is strong (PA2=1) and PA3(INT0) changes from 1 to 0, ADC0 is run.
// When light intensity is weak (PA2=0), CPU sleeps.
// When CPU sleeps, if PA3(INT0) changes from 1 to 0, CPU is waken up and ADC0 is run.
// PA3 is the INT0 pin
// PD7~PD0(a~h) and PE3~PE0(com0~3) are used to control Seven-segment LEDs
// PA0 is ADC inputs
/// Interconnect lines on the Practice Board : 
//   (1) VR --> MCU :  PA0 --> PA0,  PA1 --> PA1 ; NOTE: connect VDD GND
//	 (2) 4*4KEY --> MCU : PA3 --> PA3 ("F"); NOTE: GND JUMP
//   (3) 7-SEG LEDs --> MCU : PD --> PD, PE --> PE
//   (4) CDS --> MCU : PA2 --> PA2; NOTE: connect VDD GND
//	 PA3@MPU: PA3/INT0/C0-/AN3

#include "HT66F50.h" 

#define Led7_com _pe // pe 埠
#define Led7_seg _pd // pd 埠
//  In Holtek C, char, int(16), short(16) --> 8 bits, long(32) --> 16 bits
unsigned char led7com[4] = { 0x08, 0x04, 0x02, 0x01 } ;
unsigned char led7seg[11]= { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 
                     		0x6d, 0x7d, 0x07, 0x7f, 0x67,
                     		0x00} ;
unsigned char dig_bcd[4];  
unsigned long adc_val0;
unsigned short 	adc0h, adc0l;
//             		
//
void delay (unsigned char n)   // delay function
{
	unsigned char  idy1;
	unsigned char idy2 ;

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
//
void bin2bcd (unsigned long no)   // delay function
{
//  In Holtek C, char, int(16), short(16) --> 8 bits, long(32) --> 16 bits
	unsigned short dig[4]; 
	unsigned long digx,  digy;
	unsigned short ii ;
    
    dig[3] = no / 1000;
    digx = no % 1000;
    dig[2] = digx / 100;
    digy = digx % 100;
    dig[1] = digy / 10;  
    dig[0] = digy % 10;
    for(ii=0; ii<4; ii++) 
   	 	dig_bcd[ii] = dig[ii];
}
//
//
void __attribute((interrupt(0x04))) isr_int0(void)
{
//	_adcr0^=0b00000001; // [5]=1:ADOFF ADC OFF;[4]=1:ADRH[3:0]-ADRL[7:0]; [2:0]=000:AN0
//	_start=1;
//	_start=0;
	_int0f=0;
}
//
void __attribute((interrupt(0x1c))) isr_adc(void)
{
//	unsigned char int_acc, int_status;
//	int_acc=_acc;
//	int_status=_status;
	adc0h=_adrh;
	adc0l=_adrl;
	adc_val0=adc0h*256+adc0l;	
	bin2bcd(adc_val0);
//	_adf=0;
//	_acc=int_acc;
//	_status=int_status;	
}
//		
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

	_acerl =0b00000011;  //POR:0b11111111, "0":I/O, "1":AN
	_pac |= 0b00001100;  // "1":input, "0":output 
	_papu |= 0b00001000; // pull up PA3;  INT0
//	pa2 and pa3 : c0+ and c0-;  c0out : PA0 
//	pc2 and pc3 : c1+ and c1-;  c1out : PA5 	
	_c0sel=0;  //set PA3/C0-, PA2/C0+ as I/O
//	_c1sel=0;  //set PC3/C1-, PC2/C1+ as I/O
	_integ=0b00001010;  // [3:2][1:0]=10-->falling edge;=01-->rising edge
	_int0e=1;
	_int0f=0;
	_ade=1;
	_adf=0;
	_emi=1;  //INTC0[0]:global interrupt	
//
	_adcr0=0b00010000; // [5]=1:ADOFF ADC OFF;[4]=1:ADRH[3:0]-ADRL[7:0]; [2:0]=000:AN0
	_adcr1=0b00000001; // [2:0]=100: fSYS/16; 101: fSYS/32; 110: fSYS/64
	_adoff=0; //power on ADC
//			
/*
	_adcr0=0b00010000; // [5]=1:ADOFF ADC OFF;[4]=1:ADRH[3:0]-ADRL[7:0]; [2:0]=000:AN0
	_start=1;
	_start=0;
	while (_eocb)
	{
	};	
*/	
	while(1)
	{

		if (_pa2 == 0)
		{
			Led7_com = 0x00 ; 	// common-cathode pins off; 0-->off, 1-->on
   			Led7_seg = led7seg[10];	// change the LED pattern 
   			_adoff=1;
			GCC_HALT();			
		}
//		
		_adcr0=0b00010000; // [5]=1:ADOFF ADC OFF;[4]=1:ADRH[3:0]-ADRL[7:0]; [2:0]=000:AN0
		_start=1;
		_start=0;
//
//		while (_eocb)
//		{
//		};
//		adc0h=_adrh;
//		adc0l=_adrl;
//		adc_val0=adc0h*256+adc0l;	
//		bin2bcd(adc_val0);
			
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
