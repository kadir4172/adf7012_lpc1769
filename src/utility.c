/*
 * utility.c
 *
 *  Created on: Jun 28, 2014
 *      Author: kadir
 */

#include "LPC17xx.h"                        /* LPC17xx definitions */
#include "ssp.h"
#include "dac.h"
#include "adc.h"
#include "utility.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>


uint32_t Systick_Counter = 0; //Her 10 ms de bir tick sayar
uint32_t Delay_Counter   = 0; //Delay_ms fonksiyonunda kullanilacak counter, 10ms de bir tick sayar
uint8_t  timeout_flag = 0;
uint32_t timeout_check;

/******************************************************************************
** Function name:		Gpio_Config
**
** Descriptions:		Beacon MCU nun ilgili pinlerini konfigure eder
** Returned value:		returns TRUE if successful
**
******************************************************************************/
_Bool Gpio_Config(void){

  LPC_PINCON->PINSEL0 &= ~(0x3<<0 | 0x3<<2); //* P0.0 ve P0.1 GPIO set edildi
  LPC_GPIO0->FIODIR   |= (0x1<<0 | 0x1<<1) ; //* P0.0 ve P0.1 output set edildi
  return TRUE;

}


/******************************************************************************
** Function name:		Systick_Init
**
** Descriptions:		Systick interrupt ini 10ms de bir olacak sekilde konfigure eder ve baslatir.
** Returned value:		returns TRUE if successful
**
******************************************************************************/
_Bool Systick_Init(void){
  SysTick->CTRL =  7;      //interrupt enable, systick enable
  SysTick->LOAD = 0xAFC7F; //systick for 10 miliseconds
  return TRUE;
}

/******************************************************************************
** Function name:		Systick_Handler
**
** Descriptions:		Systick interrupt handle eden fonksiyondur.
** Returned value:		returns TRUE if successful
**
******************************************************************************/
void SysTick_Handler(void){
	Systick_Counter += 1;
	Delay_Counter   += 1;
}


/******************************************************************************
** Function name:		Delay_ms
** Parameters :         counter: Delay icin girilen counter degeri
** Descriptions:		(counter * 10) [ms] kadar kodu bekletir
** Returned value:		returns TRUE if successful
**
******************************************************************************/
_Bool Delay_ms(uint32_t counter){
  Delay_Counter = 0;
  while(counter > Delay_Counter);
  return TRUE;
}

/******************************************************************************
** Function name:		Set_Delay
** Parameters :         counter: Timeout icin girilen counter degeri
** Descriptions:		Check_Delay tarafindan kontrol edilmek uzere zaman asimi degerini dondurur
** Returned value:		zaman asimi degerini dondurur
**
******************************************************************************/
uint32_t Set_Delay (uint32_t counter)
{
  return(Systick_Counter + counter -1);
}

/******************************************************************************
** Function name:		Check_Delay
** Parameters :         counter: Timeout icin girilen counter degeri
** Descriptions:		Zaman asimi dolduysa True doner.
** Returned value:		Zaman asimi flag i doner
**
******************************************************************************/
uint8_t Check_Delay(uint32_t counter)
{
  return(((counter - Systick_Counter)& 0x80000000) >> 27);
}

/******************************************************************************
** Function name:		Read_Adf7012_Muxout
**
** Descriptions:		adf7012 entegresinin muxout pininde ADC ile data okur
** Returned value:		returns TRUE if successfull
**
******************************************************************************/
_Bool Read_Adf7012_Muxout(uint32_t* read_val){

  Delay_ms(1);

  *read_val = ADCRead(ADC_CHANNEL);

  //conversion not succesful
  if(timeout_flag != 0){
      timeout_flag = 0;
      return FALSE;
    }

  Delay_ms(1);
  return TRUE;
}


/******************************************************************************
** Function name:		Write_Adf7012_Reg
**
** Descriptions:		adf7012 entegresine istenen register degerini yazar
** Parameters:			reg_value , yazilacak byte dizisinin ilk karaketerine pointer
** Returned value:		returns TRUE if successfull
**
******************************************************************************/
_Bool Write_Adf7012_Reg(uint8_t* reg_value, uint8_t size_of_reg){

	//*reg_value = 0x55;
  Delay_ms(10);
  ADF7021_LOAD_REGISTER_ENABLE;
  Delay_ms(1);
force_register:
  SSPSend(PORTNUM, reg_value, size_of_reg);
  if(timeout_flag != 0){
      timeout_flag = 0;
      goto force_register;
    }
  Delay_ms(1);
  ADF7021_LOAD_REGISTER_DISABLE;
  Delay_ms(10);
  return TRUE;
}

/******************************************************************************
** Function name:		Send_Vcxo_Signal
**
** Descriptions:		Analog output pinine istenilen degeri yazar
** Parameters:			DAC cikisina yazilacak 32 bit veri
** Returned value:		returns TRUE if successfull
**
******************************************************************************/
_Bool Send_Vcxo_Signal(uint32_t value){

	LPC_DAC->CR = value;   //DAC cikisina value degerini yaz
    return TRUE;
}
/******************************************************************************
** Function name:		Init_Adf7012
**
** Descriptions:		adf7012 entegresini OOK modunda istenen konfigurasyonda baslatir
** Returned value:		returns TRUE if successfull
**
******************************************************************************/
_Bool Init_Adf7012(void){

uint8_t register0[4] = {0x04, 0x11, 0xE0, 0x00};
uint8_t register1[3] = {0x5B, 0x40, 0x01}      ;
uint8_t register2[4] = {0x00, 0x00, 0x81, 0xEE};
uint8_t register3[4] = {0x00, 0x45, 0x20, 0xFF};

Delay_ms(500);

/*send register0*/

ADF7021_LOAD_REGISTER_ENABLE;

Delay_ms(1);
force_register0:
  SSPSend(PORTNUM, register0, sizeof(register0));
  if(timeout_flag != 0){
    timeout_flag = 0;
    goto force_register0;
  }
  Delay_ms(1);
  ADF7021_LOAD_REGISTER_DISABLE;
  Delay_ms(10);

/*send register1*/
  ADF7021_LOAD_REGISTER_ENABLE;
  Delay_ms(1);
force_register1:
  SSPSend(PORTNUM, register1, sizeof(register1));
  if(timeout_flag != 0){
    timeout_flag = 0;
    goto force_register1;
  }
  Delay_ms(1);
  ADF7021_LOAD_REGISTER_DISABLE;
  Delay_ms(10);

/*send register2*/
  ADF7021_LOAD_REGISTER_ENABLE;
  Delay_ms(1);
force_register2:
  SSPSend(PORTNUM, register2, sizeof(register2));
  if(timeout_flag != 0){
     timeout_flag = 0;
     goto force_register2;
   }
  Delay_ms(1);
  ADF7021_LOAD_REGISTER_DISABLE;
  Delay_ms(10);


  /*send register3 */
  ADF7021_LOAD_REGISTER_ENABLE;
  Delay_ms(1);
force_register3:
  SSPSend(PORTNUM, register3, sizeof(register3));
  if(timeout_flag != 0){
     timeout_flag = 0;
     goto force_register3;
   }
  Delay_ms(1);
  ADF7021_LOAD_REGISTER_DISABLE;
  Delay_ms(10);

return TRUE;
}

/******************************************************************************
** Function name:		Reverse_Array
**
** Parameters:			sirasi degistirilecek array ve uzunlugu
** Returned value:		returns TRUE if successfull
**
******************************************************************************/
_Bool Reverse_Array(uint8_t* input,uint8_t length){
  uint8_t i = 0;
  uint8_t* buffer_array = malloc(sizeof(uint8_t) * length);
  memcpy(buffer_array, input, length);

  for(i = 0; i<length; i++){
	  *(input+i) = *(buffer_array+(length-1)-i);
  }

  free (buffer_array);

return TRUE;
}
