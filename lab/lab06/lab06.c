// This is a Analog-to-digital conversion (ADC) program :
// This is a control program of the ADC operation for the inputs of PA0~PA2
// IF PA2(CDS light)>2*PA0, then the alarm is on
// IF PA2(CDS light)<PA0, CPU goes into sleep mode
// IF (PA2(CDS light)>=PA0) && (PA2(CDS light)<=2*PA0), LED PC0 on
// Display mode:  automatically display adc value in turn
// Push the "7", CPU will wake up
// PD, PE for 7seg LED, PA0~PA2 for ADC, PC5 for alarm
//
// Interconnect lines on the Practice Board :
//   (1) BUZZER --> MCU : PC4 --> PC5
//   (2) VR --> MCU :  PA0 --> PA0,  PA1 --> PA1
//   (3) 7-SEG LEDs --> MCU : PD --> PD, PE --> PE
//	 (4) CDS --> MCU : PA2 --> PA2
//	 (5) 4*4KEY --> MCU : PA1 --> PA5 ("7") NOTE: GND JUMP	//push the button to Wakeup CPU
// In the assigned exercise,
//    4*4KEY --> MCU : PA1 --> PA5 ("7"), PA2 --> PA4 ("B"),  PA3 --> PA3 ("F") NOTE: GND JUMP

#include "HT66F50.h"

#define Led7_com _pe // pe 埠
#define Led7_seg _pd // pd 埠
#define adc_disp_sw 50
//  In Holtek C, char, int(16), short(16) --> 8 bits, long(32) --> 16 bits
const unsigned char led7com[4] = {0x08, 0x04, 0x02, 0x01};
const unsigned char led7seg[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66,
								   0x6d, 0x7d, 0x07, 0x7f, 0x67};
//                     		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x88, 0x76, 0x01 } ;

unsigned char dig_bcd[4];

//
void delay(unsigned char n) // delay function
{
	unsigned char idy1;
	unsigned char idy2;

	for (idy1 = 0; idy1 < n; idy1++) //
	{
		GCC_NOP();
		GCC_NOP();
		for (idy2 = 0; idy2 < 4; idy2++)
		{
			GCC_NOP();
			GCC_NOP();
			GCC_NOP();
		}
	}
}
//
void bin2bcd(unsigned int no) // delay function
{
	//  In Holtek C, char, int(16), short(16) --> 8 bits, long(32) --> 16 bits
	unsigned char dig[4];
	unsigned long digx, digy;
	unsigned short ii;

	dig[3] = no / 1000;
	digx = no % 1000;
	dig[2] = digx / 100;
	digy = digx % 100;
	dig[1] = digy / 10;
	dig[0] = digy % 10;
	for (ii = 0; ii < 4; ii++)
		dig_bcd[ii] = dig[ii];
}

void main(void) // 主函式
{
	//  In Holtek C, char, int(16), short(16) --> 8 bits, long(32) --> 16 bits
	unsigned char dig_pos, adc_no; // position of 4-digit seven-segment LEDs
	unsigned int adc_val;
	unsigned char adc0h, adc0l, adc1h, adc1l, adc2h, adc2l; //,adc1h, adc1l;
															//	_acerl =0;  //POR:0b11111111,
															// The ACERH and ACERL control registers contain the ACER11~ACER0 bits
	// which determine which pins on PF1, PF0, PE1, PE0, and PORTA are used
	// as analog inputs for the A/D converter input and which pins are not
	// to be used as the A/D converter input. Setting the corresponding bit
	// bit high will select the A/D input function, clearing
	// the bit to zero will select either the I/O or other
	// pin-shared function.
	_smod = 0b11000000;	   // [7:5]=100->fH/16,110-->fH/4; [0]=1-->fH/? or fL
						   //	_smod=0b00000000; 	// [7:5]=000,001->fL,[0]=1-->fH, [1]=0:SLEEP
	_pdc = 0;			   // 設定 PD 埠為輸出; seven-segment LEDs
	_pec = 0;			   // 設定 PE 埠為輸出; PE[3:0]=com port PE[7:4]:keyboard col
	Led7_com = led7com[0]; // initial common port
	Led7_seg = led7seg[0]; // initial LED port
						   //
						   // set PC port to be used as digital I/O
						   /*	_pcc=0;		//PC7~PC0為輸出
							   _t1bcp0=0; 	// pc0/TP1B_0 is disconnected to TM1
							   _t1bcp1=0; 	// pc1/TP1B_1 is disconnected to TM1
							   _c1sel=0;  	//set PC2/C1+ and PC3/C1- and are disconnected c1+ and c1-
							   _t2cp0=0;  	// pc3/TP2_0 is disconnected to TM2
							   _t2cp1=0;  	// pc4/TP2_1 is disconnected to TM2
							   _t1bcp2=0; 	// pc5/TP0_1/TP1B_2 is disconnected to TM1
							   _t0cp1=0;  	// pc5/TP0_1/TP1B_2 is disconnected to TM0
						   */
	_tm0c0 = 0b01000000;   //[7]:pause, [6:4]=100:ftbc, [3]:t0on, [2:0]:ccrp=000:overflow
	_tm0c1 = 0b00111001;   //[7:6]=00:comp match mode; [5:4]:toggle output; [3]=1:initial high; [2]:invert; [0]=1:comp_A clr
						   //	_tm0c1=0b00111001; //[7:6]=10:PWM; [5:4]:PWM output; [3]=1:initial high; [2]:invert; [1]=0:CCRP(period)/CCRA(duty)
						   //  _tm2c0=0b00110001;
						   //[7]=T2PAU; [6:4]=TnCK2~TnCK0; [3]=T2ON; [2:0]=T2RP2~T2RP0(TM0.TM3)/unused(TM2)
						   //  _tm2c1=0b00110001;
						   //[7:6]=T2M1,T2M0; [5:4]=T2IO1,T2IO0; [3]=T2OC; [2]=T2POL; [1]=T2DPX; [0]=T2CCLR
						   //[7:6]=00:comp match mode, [5:4]=11:toggle, 01:low, 10:high, [3]=0:init low, [0]=1:comp_A clr
						   //[7:6]=01:Capture Input Mode(STM) /or undefined(CTM),
						   //   [5:4]=00:rising/01:falling/10:rising&falling/11:input capture disable
						   //   when capturing(input rising or falling)-->on-going counting value is stored in CCRA
						   //[7:6]=10:PWM/single pulse, [5:4]=10:PWM output, [3]=0/1:active low/high,
						   //   [2]=0/1:noninvert/invert, [1]=0:CCRP/per(CCPA/duty), [0] not used
						   //  single pulse: T2ON:0->1 active --- T2ON:1->0 or match CCRA finish single pulse
						   //   [5:4]=T2IO1,T2IO0=11 for single pulse
						   //[7:6]=11:Timer/Counter, [5:4]=unused, [3:0] not used
						   // In the Timer/Counter Mode, the TM output pin control must be disabled.
	//	_pac0 = 0 ; 	// 設定 PA0 埠為輸出
	//	_acerl =0;		// 設定 PA 埠為一般I/O，非AN類比輸入
	_pcc5 = 0; // 設定 PC5 埠為輸出 for TM0 output
	_pc5 = 0;
	_t1bcp2 = 0; // disable PC5 to act as TP1B_2. default setting : disable
	_tm0al = 255;
	_tm0ah = 0;
	_t0cp0 = 0; // t0cp0:pa0, t0cp1:pc5, pc5(PCK)-->PCK pin default disable
	_t0cp1 = 1; // t0cp0:pa0, t0cp1:pc5, pc5(PCK)-->PCK pin default disable
	//
	_pcc0 = 0; // 設定 PC0 埠為輸出
	_pc0 = 0;
	//
	/*multifunctional I/O pins: must close high-priority functions to assign a desired function to a I/O pin */
	/* the rightmost function is the highest-priority function                                               */
	//  PA0/C0X/TP0_0/AN0 ; PA1/TP1A/AN1 ; PA2/TCK0/C0+/AN2 ; PA3/INT0/C0-/AN3 ;
	//	PC0/TP1B_0/SCOM0 ; PC1/TP1B_1/SCOM1
	//  PC2/TCK2/PCK/C1+ ;  PC3/PINT/TP2_0/C1- ; PC4/[INT0]/[PINT]/TCK3/TP2_1
	//  PC5/[INT1]/TP0_1/TP1B_2/[PCK]  ; _t1bcp2 default=0;
	//
	//	set PC port to be used as digital I/O
	//	_pcc=0;	//PC7~PC0為輸出
	_t1bcp0 = 0;		 // pc0/TP1B_0 is disconnected to TM1
						 //	_t1bcp1=0; // pc1/TP1B_1 is disconnected to TM1
						 //	_c1sel=0;  //set PC2/C1+ and PC3/C1- and are disconnected c1+ and c1-
						 //	_t2cp0=0;  // pc3/TP2_0 is disconnected to TM2
						 //	_t2cp1=0;  // pc4/TP2_1 is disconnected to TM2
						 //	_t1bcp2=0; // pc5/TP0_1/TP1B_2 is disconnected to TM1
						 //	_t0cp1=0;  // pc5/TP0_1/TP1B_2 is disconnected to TM0
						 //
						 //  set PA port to be used as digital I/O
						 //  _c0os=1; 	// pa0 is disconnected to c0x and purly acts as an I/O pin
						 //  _t0cp0=0; 	// pa0 is disconnected to TM0 and purly acts as an I/O pin
						 //  _t1acp0=0;	// pa1 is disconnected to TM1
	_c0sel = 0;			 // pa2 and pa3 are disconnected c0+ and c0-, respectively, and act as I/O pins
	_c1os = 1;			 // pa5 is disconnected to c1x and purly acts as an I/Opin
						 //
						 //	Don't set pull-up connections of PA0, PA1, PA2 because these three I/O is used as
						 //    analog inputs of ADC. If the pull-up connections are set to be "1", they will be disabled.
	_acerl = 0b00000111; // PA0, PA1, PA2: AN輸入; 0:設定 PA 埠為一般I/O，非AN類比輸入
	_pac |= 0b11111111;	 // set PA3~PA5 input
	_papu |= 0b00111000; // pull up PA3, PA4, PA5
	_pawu |= 0b00100000; // enable PA5 wakeup function
						 //
						 // ADC setting
	_adcr0 = 0b00010000; // [5]=1:ADOFF ADC OFF;[4]=1:ADRH[3:0]-ADRL[7:0]; [2:0]=000:AN0
	_adcr1 = 0b00000110; // [2-0]=100: fSYS/16; 101: fSYS/32; 110: fSYS/64
	_adoff = 0;			 // power on ADC
	//	initial 4-digit seven-segment LEDs : 0000
	dig_bcd[0] = 0;
	dig_bcd[1] = 0;
	dig_bcd[2] = 0;
	dig_bcd[3] = 0;
	//	_adoff=1; power on ADC
	//
	adc_no = 0;

	//
	//
	while (1)
	{

		if (_pa5 == 1)
		{
			_smod = 0b11000000;
		}

		_adcr0 = 0b00010000; // [5]=1:ADOFF ADC OFF;[4]=1:ADRH[3:0]-ADRL[7:0]; [2:0]=000:AN0
							 //		_adoff=0; //power on ADC
							 //		_ace0=1;
							 //		delay(2);
							 //		_start=0;
		_start = 1;
		_start = 0;
		while (_eocb)
		{
		};
		adc0h = _adrh;
		adc0l = _adrl;

		_adcr0 = 0b00010001; // [5]=1:ADOFF ADC OFF;[4]=1:ADRH[3:0]-ADRL[7:0]; [2:0]=000:AN0
							 //		_adoff=0; //power on ADC
							 //		_ace0=1;
							 //		delay(2);
							 //		_start=0;
		_start = 1;
		_start = 0;
		while (_eocb)
		{
		};
		adc1h = _adrh;
		adc1l = _adrl;
		//
		//***	Insert AN1 ADC conversion herein 	***//
		//
		//		adc1h=_adrh;
		//		adc1l=_adrl;
		_adcr0 = 0b00010010; // [5]=1:ADOFF ADC OFF;[4]=1:ADRH[3:0]-ADRL[7:0]; [2:0]=000:AN0
							 //		_ace2=1;
							 //		_adoff=0; //power on ADC
							 //		_start=0;
		_start = 1;
		_start = 0;
		while (_eocb)
		{
		};
		// Choose the display data: Normal-->CDS, "B"-->VR0, "F"-->VR1
		adc2h = _adrh;
		adc2l = _adrl;

		if (adc_no / adc_disp_sw == 0) // #define adc_disp_sw 50
			adc_val = adc0h * 256 + adc0l;
		else if (adc_no / adc_disp_sw == 1)
			adc_val = adc1h * 256 + adc1l;
		else
			adc_val = adc2h * 256 + adc2l;

		//*** Check whether the "B" or "F" button is pushed 	***//
		//    If yes, assign the related ADC value to adc_val	   //

		bin2bcd(adc_val);

		//*** Modifiy the if-else statements to conform to the required conditions	***//
		// Check whether VR1 >= VR0, CDS >= VR0, and CDS <= VR1
		// If not, the buzzer is on
		_pc0 = 0;

		if (adc2h * 256 + adc2l < adc0h * 256 + adc0l) // fall asleep
		{
			_adcr0 |= 0b00100000; // [5]=1:ADOFF ADC OFF;[4]=1:ADRH[3:0]-ADRL[7:0]; [2:0]=000:AN0
			_t0on = 0;
			Led7_com = 0x00; // common-cathode pins off; 0-->off, 1-->on
			GCC_HALT();
			delay(50);
			_adcr0 &= 0b11011111; // [5]=1:ADOFF ADC OFF;[4]=1:ADRH[3:0]-ADRL[7:0]; [2:0]=000:AN0
		}
		else if ((adc2h * 256 + adc2l > (adc1h * 256 + adc1l)) | (adc1h * 256 + adc1l <= (adc0h * 256 + adc0l)))
			_t0on = 1;
		else
		{
			_t0on = 0;
		}

		//	check whether the light intensity is in the normal range
		if ((adc2h * 256 + adc2l <= (adc1h * 256 + adc1l)) && (adc2h * 256 + adc2l >= adc0h * 256 + adc0l))
			_pc0 = 1;
		else
			_pc0 = 0;

		//
		// scanning display of 4-digit seven-segment LEDs
		//		more uniform light intensity
		if (_pa4 == 0)
		{
			bin2bcd(adc0h * 256 + adc0l);

			for (dig_pos = 0; dig_pos < 4; dig_pos++)
			{
				Led7_com = 0x00;					  // common-cathode pins off; 0-->off, 1-->on
				Led7_seg = led7seg[dig_bcd[dig_pos]]; // change the LED pattern
				if (dig_pos == 0)
					Led7_seg = led7seg[dig_bcd[dig_pos]] | 0b10000000; // change the LED pattern
				Led7_com = led7com[dig_pos];						   // change the seleted digit position
				delay(56 - (dig_pos / 3) * 55);						   // longer delay --> higher LED brightness
			}
		}
		else if (_pa3 == 0)
		{
			bin2bcd(adc1h * 256 + adc1l);
			for (dig_pos = 0; dig_pos < 4; dig_pos++)
			{
				Led7_com = 0x00;					  // common-cathode pins off; 0-->off, 1-->on
				Led7_seg = led7seg[dig_bcd[dig_pos]]; // change the LED pattern
				if (dig_pos == 1)
					Led7_seg = led7seg[dig_bcd[dig_pos]] | 0b10000000; // change the LED pattern
				Led7_com = led7com[dig_pos];						   // change the seleted digit position
				delay(56 - (dig_pos / 3) * 55);						   // longer delay --> higher LED brightness
			}
		}
		else
		{
			for (dig_pos = 0; dig_pos < 4; dig_pos++)
			{
				Led7_com = 0x00;					  // common-cathode pins off; 0-->off, 1-->on
				Led7_seg = led7seg[dig_bcd[dig_pos]]; // change the LED pattern
				if (dig_pos == adc_no / adc_disp_sw)
					Led7_seg = led7seg[dig_bcd[dig_pos]] | 0b10000000; // change the LED pattern
				Led7_com = led7com[dig_pos];						   // change the seleted digit position
				delay(56 - (dig_pos / 3) * 55);						   // longer delay --> higher LED brightness
			}
		}

		//*** Change whether the adc_no has been up to the top limit	***//
		//	  If yes, reset to zero, otherwise, increase it by 1.
		if (adc_no >= (3 * adc_disp_sw - 1))
			adc_no = 0;
		else
			adc_no++;
		//
	}
}