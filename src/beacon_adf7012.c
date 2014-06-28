/****************************************************************************
This file is part of Beacon_software for Adf7021
****************************************************************************/
#include <cr_section_macros.h>
#include <NXP/crp.h>
#include <stdint.h>
// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const uint32_t CRP_WORD = CRP_NO_CRP ;

#include "LPC17xx.h"                        /* LPC17xx definitions */
#include "ssp.h"
#include "timer.h"
#include "dac.h"
#include "adc.h"
#include "utility.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ax25.h"
#include "audio_tone.h"

extern  _Bool PTT_OFF;
extern void Ptt_Off(void);

/******************************************************************************
**   Main Function  main()
******************************************************************************/
int main (void)
{
  /* SystemClockUpdate() updates the SystemFrequency variable */
  SystemClockUpdate(); //SystemFrequency = 72000000 by default
  Systick_Init();      //10 ms de bir tick atacak sekilde Systick i baslat

  Gpio_Config();       //Beacon MCU da kullanilacak pinleri konfigure et

  ADF7021_CHIP_POWER_DOWN;        //CE pini asagi cek
  Delay_ms(10);
  ADF7021_LOAD_REGISTER_DISABLE;  //LE pinini yukari cek, load register disable olsun
  Delay_ms(10);
  ADF7021_CHIP_POWER_UP;          //CE pinini yukari cek, ADF7012 enable olsun
  Delay_ms(10);

  SSP1Init();			          // SSP1 portunda SPI konusalim*/
  DACInit();                      // 1.2kHz ve 2kHz sinus olusturmak icin DAC peripheral i baslatalim
  ADCInit(ADC_CLK);               // 1Mhz ADC peripheral enable
  Delay_ms(100);

  s_address beacon_address[1] = {{"TAMSAT", 5}};
  Ax25_Send_Header(beacon_address,1);
  Ax25_Send_String("Hello World!");
  Ax25_Send_Footer();

  Modem_Setup();
  Delay_ms(100);

  while ( 1 )                   //main de yapilacak is kalmadi bundan sonra isr lerde devam edecegiz
  {
	  if(PTT_OFF){
		  Ptt_Off();
		  PTT_OFF  = FALSE;
	  }

	  //modem_packet_size=256*8;
	  Modem_Flush_Frame();
	  Delay_ms(600);
  }
  return 0;
}

/******************************************************************************
**                            End Of File
******************************************************************************/

