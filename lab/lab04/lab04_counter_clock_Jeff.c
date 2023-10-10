// This is a Timer Module Timer/Counter program :
//    modifying _tm2c1 to observe results
// tm2c0, tm2c1, t2cp0, t2cp1 for TM2 --> change 2 into 0 and change pc3,pc4 into pa0, pc5 for TM0,
// 12-hour AM/PM clock time :
//    display time with all dark decimal points before noon or midday (i.e., AM clock time)
//    display time with all lighting decimal points after noon or midday (i.e., PM clock time)
// Interconnect lines on the Practice Board :
//   (1) 7 segment 4 digit display --> MCU : PD-->PD, PE-->PE
//   (2) LEDs --> MCU : PC0 --> PC0		//flickering LED once per 2 seconds

#include "HT66F50.h"

#define Led7_com _pe  // pd 埠
#define Led7_seg _pd  // pe 埠
#define divf_no 32768 // 65536
#define speed_up 4    // speed up counting of timer

const unsigned char led7com[4] = {0x08, 0x04, 0x02, 0x01}; //{LSB, .. ,MSB}: PE0,PE1,PE2,PE3-->MSB~~LSB
const unsigned char led7seg[11] = {0x3f, 0x06, 0x5b, 0x4f, 0x66,
                                   0x6d, 0x7d, 0x07, 0x7f, 0x67, 0x00};

unsigned char t2_int_H = divf_no / speed_up / 256; // 16
unsigned char t2_int_L = divf_no / speed_up % 256; // 255
unsigned char t2_dL, t2_dH;
unsigned char dig_bcd[4]; // dig_bcd[0]:LSB(units digit of minute), .. , dig_bcd[3]:MSB(tens digit of hour)

void delay(unsigned char n) // delay function
{
    unsigned char idy1, idy2;

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

void bcd_4dig_to_4byte(unsigned char Hbyte_bcd, unsigned char Lbyte_bcd) // tranform 2 bytes of BCD codes into 4 bytes
{

    dig_bcd[0] = Lbyte_bcd & 0x0f; // low nibble of Lbyte_bcd
    dig_bcd[1] = Lbyte_bcd / 16;   // high nibble of Lbyte_bcd
    dig_bcd[2] = Hbyte_bcd & 0x0f; // low nibble of Hbyte_bcd
    dig_bcd[3] = Hbyte_bcd / 16;   // high nibble of Hbyte_bcd
}

void main(void) // 主函式
{
    //	unsigned int  j, i ;
    unsigned char dig_pos, bcd_U, bcd_H, bcd_L, bcdcode, ampm; // T12_24

    //	_acerl =0;  //POR:0b11111111,
    // The ACERH and ACERL control registers contain the ACER11~ACER0 bits which determine
    // which pins on PF1, PF0, PE1, PE0, and PORTA are used as analog inputs for the A/D
    // converter input and which pins are not to be used as the A/D converter input.
    // Setting the corresponding bit high will select the A/D input function, clearing
    // the bit to zero will select either the I/O or other pin-shared function.
    //
    //*** fsys must be at least 400 times larger than 32KHz, depending on delay().		*** //
    //  Sweep time for 7-seg 4 digit display plus time for checking and changing bcd bytes	//
    //    must be less than (1/32K) second													//
    //**************************************************************************************//
    _smod = 0b00000001;    // [7:5]=100->fH/16,110-->fH/4,111-->fH/2; [0]=1/0-->fH/(? or fL)
                           //	_smod=0b00000000; 	// [7:5]=000,001->fL,[0]=1-->fH, [1]=1/0:IDLE/SLEEP
    _pdc = 0;              // 設定 PD 埠為輸出
    _pec = 0;              // 設定 PE 埠為輸出
    _pcc0 = 0;             // 設定 PC0 埠為輸出
    Led7_com = led7com[0]; // 7-seg display COM (PD) OFF
    Led7_seg = led7seg[0]; // 7-seg display (PE) OFF
    _pc0 = 0;              // flickering LED for counting second; flickering once per 2 seconds
    //
    //	_wdtc=0b01111010; //[3:0]=1010:default-->disable WDT timer
    //	_acerl = _acerl & 0b11110111;  //POR:0b11110111, disable PA3 as AN3
    //	_c0sel=0; 		//disable PA2/TCK0/C0+/AN2, PA3/INT0/C0-/AN3 as C0+, C0-
    //	_pac3 = 1 ; 	// 設定 PA3 埠為輸入
    //
    //	_c1sel=0;  	//disable PC2/TCK2/PCK/C1+, PC3/PINT/TP2_0/C1- as C1+, C1-
    //	_pcc3=0;	//PC3/PINT/TP2_0/C1-
    //	_pcc4=0;	//PC4/[INT0]/[!PINT]/TP2_1
    //*** _tm2c0 register setting ***
    // _tm2c0: [7]=T2PAU; [6:4]=TnCK2~TnCK0; [3]=T2ON; [2:0]=unused
    //         [6:4]=000:fsys/4; 001:fsys; 010:fH/16; 011:fH/64; 100:ftbc(fL);
    //				110: TCK2 rising edge clock; 111: TCK2 falling edge clock
    //*******************************
    //*** _tm2c1 register setting (Output pins T2CP0-->PC3,T2CP1-->PC4) ***
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
    // In the Timer/Counter Mode, the TM output pin control must be disabled.
    //  TM output pin control resgister: TMPC0:T0CP1,T0CP0; TMPC1:T3CP1,T3CP0,T2CP1,T2CP0
    //*******************************
    //	_tm2al=255;
    //	_tm2ah=32;
    //	_tm2rp=0;
    //
    _tm2c0 = 0b01000000; //[6:4]=100:ftbc, [2:0] not used for TM2
    _tm2c1 = 0b11000000; //[7:6]=11:Timer/Counter, [5:4]=unused, [3:0] not used
                         //	_tm2rp=0;	// overflow
                         // In the Timer/Counter Mode, the TM output pin control must be disabled.
    _t2cp0 = 0;          // t2cp0:pc3, t2cp1:pc4; default: _t2cp0=1
    _t2cp1 = 0;          // t2cp0:pc3, t2cp1:pc4; default: _t2cp1=0
    _t2on = 1;
    ampm = 0;     // ampm=0x00:AM; ampm=0x80:PM
    bcd_U = 0x11; // Hour
    bcd_H = 0x58; // Minute
    bcd_L = 0x00; // Second

    bcd_4dig_to_4byte(bcd_U, bcd_H);

    while (1)
    {
        //  tm2dh, tm2dl can not be written, but can be cleared by setting t2on from low to high
        t2_dH = _tm2dh; // read procedure : high byte first, low byte last
        t2_dL = _tm2dl;
        if ((t2_dH >= t2_int_H) && (t2_dL == t2_int_L))
        {
            _t2on = 0;
            GCC_NOP();
            _t2on = 1; // TM2 is reset to "0" and counts again
            // Change the T12_24 value to switch the clock time mode (12H or 24H)
            //    by bit-wised XOR operation. T12_24=0-->12H, T12_24=1-->24H
            // Change the lighting status of the PC7 LED to indicate the clock time mode of
            //    12H or 24H.  dark-->12H, lighting-->24H
            // If the clock time mode is switched to 12H mode and it is at P.M.,
            //    Change bcd_U to bcd_U plus 0x12.
            // If the clock time mode is switched to 24H mode and it is after noon,
            //    Change bcd_U to bcd_U minus 0x12.
            //			GCC_NOP();
            if (bcd_L == 0x59)
            {
                bcd_L = 0x00;
                if (bcd_H == 0x59)
                {
                    bcd_H = 0x00;
                    // Insert if-else herein to check the value of T12_24.
                    // If T12_24=0, deal with the hours portion of the 12-H clock time.
                    // Otherwise, deal with the hours portion of the 24-H clock time.
                    // In the 24-H clock time mode, remember to reset ampm (i.e., ampm=0)
                    if (bcd_U == 0x11)
                    {
                        bcd_U = 0x00;
                        ampm = ampm ^ 0b10000000; // switch AM/PM; ampm=0:AM; ampm=0x80:PM
                    }
                    else
                    {
                        bcdcode = bcd_U & 0x0f;
                        if (bcdcode == 0x09)
                            bcd_U = bcd_U + 0x07;
                        else
                            bcd_U++;
                    }
                }
                else
                {
                    bcdcode = bcd_H & 0x0f;
                    if (bcdcode == 0x09)
                        bcd_H = bcd_H + 0x07;
                    else
                        bcd_H++;
                }
            }
            else
            {
                bcdcode = bcd_L & 0x0f;
                if (bcdcode == 0x09)
                    bcd_L = bcd_L + 0x07;
                else
                    bcd_L++;
            }
            // Flicker PC0 once per 2 seconds
            if (bcd_L % 2 == 1)
                _pc0 = 1;
            else
                _pc0 = 0;
            //_pc0 = ~_pc0;		//bit-wised "not" operator
            bcd_4dig_to_4byte(bcd_U, bcd_H);
        }
        //		Sweeping 7-seg 4 digit display within (1/32000) second
        for (dig_pos = 0; dig_pos < 4; dig_pos++)
        {
            Led7_com = 0x00;
            judge = 0xff;
            if ((dig_pos == 1 & dig_bcd[dig_pos] == 0) | (dig_pos == 3 & dig_bcd[dig_pos] == 0))
            {
                judge = 0x00;
            }
            Led7_seg = (led7seg[dig_bcd[dig_pos]] | ampm); // 8x8 LED Y-axis (PE) ON
            Led7_com = led7com[dig_pos] & judge;           // 8x8 LED X-axis (PD) OFF
            delay(2);
        }
    }
}
