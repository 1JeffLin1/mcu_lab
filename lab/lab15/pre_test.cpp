// This is a PN9 code generation program :
// Normal: 7-segment display = BCD codes of (ADC value of VR1)/8 :
// strong lighting: Normal status, display ADC value/4 (decimal)
// Weak lighting: PN code genration on, display PN code value (decimal)
/// Interconnect lines on the Practice Board :
//   (1) VR --> MCU :  PA0 --> PA0, PA1 --> PA1
//	 (2) CDS --> MCU : PA2 --> PA2
//   (3) 7-SEG LEDs --> MCU : PD --> PD, PE --> PE
//   (4) 3-color LED --> MCU : Bled --> PC0, Gled --> PC1, Rled --> PC2, gnd --> PC5
//
#include "HT66F50.h"

// #define LedPort8x8 _pd // pd 埠
#define Led7_com _pe // pd 埠
#define Led7_seg _pd // pe 埠
// #define divf_no 32767 // 65536
#define divf_no 32768 / 1						  // clock freq =32.768 kHz
const unsigned char t2_int_H = divf_no / 1 / 256; // 16
const unsigned char t2_int_L = divf_no / 1 % 256; // 255

const unsigned char led7com[4] = {0x08, 0x04, 0x02, 0x01};
const unsigned char led7seg[11] = {0x3f, 0x06, 0x5b, 0x4f, 0x66,
								   0x6d, 0x7d, 0x07, 0x7f, 0x67, 0x00};

//
unsigned char PN9_LB, PN9_HB;
unsigned char pn9_hbit;
unsigned dig_bcd[4];
unsigned int disp_val;
unsigned char PN_code_sel; // 0: PN10; 1:PN13;
// 弱光1, 強光0

void tm2_timer(void);
// enter the TM2 interrupt vector per 0.5 second
void __attribute((interrupt(0x14))) isr_TM2(void)
{

	tm2_timer();
	//	_mf0f=0;
	_t2af = 0;
}

void delay(unsigned char n) // delay function
{
	unsigned char idy1, idy2;

	for (idy1 = 0; idy1 < n; idy1++) //
	{
		GCC_NOP();
		for (idy2 = 0; idy2 < 16; idy2++)
		{
			GCC_NOP();
		}
	}
}

void tm2_timer(void) // delay function
{
	unsigned char Tled_color;
	if (PN_code_sel == 1)
	{
		// PN9: x^9+x^5+1  : register 8~0 : reg8 input = reg5 output ^ reg0 output
		// bit9:bit0 of PN9_LB, bit5:bit4 of PN9_LB
		/*
		pn9_hbit=((PN9_LB>>4)^PN9_LB)&0x01; //get PN9_LB(4)^PN9_LB(0)
		PN9_LB=(PN9_HB<<7)|(PN9_LB>>1); //PN9 shift right by 1
		PN9_HB=pn9_hbit;
		*/
		// PN10:x^10+x^7+1  : register 9~0 : reg9 input = reg7 output ^ reg0 output
		// bit10:bit0 of PN9_LB, bit7:bit3 of PN9_LB
		/*
		pn9_hbit=((PN9_LB>>3)^PN9_LB)&0x01; //get PN9_LB(3)^PN9_LB(0) at 0-site
		pn9_hbit=(pn9_hbit<<1); //put the generated bit to PN9_HB(1)
		PN9_LB=(PN9_HB<<7)|(PN9_LB>>1); //PN9 shift right by 1
		PN9_HB=pn9_hbit|(PN9_HB>>1);
		*/
		// PN11:x^11+x^9+1
		// bit11:bit0 of PN9_LB, bit9:bit2 of PN9_LB
		/*
		pn9_hbit=((PN9_LB>>2)^PN9_LB)&0x01; //get PN9_LB(2)^PN9_LB(0) at 0-site
		pn9_hbit=(pn9_hbit<<2); //put the generated bit to PN9_HB(2)
		PN9_LB=(PN9_HB<<7)|(PN9_LB>>1); //PN9 shift right by 1
		PN9_HB=pn9_hbit|(PN9_HB>>1);
		*/
		// PN12:x^12+x^11+x^10+x^4+1
		// bit12:bit0 of PN9_LB(1st bit), bit11:bit1 of PN9_LB, bit10:bit2 of PN9_LB, bit4:bit0 of PN9_HB(9th bit)
		/*
		pn9_hbit=((PN9_HB>>0)^(PN9_LB>>2)^(PN9_LB>>1)^PN9_LB)&0x01; //get PN9_HB(0)^PN9_LB(2)^PN9_LB(1)^PN9_LB(0) at 0-site
		pn9_hbit=(pn9_hbit<<3); //put the generated bit to PN9_HB(3)
		PN9_LB=(PN9_HB<<7)|(PN9_LB>>1); //PN9 shift right by 1
		PN9_HB=pn9_hbit|(PN9_HB>>1);
		*/
		// PN13:x^13+x^12+x^11+x^8+1
		// bit13:bit0 of PN9_LB(1st bit), bit12:bit1 of PN9_LB, bit11:bit2 of PN9_LB, bit8:bit5 of PN9_LB
		/*
		pn9_hbit=((PN9_LB>>5)^(PN9_LB>>2)^(PN9_LB>>1)^PN9_LB)&0x01; //get PN9_LB(5)^PN9_LB(2)^PN9_LB(1)^PN9_LB(0) at 0-site
		pn9_hbit=(pn9_hbit<<4); //put the generated bit to PN9_HB(4)
		PN9_LB=(PN9_HB<<7)|(PN9_LB>>1); //PN9 shift right by 1
		PN9_HB=pn9_hbit|(PN9_HB>>1);
		*/

		// PN10:x^10+x^7+1  : register 9~0 : reg9 input = reg7 output ^ reg0 output
		// bit10:bit0 of PN9_LB, bit7:bit3 of PN9_LB
		pn9_hbit = ((PN9_LB >> 3) ^ PN9_LB) & 0x01; // get PN9_LB(3)^PN9_LB(0) at 0-site
		pn9_hbit = (pn9_hbit << 1);					// put the generated bit to PN9_HB(1)
		PN9_LB = (PN9_HB << 7) | (PN9_LB >> 1);		// PN9 shift right by 1
		PN9_HB = pn9_hbit | (PN9_HB >> 1);

		// PN7:x^7+x^6+1        not sure its right or fales
		//		pn9_hbit = ((PN9_LB >> 1) ^ PN9_LB) & 0x01; // get PN9_LB(3)^PN9_LB(0) at 0-site
		//		PN9_LB = (PN9_HB << 7) | (PN9_LB >> 1);		// PN9 shift right by 1
		//		PN9_HB = pn9_hbit | (PN9_HB >> 1);

		// PN8                  not sure its right or fales
		//		pn9_hbit = ((PN9_LB >> 4) ^ (PN9_LB >> 3) ^ (PN9_LB >> 2) ^ PN9_LB) & 0x01; // get PN9_LB(3)^PN9_LB(0) at 0-site
		//		PN9_LB = (PN9_HB << 7) | (PN9_LB >> 1);										// PN9 shift right by 1
		//		PN9_HB = pn9_hbit | (PN9_HB >> 1);
	}
	/*
		else
		{



		}
	*/
	if (PN_code_sel == 1)
	{
		disp_val = PN9_HB * 256 + PN9_LB;

		// Get BCD values: dig_bcd[3]~dig_bcd[0]
		dig_bcd[0] = disp_val % 10;
		dig_bcd[1] = (disp_val / 10) % 10;
		dig_bcd[2] = (disp_val / 100) % 10;
		dig_bcd[3] = (disp_val / 1000);
		if (dig_bcd[3] == 0)
		{
			dig_bcd[3] = 10;
			if (dig_bcd[2] == 0)
			{
				dig_bcd[2] = 10;
				if (dig_bcd[1] == 0)
				{
					dig_bcd[1] = 10;
				}
			}
		}

		// 		Tled_color = disp_val%1 + 1;
		Tled_color = disp_val % 7 + 1;
		_pc = (_pc & 0b11111000) | Tled_color;
	}
}
//

void main(void) // 主函式
{

	long long int domain[4] = {4096, 12288, 28672, 61439}; // -4096
	//  long long int domain  [4] = {8192, 16384, 32768, 65535};
	unsigned char adc0h, adc0l, adc2h, adc2l, adc1h, adc1l, dig_pos; // adc1h,adc1l;
	int adc0_val, adc2_val, adc1_val;
	// The ACERH and ACERL control registers contain the ACER11~ACER0 bits
	// which determine which pins on PF1, PF0, PE1, PE0, and PORTA are used
	// as analog inputs for the A/D converter input and which pins are not
	// to be used as the A/D converter input. Setting the corresponding bit
	// bit high will select the A/D input function, clearing
	// the bit to zero will select either the I/O or other
	// pin-shared function.
	_smod = 0b11100000;	 // [7:5]=100->fH/16,110-->fH/4; [0]=1-->fH/? or fL
						 //	_smod=0b00000000; 	// [7:5]=000,001->fL,[0]=1-->fH, [1]=0:SLEEP
	_pdc = 0;			 // 設定 PD 埠為輸出
	_pec = 0;			 // 設定 PE 埠為輸出
						 //
						 //  _tm2c1=0b00110001;
						 //[7:6]=00:comp match mode, [5:4]=11:toggle, [3]=0:init low,
						 //          [2]=1:invert output, [0]=1:comp_A clr
						 //[7:6]=10:PWM, [5:4]=10:PWM output, [3]=0/1:active low/high,
						 //          [2]=1:invert output, [1]=0:CCRP/per(CCPA/duty), [0] not used
						 //[7:6]=11:Timer/Counter, [5:4]=unused, [3:0] not used
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
	_tm2c0 = 0b01000000; //[6:4]=100:ftbc, [2:0]=000:overflow
	_tm2c1 = 0b00000001; //[7:6]=00: Compare Match Output Mode; [0]=1/0-->Comp_A/_P clear
	_tm2al = t2_int_L;
	_tm2ah = t2_int_H;
	//	_tm2rp=0;	// overflow
	// In the Timer/Counter Mode, the TM output pin control must be disabled.
	_t2cp0 = 0; // t2cp0:pc3, t2cp1:pc4
	_t2cp1 = 0; // t2cp0:pc3, t2cp1:pc4
	//	_t2on=1;
	//
	_tm0c0 = 0b00100000; //[7]:pause, [6:4]=100:ftbc; 000: fSYS/4; 001:fSYS; 010:fH/16, [3]:t0on, [2:0]:ccrp=000:overflow
	//_tm0c1=0b00110001; //[7:6]=00:comp match mode; [5:4]:toggle output; [3]=0:initial low; [2]:invert; [0]=1:comp_A clr
	_tm0c1 = 0b10110001; //[7:6]=10:PWM; [5:4]:PWM output; [3]=0:active low; [2]:invert; [1]=0:CCRP(period)/CCRA(duty)
	_tm0al = 0xff;
	_tm0ah = 0x01;
	_pc5 = 0; // TM0 output noninverter
	// In the Timer/Counter Mode, the TM output pin control must be disabled.
	_t0cp0 = 0; // t0cp0:pa0, t0cp1:pc5
	_t0cp1 = 1; // t0cp0:pa0, t0cp1:pc5
	//
	//_pac &= 0b11111000 ; 	// 設定PC0, PC1, PC2為輸出
	//	_acerl =_acerl | 0b00000111; // 設定 PA0, PA1, PA2為ADC輸入
	_acerl = 0b00000111; // 設定 PA0, PA1, PA2為ADC輸入
	//_pac |= 0b00000111;  // set PA0 input
	//
	//	set PC port to be used as digital I/O
	//	_pcc=0;	//PC7~PC0為輸出
	_pcc = _pcc & 0b11011000;
	_t1bcp0 = 0; // pc0/TP1B_0 is disconnected to TM1
	_t1bcp1 = 0; // pc1/TP1B_1 is disconnected to TM1
	_c1sel = 0;	 // set PC2/C1+ and PC3/C1- and are disconnected c1+ and c1-
	//	_t2cp0=0;  // pc3/TP2_0 is disconnected to TM2
	//	_t2cp1=0;  // pc4/TP2_1 is disconnected to TM2
	_t1bcp2 = 0; // pc5/TP0_1/TP1B_2 is disconnected to TM1
				 //	_t0cp1=0;  // pc5/TP0_1/TP1B_2 is disconnected to TM0
				 //
				 //  set PA port to be used as digital I/O
				 // 	_c0os=0; 	// pa0 is disconnected to c0x and purly acts as an I/O pin
				 //  _t0cp0=0; 	// pa0 is disconnected to TM0 and purly acts as an I/O pin
				 //  _t1acp0=0;	// pa1 is disconnected to TM1
				 //	_c0sel=0; 	// pa2 and pa3 are disconnected c0+ and c0-, respectively, and act as I/O pins
				 //	_c1os=1;  	// pa5 is disconnected to c1x and purly acts as an I/Opin
				 //
	//_acerl=0b00000001;	// PA1: AN輸入; 0:設定 PA 埠為一般I/O，非AN類比輸入

	//
	// ADC setting
	_adcr0 = 0b00010000; // [5]=1:ADOFF ADC OFF;[4]=1:ADRH[3:0]-ADRL[7:0]; [2:0]=000:AN0
	_adcr1 = 0b00000100; // [2-0]=100: fSYS/16; 101: fSYS/32; 110: fSYS/64
	_adoff = 0;			 // power on ADC
	//	initial 4-digit seven-segment LEDs : 0000
	dig_bcd[0] = 10;
	dig_bcd[1] = 10;
	dig_bcd[2] = 10;
	dig_bcd[3] = 10;
	//	_adoff=1; power on ADC
	//
	_emi = 1;
	_mf0e = 1;
	_mf0f = 0;
	_t2ae = 1;
	_t2af = 0;
	_t2on = 1;
	//_t0on=0;
	// PN9_run=0;
	//_pa0=0;
	PN_code_sel = 0;
	//_t2on=1;
	_t0on = 1;
	// initial Pseudorandom Number: 5

	///////////////////////////
	PN9_LB = 5;
	PN9_HB = 0;
	///////////////////////////

	while (1)
	{

		//		_adcr0=0b00010000; // [5]=1:ADOFF ADC OFF;[4]=1:ADRH[3:0]-ADRL[7:0]; [2:0]=000:AN0
		//		_adoff=0; //power on ADC
		//		_ace0=1;
		//		delay(2);
		//		_start=0;
		_adcr0 = 0b00010010; // [5]=1:ADOFF ADC OFF;[4]=1:ADRH[3:0]-ADRL[7:0]; [2:0]=000:AN0
		_start = 1;
		_start = 0;
		while (_eocb)
		{
		};
		adc2h = _adrh;
		adc2l = _adrl;
		adc2_val = adc2h * 256 + adc2l;

		if (adc2_val > 2000)
		{
			PN_code_sel = 0;
			_adcr0 = 0b00010000; // [5]=1:ADOFF ADC OFF;[4]=1:ADRH[3:0]-ADRL[7:0]; [2:0]=000:AN0
			_start = 1;
			_start = 0;
			while (_eocb)
			{
			};
			adc0h = _adrh;
			adc0l = _adrl;
			adc0_val = adc0h * 256 + adc0l;
			dig_bcd[0] = adc0_val % 10;
			dig_bcd[1] = (adc0_val / 10) % 10;
			dig_bcd[2] = (adc0_val / 100) % 10;
			dig_bcd[3] = (adc0_val / 1000);
			if (dig_bcd[3] == 0)
			{
				dig_bcd[3] = 10;
				if (dig_bcd[2] == 0)
				{
					dig_bcd[2] = 10;
					if (dig_bcd[1] == 0)
					{
						dig_bcd[1] = 10;
					}
				}
			}
			_pc = (_pc & 0b11111000);
		}
		else
		{
			// 類比ad1輸入，要調整_adcr0
			_adcr0 = 0b00010001; // [5]=1:ADOFF ADC OFF;[4]=1:ADRH[3:0]-ADRL[7:0]; [2:0]=000:AN0
			_start = 1;
			_start = 0;
			while (_eocb)
			{
			};

			//      [4] = {8192, 16384, 32768, 65535}

			adc1l = _adrl; //   /5不要爆掉
			adc1h = _adrh;
			adc1_val = adc1h * 256 + adc1l;

			if (adc1_val <= 4095 && adc1_val >= 3072)
			{
				_tm2al = (domain[3] + 4096) % 256;
				_tm2ah = (domain[3] + 4096) / 256;
			}
			else if (adc1_val <= 3071 && adc1_val >= 2048)
			{
				_tm2al = (domain[2] + 4096) % 256;
				_tm2ah = (domain[2] + 4096) / 256;
			}
			else if (adc1_val <= 2047 && adc1_val >= 1024)
			{
				_tm2al = (domain[1] + 4096) % 256;
				_tm2ah = (domain[1] + 4096) / 256;
			}
			else if (adc1_val <= 1023 && adc1_val >= 0)
			{
				_tm2al = (domain[0] + 4096) % 256;
				_tm2ah = (domain[0] + 4096) / 256;
			}

			//		   	_tm0al=adc1l;
			//	        _tm0ah=adc1h;
			PN_code_sel = 1;
		}

		for (dig_pos = 0; dig_pos < 4; dig_pos++)
		{
			Led7_com = 0x00;						// 8x8 LED X-axis (PD) OFF
													//			delay(1);
			Led7_seg = led7seg[dig_bcd[dig_pos]];	// 8x8 LED Y-axis (PE) ON
			Led7_com = Led7_com | led7com[dig_pos]; // 8x8 LED X-axis (PD) OFF
			if (dig_pos < 3)
				delay(10);
			else
				delay(30);
		}
	}
}
