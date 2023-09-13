// This is a 8x8 dot-matrix LED sweeping program : Display ��N��, ��K��, ��N��, ��U�� in turn; 
// function delay(n) : Adjust the switching rate of LED column by changing the index n value.
#include "HT66F50.h"
#define LedPortx _pd // pa ��
#define LedPorty _pe // pa ��
unsigned char LedTablex[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 } ;
unsigned char LedTabley[88] = { 0x00, 0x00, 0x00, 0x7f, 0x02, 0x08, 0x20, 0x7f, //n
                       0x00, 0x00, 0x00, 0x7f, 0x08, 0x14, 0x22, 0x41,			//k
                       0x00, 0x00, 0x00, 0x7f, 0x02, 0x08, 0x20, 0x7f,			//n
                       0x00, 0x00, 0x00, 0x3f, 0x40, 0x40, 0x40, 0x3f,			//u
                       0x00, 0x00, 0x00, 0x7f, 0x49, 0x49, 0x49, 0x49,			//e
                       0x00, 0x00, 0x00, 0x7f, 0x49, 0x49, 0x49, 0x49,			//e
                       0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00,			//1
                       0x00, 0x00, 0x00, 0x7f, 0x49, 0x49, 0x49, 0x79,			//6
                       0x00, 0x00, 0x00, 0x7f, 0x7f, 0x00, 0x00, 0x00,			//|
                       0x00, 0x00, 0x00, 0x79, 0x49, 0x49, 0x49, 0x4f,			//2
                       0x00, 0x00, 0x00, 0x7f, 0x41, 0x41, 0b01000001, 0x7f,			//0
                       } ;
// sweeping x axis; At every x-axix point, show y-axix LEDs for a short time
void delay (unsigned int n)
{
	unsigned int idy1, idy2;
	for(idy1 = 0; idy1 < n ; idy1++)   // 
		{
		   	GCC_NOP();	//no operation, delay a instruction period
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
void main(void) // �D�禡
{
	unsigned int k, j, i, char_no ;
	_smod=0b11000000; 	// [7:5]=100->fH/16,110-->fH/4
	_pdc = 0 ; 	// �]�w PD �𬰿�X
	LedPortx = 0x00 ; 	// 8x8 LED X-axis (PD) OFF
	_pec=0; 			// �]�w PE �𬰿�X
    LedPorty = 0x00;	// 8x8 LED Y-axis (PE) ON	
	
	while(1)
	{
		for(char_no = 0 ; char_no < 11 ; char_no++)
		{
			for( j = 0 ; j < 100 ; j++ )
			{
				for( k = 0 ; k < 8 ; k++ )
				{
					i=char_no*8+k;
					LedPortx = 0x00 ; // ��X LED ����X, �I�G LEDx
					LedPorty = 0x00 ; // ��X LED ����X, �I�G LEDx	
					LedPortx = LedTablex[k] ; // ��X LED ����X, �I�G LEDx
					LedPorty = LedTabley[i] ; // ��X LED ����X, �I�G LEDx	
					delay(10);	   
				}
			}
		}
	}
}

