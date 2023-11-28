// This is a control program of seven-segment LEDs display and keyboard scanning. 
// This program can check whether keyin password is ok.
// PA0 ~ PA7 for keyboards
// If the keyin password is right, _pc3 will output a pulse to give a driving signal
//     for door opening or other actions
// If the keyin password is wrong, TM0 is on and a PWM signal from _pc5 is used to generate an alarm for a short time.
// "D":change password, "C":backspace, "F":ENTER
// operation procedure:
//   (1) keyin password and check : keyin four numbers (0~9) continuously and then keyin "F"
//       if there occurs a non-number key during the input of four numbers, an alarm will be given
//   (2) change password : keyin the correct password and then keyin "C". 
//       If the password is correct, a new password of four numbers can be keyed and then keyin "F"  
//
// Interconnect lines on the Practice Board : 
//   (1) LEDs --> MCU : PC3 --> PC3
//   (2) 7-seg LEDs --> MCU :  PD0~PD7 --> PD0~PD7,  PE0~PE7 --> PE0~PE7
//	 (3) Buzzer --> MCU : PC4 --> PC5
//	 (4) 4x4 KEY --> MCU : PA0~PA7 --> PA0~PA7
//	 (5) no jump line

#include "HT66F50.h" 

#define Led7_com _pe // pe 埠
#define Led7_seg _pd // pd 埠
//  In Holtek C, char, int(16), short(16) --> 8 bits, long(32) --> 16 bits
const unsigned char led7com[4] = { 0x08, 0x04, 0x02, 0x01 } ;
const unsigned char led7seg[19]= { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 
                     		0x6d, 0x7d, 0x07, 0x7f, 0x67,
                     		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x88, 0x76 } ;
//0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,-,_.,H
//unsigned char pwd[4];       
              		
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
//
unsigned short scankey()
{
//  In Holtek C, char, int(16), short(16) --> 8 bits, long(32) --> 16 bits
	unsigned short i_key, key;
	_pac = 0b11110000 ; 	// 1 : 設定 PA[7:4] 埠為輸入; 0 : 設定 PA[3:0] 埠為輸出; 
	_papu= 0b11110000;  // 1: pull-up
	_pa=0b11111110;		//reset the bit related to the 0th column	
	_acerl =0;		// 設定 PA 埠為一般I/O，非AN類比輸入
	_c0sel=0; // pa2 and pa3 are disconnected c0+ and c0-, respectively, and act as I/O pins
	_c0os=1;  // pa0 is disconnected to c0x and purly acts as an I/Opin
	_c1os=1;  // pa5 is disconnected to c1x and purly acts as an I/Opin 
	_t0cp0=0; // pa0 is disconnected to TM0 and purly acts as an I/O pin
	_t1acp0=0; //pa1 is disconnected to TM1 
// if pa0(pa5) acts as c0x(c1x), pa0(pa5) work as an output,  In this example, _c0os=1 can be eliminated
// if pa2 and pa3 acts as c0+ and c0-, respectively, they work as inputs,
	key=0;
	for(i_key=0; i_key<4; i_key++)
	{
		if(!(_pa & (1<<4)))
			break;
		key++;
		if(!(_pa & (1<<5)))
			break;
		key++;
		if(!(_pa & (1<<6)))
			break;
		key++;					
		if(!(_pa & (1<<7)))
			break;
		key++;
		_pa <<=1;	//shift '0' to the next column, [i.e. pa(i)-->pa(i+1), i=0~3]
		_pa |= 0b00000001;	//disable the column checked just now
	}
	return key;
}

unsigned short EEPROM_RD(unsigned char ee_addr)	
{
	_eea=ee_addr;//user defined address 
	_mp1=0x40;	//setup memory pointer MP1, MP1 points to EEC register 
	_bp=0x01;	//setup Bank Pointer
	_iar1=0x02; //_rden=1; 	//set RDEN bit, enable read operations
	_iar1=0x03; //_rd=1; 	//start Read Cycle - set RD bit	
	while((_iar1&0x01)) 	//while(_rd)	//check for read cycle end
	{
	}
	_iar1=0x00; //disable EEPROM read/write, i.e. _rden=0; _wren=0
	_bp=0;
	return _eed;	//return read data
}
void EEPROM_WR(unsigned char ee_addr, unsigned char ee_data)	
{
	_eea=ee_addr;	//user defined address
	_eed=ee_data;	//user defined data
	_mp1=0x40;		//setup memory pointer MP1, MP1 points to EEC register
	_bp=0x01;		//setup Bank Pointer
	_iar1=0x08;		//_wren=1;		//set WREN bit, enable write operations
	_iar1=0x0C;		//_wr=1;		//Start Write Cycle - set WR bit 
	while((_iar1&0x04))		//while(_wr)	//check for read cycle end
	{
	}
	_iar1=0x00; //disable EEPROM read/write, i.e. _rden=0; _wren=0	
	_bp=0;	
}
		
void main(void) // 主函式
 {
//  In Holtek C, char, int(16), short(16) --> 8 bits, long(32) --> 16 bits
	unsigned char key_bcd[4], key_disp[4], keypressed; // code for seven-segment LEDs, keypressed index
	unsigned short 	key; // variables for getting a returned value from keyscan function
	unsigned char dig_pos;	//position of 4-digit seven-segment LEDs
	unsigned char key_num;  //keypressed time: 1~4
	unsigned char i_chk, pwd_ok, disp_symbol, i_chg, key_mode; //, i_pwdok;
	unsigned char pwd[4];
//	_acerl =0;  //POR:0b11111111, 
//The ACERH and ACERL control registers contain the ACER11~ACER0 bits 
//which determine which pins on PF1, PF0, PE1, PE0, and PORTA are used 
//as analog inputs for the A/D converter input and which pins are not
//to be used as the A/D converter input. Setting the corresponding bit
//bit high will select the A/D input function, clearing
//the bit to zero will select either the I/O or other
//pin-shared function.
	_smod=0b11000000;	// [7:5]=100->fH/16,110-->fH/4; [0]=1-->fH/? or fL
//	_smod=0b00000000; 	// [7:5]=000,001->fL,[0]=1-->fH, [1]=0:SLEEP	
	_pdc = 0 ; 	// 設定 PD 埠為輸出; seven-segment LEDs
	_pec = 0 ; 	// 設定 PE 埠為輸出; PE[3:0]=com port
	Led7_com = led7com[0] ; 	// initial common port
    Led7_seg = led7seg[0];	// initial LED port	
//
//
// set PC port to be used as digital I/O
//	_pcc=0;	//PC7~PC0為輸出
//	_t1bcp0=0; // pc0/TP1B_0 is disconnected to TM1
//	_t1bcp1=0; // pc1/TP1B_1 is disconnected to TM1
	_c1sel=0;  //set PC2/C1+ and PC3/C1- and are disconnected c1+ and c1-
	_t2cp0=0;  // pc3/TP2_0 is disconnected to TM2 
//	_t2cp1=0;  // pc4/TP2_1 is disconnected to TM2	
	_t1bcp2=0; // pc5/TP0_1/TP1B_2 is disconnected to TM1
//	_t0cp1=0;  // pc5/TP0_1/TP1B_2 is disconnected to TM0
//
//  set PA port to be used as digital I/O
	_c0os=1; // pa0 is disconnected to c0x and purly acts as an I/O pin
	_t0cp0=0; // pa0 is disconnected to TM0 and purly acts as an I/O pin
	_t1acp0=0; //pa1 is disconnected to TM1 
	_c0sel=0; // pa2 and pa3 are disconnected c0+ and c0-, respectively, and act as I/O pins	
	_c1os=1;  // pa5 is disconnected to c1x and purly acts as an I/Opin 
	_acerl=0b00000000;	// PA0, PA1: AN輸入; 0:設定 PA 埠為一般I/O，非AN類比輸入
//
//	initial 4-digit seven-segment LEDs : 0000
	key_disp[3]=16;
	key_disp[2]=16;				
	key_disp[1]=16;					
	key_disp[0]=16;   
//	
	keypressed=0;
//
	_pc3=0;
	_pcc3=0;
	key_num=0;	
	pwd_ok=0;
	key_mode=0; //0:steadby, 1:check, 2:change, 3:change_do, >=4:error 	
	disp_symbol=16;
//
// setting alarm for wrong password
	_tm0c0=0b01000000; //[6:4]=100:ftbc, [2:0]=000:overflow 
	_tm0c1=0b10101000; 	
//[7:6]=00:comp match mode, [5:4]=11:toggle, [3]=0:init low, 
//          [2]=1:invert output, [0]=1:comp_A clr	
//[7:6]=10:PWM, [5:4]=10:PWM output, [3]=0/1:active low/high, 
//          [2]=1:invert output, [1]=0:CCRP/per(CCPA/duty), [0] not used	
//[7:6]=11:Timer/Counter, [5:4]=unused, [3:0] not used
	_pcc5= 0;		// 設定 PC0 埠為輸出
	_pc5 = 0;	
	_tm0al=255;
	_tm0ah=1;	
	_t0cp0=0;  //t0cp0:pa0, t0cp1:pc5, pc5(PCK)-->PCK pin default disable
	_t0cp1=1;  //t0cp0:pa0, t0cp1:pc5, pc5(PCK)-->PCK pin default disable
	
//***********************************************************************
// trun on one at the same time

/*  initial password
    pwd[3] = 0;
    pwd[2] = 0;
    pwd[1] = 0;
    pwd[0] = 0;  
*/

//  Access Password From EEPROM
	for(i_chg=0; i_chg<4; i_chg++)
	{
		pwd[i_chg]=EEPROM_RD(i_chg);
	}
//***********************************************************************

//
//
	for(i_chg=0; i_chg<200; i_chg++)
	{
		for( dig_pos = 0 ; dig_pos < 4 ; dig_pos++ )
		{	
			Led7_com = 0x00 ; 	// common-cathode pins off; 0-->off, 1-->on
   			Led7_seg = led7seg[pwd[dig_pos]];	// change the LED pattern
			Led7_com = led7com[dig_pos] ; 	// change the seleted DI0git position
   			delay(20);  //longer delay --> higher LED brightness	   
		}	
	}	
//
//					
	while(1)
	{
//  key=16-->not keypressed, keypressed=0,   
//  key<16-->keypressed occurs, keypressed=1
//  key<16 && keypreeed==0-->key is pressed just now, display changes
//  每按一個健後，一定要放開鍵後再按，才能算另一個輸入。所以，判斷按鍵一定要在keypressed=0時。
		key=scankey();    			
		if ((key < 16) && (keypressed==0))
		{	
			key_num++;
//	判斷輸入的字元: 12-->backspace, 0~9儲存輸入碼及更改顯示(往高字元推一位), 其他則重頭輸入	
			if ((key==12))
			{
				if(key_num>=2)    //此時key_num的值等於所輸入數字數目加1("C")
				{
					key_num=key_num-2;
					key_bcd[0]=key_bcd[1];
					key_bcd[1]=key_bcd[2];				
					key_bcd[2]=key_bcd[3];					
					key_bcd[3]=0;
					key_disp[0]=key_disp[1];
					key_disp[1]=key_disp[2];				
					key_disp[2]=key_disp[3];					
					key_disp[3]=disp_symbol;							
				}
			}
			else if (key<=9)
			{
				key_bcd[3]=key_bcd[2];
				key_bcd[2]=key_bcd[1];				
				key_bcd[1]=key_bcd[0];					
				key_bcd[0]=key;
				key_disp[3]=key_disp[2];
				key_disp[2]=key_disp[1];				
				key_disp[1]=key_disp[0];					
				key_disp[0]=18;
			}
			else
			{
				if (key_num <= 4)
				{
					key_num=0;
					key_bcd[3]=0;
					key_bcd[2]=0;				
					key_bcd[1]=0;					
					key_bcd[0]=0;
					key_disp[3]=disp_symbol;
					key_disp[2]=disp_symbol;				
					key_disp[1]=disp_symbol;					
					key_disp[0]=disp_symbol;
					_t0on=1;
					delay(2000);
					_t0on=0;
				}
				else						
				{
					if (key==13)
						key_mode|=0b00000010;
					else if (key==15)
						key_mode|=0b00000001;
					else 
					{
						key_mode|=0b00000100;
						key_bcd[3]=16;
						key_bcd[2]=16;				
						key_bcd[1]=16;					
						key_bcd[0]=16;
					}	
				}
			}			
			keypressed=1;	
		}
		//No button is pressed
		if (key == 16)
			keypressed=0;
		//Five buttons have been pressed sequentially and no button is pressed for now
		if ((key_num == 5) && (keypressed==0))
		{
			key_num=0;
			pwd_ok=1;
			for(i_chk=0; i_chk<4; i_chk++)
			{
				if(key_bcd[i_chk] != pwd[i_chk])
					pwd_ok=0;				
			}
			switch(key_mode)
			{
				case 3:	//new-password-setting mode
					for(i_chg=0; i_chg<4; i_chg++)
					{
						pwd[i_chg]=key_bcd[i_chg];
						EEPROM_WR(i_chg, key_bcd[i_chg]);
					}	
					key_mode=0;
					disp_symbol=16;
//				pwd_chg=0;
//				pwd_chk=0;
					break;
				case 2:	//change-password mode 
					if(pwd_ok==0)
						goto key_alarm;
					else
					{
						disp_symbol=17;	//change the display symbol for new password					
						break;
					}
				case 1:	//check-password mode
					if (pwd_ok==1)
					{
						_pc3=1;
						delay(4000);				
						disp_symbol=16;	
						key_mode=0;
						break;
					}
					else
						goto key_alarm;
				default:
	key_alarm:		_t0on=1;
					key_mode=0;	
					disp_symbol=16;			
					break;
			}
			delay(2000);
			_pc3=0;	
			_t0on=0;
			key_disp[3]=disp_symbol;
			key_disp[2]=disp_symbol;				
			key_disp[1]=disp_symbol;					
			key_disp[0]=disp_symbol;
		}			
//
// scanning display of 4-digit seven-segment LEDs						
		for( dig_pos = 0 ; dig_pos < 4 ; dig_pos++ )
		{	
			Led7_com = 0x00 ; 	// common-cathode pins off; 0-->off, 1-->on
   			Led7_seg = led7seg[key_disp[dig_pos]];	// change the LED pattern
			Led7_com = led7com[dig_pos] ; 	// change the seleted digit position
   			delay(2);  //longer delay --> higher LED brightness	   
		}					
	}	
}
