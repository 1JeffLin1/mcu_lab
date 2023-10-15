// This is a Timer Module Comp/Match program for playing music:
//      fsys=12MHz : using HIRC/12MHz
//          Configuration Oprions: High Freq OSC/HIRC,  HIRC frequency selection/12MHz
//      Standard TM2之compare match功能運用於音樂盒之範例程式
//      使用TM2的t2cp1控制之輸出PC4接至蜂鳴器來發出音樂聲
// music_tone[ ]:陣列內是簡譜, music_beat[ ]是對應簡譜每個音的節拍長度(如4:全音符, 3:3/4音符, 2:1/2音符
// Interconnect lines on the Practice Board :
//   (1) Buzzle --> MCU : PC4 --> PC4, VCC-->VCC, GND-->GND		//generate music
//   (2) CDS(光敏) --> MCU : PA2 --> PA2, VCC-->VCC, GND-->GND	//switch music
//
//  12e6/261.63=45866-->change the current state to its complementary state whenever compare match occurs
//                   -->counting number = 45866/2=22933 = (89)(149)
//  12MHz, C:261.63Hz(22933), D:293.66Hz(20432)  E:329.63Hz(18202)  F:349.23Hz(17181)
//         G:392Hz(15306)     A:440Hz(13636)     B:493.88Hz(12149)  C:523.25Hz(11467)

// const unsigned short tone[8] = {22933, 20432, 18202, 17181, 15306, 13636, 12149, 11467} ;
//                                 0  1(C) 2(D) 3(E)  4(F) 5(G) 6(A) 7(B) 1(C)

#include "HT66F50.h"
// tone frequency=12MHz/(tm2ah, tm2al)/2;  (tm2ah, tm2al) =12MHz/(tone frequency)/2
const unsigned char tone_HB[9] = {0, 89, 79, 71, 67, 59, 53, 47, 44};
const unsigned char tone_LB[9] = {0, 149, 208, 26, 29, 202, 68, 117, 203};
// music_tone[ ]:陣列內是簡譜, music_beat[ ]是對應簡譜每個音的節拍長度(如4:全音符, 3:3/4音符, 2:1/2音符
// const unsigned char music_tone[24] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 5, 3, 3, 4, 2, 2, 1, 2, 3, 4, 5, 5, 5} ;
// const unsigned char music_beat[24] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 4, 2, 2, 4, 2, 2, 4, 2, 2, 2, 2, 2, 2, 4} ;
//
const unsigned char music1_tone[59] = {5, 3, 3, 0, 4, 2, 2, 0, 1, 2, 3, 4, 5, 5,
									   5, 0, 5, 3, 3, 0, 4, 2, 2, 0, 1, 3, 5, 5, 3,
									   0, 2, 2, 2, 2, 2, 3, 4, 0, 3, 3, 3, 3, 3,
									   4, 5, 0, 5, 3, 3, 0, 4, 2, 2, 0, 1, 3, 5, 5, 1};
const unsigned char music1_beat[59] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
									   4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4,
									   2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2,
									   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4};
//
const unsigned char music2_tone[50] = {1, 1, 5, 5, 6, 6, 5, 0, 4, 4, 3, 3, 3,
									   2, 2, 1, 0, 5, 5, 4, 4, 3, 3, 2, 0,
									   5, 5, 4, 4, 3, 3, 2, 0, 1, 1, 5, 5,
									   6, 6, 5, 0, 4, 4, 3, 3, 3, 2, 2, 1, 0};
const unsigned char music2_beat[50] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 2,
									   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
									   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
									   4, 4, 4, 4, 4, 4, 4, 2, 2, 4, 4, 4, 4};
// machine periosd = 4/12E6; jump: 2 machine period
// 3 GCC_NOP() in the 2nd for-loop:
//   {2_move+[2_move+(3_nop+1_minus1+1_jump)*30+1_minus1+1_jump]*n}*4/12E6
//   ={2+[2+(3+1+2)*30+1+2]*n}*4/12E6=(2+185n)*4/12E6=(740n+8)/12E6
// 4 GCC_NOP() in the 2nd for-loop:
//   {2_move+[2_move+(4_nop+1_minus1+1_jump)*30+1_minus1+1_jump]*n}*4/12E6
//   ={2+[2+(4+1+2)*30+1+2]*n}*4/12E6=(2+215n)*4/12E6=(860n+8)/12E6
// 5 GCC_NOP() in the 2nd for-loop:
//   {2_move+[2_move+(5_nop+1_minus1+1_jump)*30+1_minus1+1_jump]*n}*4/12E6
//   ={2+[2+(5+1+2)*30+1+2]*n}*4/12E6=(2+245n)*4/12E6=(980n+8)/12E6
void delay(unsigned int n)
{
	unsigned int idy1, idy2;

	for (idy1 = 0; idy1 < n; idy1++) //
	{
		//		   	GCC_NOP();
		//		   	GCC_NOP();
		for (idy2 = 0; idy2 < 30; idy2++)
		{
			GCC_NOP();
			GCC_NOP();
			GCC_NOP();
			// GCC_NOP();	//
			// GCC_NOP();	//
		}
	}
}
// machine periosd = 4/12E6
//  [2_move+(2_move+185*250+1_minus1+1_jump)*n+1_minus1+1_jump]*4/12E6
//       =[2+(2+185*250+1+2)*n+1+2]*4/12E6=[2+46255n+3]*4/12E6-->0.0154*n s
//  [2+(2+185*200+1+2)*n+1+2]*4/12E6=[37005n+5]*4/12E6-->0.0123*n s
//  [2+(2+245*200+1+2)*n+1+2]*4/12E6=[42804n+5]*4/12E6-->0.0163*n s
//  [2+(2+245*250+1+2)*n+1+2]*4/12E6=[53504n+4]*4/12E6-->0.0204*n s
//  If 1 beat = quarter not (1/4音符), 1 whole note = 4 beats
//  Time that 1 whole note lasts for is
//    (4*4+1)*0.0588s=1s; (4*4+1)*0.0154s=0.2618s; (4*4+1)*0.0123s=0.2091s; (4*4+1)*0.0163=0.2771s
//	  (4*20+1)*0.0123s=1s; (4*20+1)*0.0154s=1.2474s; (4*20+1)*0.0123s=1s; (4*20+1)*0.0163=1.3203s
void delay_m(unsigned int n)
{
	unsigned int i;

	for (i = 0; i < n; i++)
	{
		delay(200);
	}
}
//
void main(void) // 主函式
{
	unsigned char i, music_length, music_no, beat; // music_no
	// unsigned char music_beat[60], music_tone[60];
	//	unsigned char k, d_match;  // don't use unsigned int ;
	//	unsigned char tone_HB[8], tone_LB[8];  // don't use unsigned int ;
	//	_acerl =0;  //POR:0b11111111,
	// The ACERH and ACERL control registers contain the ACER11~ACER0 bits
	// which determine which pins on PF1, PF0, PE1, PE0, and PORTA are used
	// as analog inputs for the A/D converter input and which pins are not
	// to be used as the A/D converter input. Setting the corresponding bit
	// bit high will select the A/D input function, clearing
	// the bit to zero will select either the I/O or other
	// pin-shared function.
	_smod = 0b10000001;			  // [7:5]=100->fH/16,110-->fH/4; [0]=1/0-->fH/?_or_fL_[7:5]
								  //	_smod=0b00000000; 	// [7:5]=000,001->fL,[0]=1-->fH/?_or_fL, [1]=1/0:IDLE/SLEEP mode
								  //
								  /*multifunctional I/O pins: must close high-priority functions to assign a desired function to a I/O pin */
								  /* the rightmost function is the highest-priority function                                               */
								  //  PA0/C0X/TP0_0/AN0 ; PA2/TCK0/C0+/AN2 ; PA3/INT0/C0-/AN3 ;
								  //  PC2/TCK2/PCK/C1+ ;  PC3/PINT/TP2_0/C1- ; PC4/[INT0]/[PINT]/TCK3/TP2_1
								  //  PC5/[INT1]/TP0_1/TP1B_2/[PCK]  ; _t1bcp2 default=0;
	_acerl = _acerl & 0b11111011; // 設定 PA2 埠為數位埠
	_c0sel = 0;					  // _cp0c[7]=c0sel=0->設定PA3,PA2不為比較器C0的輸入c0-,C0+
	_pac2 = 1;					  // 設定 PA2 埠為輸入

	_c1sel = 0; // disable PC3/PINT/TP2_0/C1- as I/O
	_pcc3 = 0;	// 設定 PC3 埠為輸出
	_pcc4 = 0;	// 設定 PC4 埠為輸出
	// tm2c0 控制計數器的時脈， 開始/暫停/停止計時
	// tm2c1 控制模式(比對吻合, 輸入捕捉, 等等), 輸出功能選擇
	_tm2c0 = 0b00010000; //[6:4]=100:ftbc, 000: fSYS/4, 001: fSYS=fH; [3]:t20n; [2:0]=000:overflow, not used
	_tm2c1 = 0b00110011; // toggle
	_pc4 = 0;
	//_tm2c1 register: function setting
	//[7:6]=00:comp match mode, [5:4]=11:toggle, [3]=0:init low,
	//          [2]=1:invert output, [0]=1/0:comp_A clr/comp_P clr
	//[7:6]=10:PWM, [5:4]=10:PWM output, [3]=0/1:active low/high,
	//          [2]=1:invert output, [1]=0:CCRP/per(CCPA/duty), [0] not used
	//[7:6]=11:Timer/Counter, [5:4]=unused, [3:0] not used
	_tm2al = 0;
	_tm2ah = 255;
	_tm2rp = 10;
	_t2cp0 = 0; // t2cp0:pc3, t2cp1:pc4
	_t2cp1 = 1; // t2cp0:pc3, t2cp1:pc4
	_t2on = 0;

	while (1)
	{

		// save tones and beats of the 1st song to music_tone[] and music_beat[]
		if (_pa2 == 1) // light
		{
			music_no = 1; // record the number of the currently selected music, 0(dark) or 1(light)
			music_length = 59;
			/*
			for(j=0; j<=music_length-1;  j++)
			{
				music_tone[j]=music1_tone[j];
				music_beat[j]=music1_beat[j];
			}
			*/
		}
		else // dark
		{
			music_no = 0; // record the number of the currently selected music, 0(dark) or 1(light)
			music_length = 50;
			/*
			for(j=0; j<=music_length-1;  j++)
			{
				music_tone[j]=music2_tone[j];
				music_beat[j]=music2_beat[j];
			}
			*/
		}
		int judge;
		// play music by using music_tone[] and music_beat[]
		for (i = 0; i <= music_length - 1; i++) // 23就是music_tone陣列範圍24減一(0 ~ 23)
		{
			judge = 0;
			if (_pa2 != music_no)
			{
				judge = 1;
				break;
			}
			if (music_no == 1)
			{
				_tm2al = tone_LB[music1_tone[i]]; // tone frequency=12MHz/(tm2ah, tm2al)/2
				_tm2ah = tone_HB[music1_tone[i]]; //(tm2ah, tm2al) =12MHz/(tone frequency)/2
				beat = music1_beat[i];
			}
			if (music_no == 0)
			{
				_tm2al = tone_LB[music2_tone[i]]; // tone frequency=12MHz/(tm2ah, tm2al)/2
				_tm2ah = tone_HB[music2_tone[i]]; //(tm2ah, tm2al) =12MHz/(tone frequency)/2
				beat = music2_beat[i];
			}
			_t2on = 1;
			delay_m(beat * 4); //(music_beat[i]*4+1)*0.0123s; music_beat[i]=4-->0.2091s
			// delay_m(beat*20); //(music_beat[i]*4+1)*0.0123s; music_beat[i]=4-->0.2091s
			// delay_m(music_beat[i]*20); //(music_beat[i]*20+1)*0.0123s; music_beat[i]=4-->1s
			_t2on = 0;
			// check whether music_no is nor equal to _pa2.
			// If yes, jump away from the for-loop(i.e., break)
			delay_m(1);
		}
		if (judge != 1)
		{
			delay_m(250);
		}
	}
}