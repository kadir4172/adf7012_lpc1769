/****************************************************************************
This file is part of Beacon_software for Adf7021
****************************************************************************/
#include <cr_section_macros.h>
#include <NXP/crp.h>

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_CRP ;

#include "LPC17xx.h"                        /* LPC17xx definitions */
#include "ssp.h"
#include "timer.h"
#include "dac.h"
#include "common_variables.h"

#define ADF7021_LOAD_REGISTER_ENABLE   LPC_GPIO0->FIOCLR |= (0x1<<0)  //P0.0 LE pini belirlendi
#define ADF7021_LOAD_REGISTER_DISABLE  LPC_GPIO0->FIOSET |= (0x1<<0)

#define ADF7021_CHIP_POWER_UP    LPC_GPIO0->FIOSET |= (0x1<<1)  //P0.1 CE  pini olarak belirlendi
#define ADF7021_CHIP_POWER_DOWN  LPC_GPIO0->FIOCLR |= (0x1<<1)

#define PORTNUM  1  //SSP1 portu Spi icin kullanilacak

uint32_t Systick_Counter = 0; //Her 10 ms de bir tick sayar
uint32_t Delay_Counter   = 0; //Delay_ms fonksiyonunda kullanilacak counter, 10ms de bir tick sayar
uint8_t timeout_flag = 0;

/*Function Prototypes*/
_Bool Systick_Init(void);
_Bool Delay_ms(uint32_t);
_Bool Gpio_Config(void);
_Bool Write_Adf7012_Reg(uint8_t*);
_Bool Init_Adf7012(void);
uint32_t Set_Delay (uint32_t);
uint8_t Check_Delay(uint32_t);
/*Function Prototypes*/


uint32_t timeout_check;
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
  Delay_ms(10);

  Init_Adf7012();                 //Adf7012 registerlarini istedigimiz konfigurasyonda yazalim
  Init_Timer(10);                 //10us intervalinde timer0 baslat
  Enable_Timer();                 //Timer0 enable et


  while ( 1 );                    //main de yapilacak is kalmadi bundan sonra isr lerle devam edecegiz
  return 0;
}


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
** Function name:		write_adf7012_reg
**
** Descriptions:		adf7012 entegresine istenen register degerini yazar
** Parameters:			reg_value , yazilacak byte dizisinin ilk karaketerine pointer
** Returned value:		returns TRUE if successfull
**
******************************************************************************/
_Bool Write_Adf7012_Reg(uint8_t* reg_value){

  Delay_ms(10);
  ADF7021_LOAD_REGISTER_ENABLE;
force_register:
  SSPSend(PORTNUM, reg_value, sizeof(reg_value));
  if(timeout_flag != 0){
      timeout_flag = 0;
      goto force_register;
    }
  ADF7021_LOAD_REGISTER_DISABLE;
  Delay_ms(10);
  return TRUE;
}


/******************************************************************************
** Function name:		init_adf7012
**
** Descriptions:		adf7012 entegresini OOK modunda istenen konfigurasyonda baslatir
** Returned value:		returns TRUE if successfull
**
******************************************************************************/
_Bool Init_Adf7012(void){

uint8_t register0[4] = {0x04, 0x15, 0xE0, 0x00};
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
**                            End Of File
******************************************************************************/

