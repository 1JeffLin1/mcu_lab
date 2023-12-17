// This is a control program of SPI with seven-segment LEDs dissplay. 
// The SPI modual MPU is chosen as a slave device which transmits the ADC_value/32 and receives
//    the ADC_value/32 from a master device. 
// The pins _pb5, _pa5, _pa6, and _pa7 are used to act as a master SPI pins. 
//    master SDI0 <--> slave SDO0 ;  master SDO0 <--> slave SDI0
// The seven-segment LEDs dispaly received data(3:2) and transmitted data(1:0), 
/// Interconnect lines on the Practice Board : 
//   (1) VR --> MCU :  PA0 --> PA0
//   (2) 7-SEG LEDs --> MCU : PD --> PD, PE --> PE
//   (3) SPI slave --> SPI master : pb5 --> pb5, pa7 --> pa7, pa6 --> pa5, pa5 --> pa6

#include "HT66F50.h" 

#define Led7_com _pe // pe 埠
#define Led7_seg _pd // pd 埠
#define SCS0 _pb5	//slave: input; master: output
#define SCK0 _pa7	//slave: input; master: output
#define SDI0 _pa6	//slave: input; master: input
#define SDO0 _pa5	//slave: output; 
//#define SCS_M _pb6	//master: output
//#define SCK_M _pa4	//master: output
//#define SDI_M _pa3	//master: input
//#define SDO_M _pa2	//master: output
#define divf_no 32768  // clock freq =32.768 kHz
const unsigned char t2_int_H = divf_no/1/256;  //16 
const unsigned char t2_int_L = divf_no/1%256;  //255 
//  In Holtek C, char, int(16), short(16) --> 8 bits, long(32) --> 16 bits
const unsigned char led7com[4] = { 0x08, 0x04, 0x02, 0x01 } ;
const unsigned char led7seg[19]= { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 
                     		0x6d, 0x7d, 0x07, 0x7f, 0x67,
                  		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x88, 0x76 } ;
//                   		                     		
//unsigned char pwd[4] = {0, 1, 0, 1 } ;   
unsigned char dig_bcd[4]; //, sim_rx_num;   
unsigned char spi_data;
unsigned char spi_read; 
unsigned char adc_val0; //, adc_val1;
unsigned char adc0h, adc0l, channel; //, adc1h, adc1l; // , adc2h, adc2l;
//  
/*
void __attribute((interrupt(0x14))) isr_tm2_mf0(void)
{
	SCS0 = 0;
	GCC_NOP();
	GCC_NOP();
	_simd=spi_data;								
	_t2af=0;
	_mf0f=0;
}
*/
//
void __attribute((interrupt(0x20))) isr_sim(void)
{
	//unsigned char digsim[2]; 
	//unsigned char digsimx;  //,  digsimy;
	//unsigned char ii;
		
	spi_read=_simd;	
	channel=spi_read;

//	SCS0 = 1;
	
	//adc_val0=(adc0h<<3) | (adc0l>>5);		//ADC_value/32 : 0~128
	if(channel == 1)
		spi_data=adc_val0;	
	else
		spi_data=99;
					
	//if (spi_data > 99)
	//	spi_data=99;
		
	_simd=spi_data;
/*
    digsimx = spi_data % 100;
    digsim[1] = digsimx / 10;  
    digsim[0] = digsimx % 10;  
    if(channel == 1)
    {
    	for(ii=0; ii<2; ii++) 
   	 		dig_bcd[ii] = digsim[ii];
    }
*/       			   	
	_trf=0;
	_simf=0;	
	_mf2f=0;   	 		
    	 	
} 
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
void bin2bcd (unsigned char no)   // delay function
{
//  In Holtek C, char, int(16), short(16) --> 8 bits, long(32) --> 16 bits
	unsigned char dig[2]; 
	unsigned char digy;  //digx,  digy;
	unsigned char ii ;
    
//    dig[3] = no / 1000;
//    digx = no % 1000;
//    dig[2] = digx / 100;
    digy = no % 100;
    dig[1] = digy / 10;  
    dig[0] = digy % 10;
    for(ii=0; ii<2; ii++) 
   	 	dig_bcd[ii] = dig[ii];
}
//
//
//		
void main(void) // 主函式
{
//  In Holtek C, char, int(16), short(16) --> 8 bits, long(32) --> 16 bits
//	unsigned char key_bcd[4], key_DI0sp[4], key_DI0spee[4], keypressed; // code for seven-segment LEDs, keypressed index
//	unsigned char key; // variables for getting a returned value from keyscan function
	unsigned char dig_pos; // DI0g_pos;	//position of 4-DI0git seven-segment LEDs
//	unsigned char key_num;  //keypressed time: 1~4
//	unsigned char i_chk, pwd_ok, DI0sp_symbol, i_chg, key_mode;
//	unsigned char spi_m_sdi;
	unsigned char disp_num;	
//	unsigned char spi_read_d[2];
//	unsigned long spi_read_data_dec;	
//	unsigned long adc_val0; //, adc_val1;
//	unsigned char adc0h, adc0l; //, adc1h, adc1l; // , adc2h, adc2l;	
//	_acerl =0;  //POR:0b11111111, 
//The ACERH and ACERL control registers contain the ACER11~ACER0 bits 
//which determine which pins on PF1, PF0, PE1, PE0, and PORTA are used 
//as analog inputs for the A/D converter input and which pins are not
//to be used as the A/D converter input. Setting the corresponDI0ng bit
//bit high will select the A/D input function, clearing
//the bit to zero will select either the I/O or other
//pin-shared function.
	_smod=0b11000000;	// [7:5]=100->fH/16,110-->fH/4; [0]=1-->fH/? or fL
//	_smod=0b00000000; 	// [7:5]=000,001->fL,[0]=1-->fH, [1]=0:SLEEP	
	_pdc = 0 ; 	// 設定 PD 埠為輸出; seven-segment LEDs
	_pec = 0b00000000 ; 	// 設定 PE7為輸入; PE[3:0]=com port輸出
//	_pepu = 0b10000000 ; 	// 設定 PE7為輸入; PE[3:0]=com port輸出
	Led7_com = led7com[0] ; 	// initial common port
    Led7_seg = led7seg[0];	// initial LED port	
//
//  ADC
	_acerl=0b00000001;	// PA0, PA1: AN輸入; 0:設定 PA 埠為一般I/O，非AN類比輸入
	_pac |= 0b00000001;  // set PA0~PA2 input
	_adcr0=0b00010000; // [5]=1:ADOFF ADC OFF;[4]=1:ADRH[3:0]-ADRL[7:0]; [2:0]=000:AN0
	_adcr1=0b00000100; // [2:0]=000: fSYS; 010: fSYS/4; 100: fSYS/16; 101: fSYS/32; 110: fSYS/64
	_adoff=0; //power on ADC
//
//	_c1sel=0; // pc2 and pc3 are DI0sconnected c1+ and c1-, respectively, and act as I/O pins
//	_pc3=0;
//	_pcc3=0;
//
/* setting alarm for wrong password
	_tm0c0=0b01000000; //[6:4]=100:ftbc, [2:0]=000:overflow 
	_tm0c1=0b10101000; 	
	_pcc5= 0;		// 設定 PC5 埠為輸出
	_pc5 = 0;	
	_t1bcp2=0;  //DI0sable PC5 to act as TP1B_2. default setting : DI0sable 	
	_tm0al=255;
	_tm0ah=1;	
	_t0cp0=0;  //t0cp0:pa0, t0cp1:pc5, pc5(PCK)-->PCK pin default DI0sable
	_t0cp1=1;  //t0cp0:pa0, t0cp1:pc5, pc5(PCK)-->PCK pin default DI0sable
*/
//	SPI: _pa5=SDO, _pa6=SDI, _pa7=SCK, _pb5=scs
	_pac |= 0b11000000 ; 	// 0 : 為輸出; 1 : 為輸入
//	_papu |= 0b01000000; // pull up PA6, PA7
//	_pbc6=0;
	_pbc5=1; 	// 0 : 為輸出; pb5:slave scs; pb6:master scs
//	_c0sel=0; 	// pa2 and pa3 are DI0sconnected c0+ and c0-, respectively, and act as I/O pins
//	_pbpu |= 0b00100000;
//
//
	_tm2c0=0b01000000; //[6:4]=100:ftbc, [2:0]=000:overflow 
	_tm2c1=0b00000001; //[7:6]=00: Compare Match Output Mode; [0]=1/0-->Comp_A/_P clear
	_tm2ah=t2_int_H; 
	_tm2al=t2_int_L;
//	_tm2rp=0;	// overflow
//In the Timer/Counter Mode, the TM output pin control must be disabled. 
//	_t2cp0=0;  //t2cp0:pc3, t2cp1:pc4
//	_t2cp1=0;  //t2cp0:pc3, t2cp1:pc4	
//	_t2on=1;
	dig_bcd[0]=0;		
	dig_bcd[1]=0;
	dig_bcd[2]=0;
	dig_bcd[3]=0;	
	_t2ae=1;
	_t2af=0;
	_mf0e=1;
	_mf0f=0;
//	_emi=1;
//
// simc2:[5]=0/1:sck=high/low when the clk is inactive; [5:4]=00 or 11:data capture at SCK rising edge
// simc2:[3]=0/1:LSB/MSB is SPI Data shift order; [2](CSEN)=0/1:disable/enable SPI SCS (SPI Slave Select) pin Control
// simc2:[1](WCOL)=0/1: SPI Write Collision flag; [0](TRF)=0/1:data transfer is going on/completed(Transmit/Receive Complete flag)  
	_simc2=0b00101000;	//[5:4]=11:SK low base, rising capture; [3]=1:MSB first
//
// simc0[7:5]: 001: SPI master mode; SPI clock is fSYS/16 ; 010: SPI master mode; SPI clock is fSYS/64 
//             011: SPI master mode; SPI clock is fTBC 
	_simc0=0b10100000;	//[7:5]=101:SPI slave; [1]=1:SIMEN
//	SCS0 = 1;	
//	_pa5=SDO, _pa6=SDA, _pa7=SCL
//
//	_t2ae=1;
//	_t2af=0;
//	_ade=1;
//	_adf=0;	
	_sime=1;
	_simf=0;	
	_mf2e=1;
	_mf2f=0;
//	_emi=1;
//
//	i_chg = 0 ;
//	SCS_M=1;  //SCS_M=0-->select spi transport
//	SCS0 = 1;
	_adcr0=0b00010000; // [5]=1:ADOFF ADC OFF;[4]=1:ADRH[3:0]-ADRL[7:0]; [2:0]=000:AN0
	_start=0;
	_adoff=0; //power on ADC
//	_ace0=1;	
	delay(1);
//	_start=0;
	_start=1;
	_start=0;
	while (_eocb)
	{
	};
	adc0h=_adrh;
	adc0l=_adrl;
//	
	adc_val0=(adc0h<<3) | (adc0l>>5);		//ADC_value/32 : 0~128
	if (adc_val0 > 99)
		adc_val0=99;
//	bin2bcd(adc_val0);	
//
//
//	disp_num=0;
	_csen=1;
	_simen=1;
	delay(1);
	_emi=1;	
//	_t2on=1;
//	
//	SCS0 = 0;
//	_simd=0x0f;
//
	channel=0;
	_simd=0xff;
//			
	while(1)
	{
//
//		insert ADC of VR1 and put its two bcd data to dig_bcd[3] and dig_bcd[2]
//		 
//		adc_val1=(adc1h<<3) | (adc1l>>5);		//ADC_value/32 : 0~128	
//
		_adcr0=0b00010000; // [5]=1:ADOFF ADC OFF;[4]=1:ADRH[3:0]-ADRL[7:0]; [2:0]=000:AN0
		_start=1;
		_start=0;
		while (_eocb)
		{
		};
		adc0h=_adrh;
		adc0l=_adrl;
		adc_val0=(adc0h<<3) | (adc0l>>5);		//ADC_value/32 : 0~128	
		//if(adc_val0>=99)
		//	adc_val0=99;
		bin2bcd(adc_val0);							
//
// scanning DI0splay of 4-DI0git seven-segment LEDs	
		for( disp_num = 0 ; disp_num < 20 ; disp_num++ ) 	
		{
			for( dig_pos = 0 ; dig_pos < 4 ; dig_pos++ )
			{	
				Led7_com = 0x00 ; 	// common-cathode pins off; 0-->off, 1-->on
   				Led7_seg = led7seg[dig_bcd[dig_pos]];	// change the LED pattern
				Led7_com = led7com[dig_pos] ; 	// change the seleted digit position
   				delay(2);  //longer delay --> higher LED brightness	   
			}
		}
	}		
}
