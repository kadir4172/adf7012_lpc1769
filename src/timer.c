/****************************************************************************
 *   $Id:: timer.c 5823 2010-12-07 19:01:00Z usb00423                       $
 *   Project: NXP LPC17xx Timer for PWM example
 *
 *   Description:
 *     This file contains timer code example which include timer 
 *     initialization, timer interrupt handler, and APIs for timer access.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
****************************************************************************/
#include "LPC17xx.h"
#include "type.h"
#include "timer.h"
#include "dac.h"
#include "common_variables.h"


  extern void Sinus_Generator(void);
  uint32_t Audio_Signal2 = 0;
  uint32_t Sinus_Generator2(uint8_t);

volatile uint32_t timer0_m0_counter = 0;
//volatile uint32_t timer1_m0_counter = 0;
//volatile uint32_t timer2_m0_counter = 0;
//volatile uint32_t timer3_m0_counter = 0;
//volatile uint32_t timer0_m1_counter = 0;
//volatile uint32_t timer1_m1_counter = 0;
//volatile uint32_t timer2_m1_counter = 0;
//volatile uint32_t timer3_m1_counter = 0;

//volatile uint32_t timer0_capture0 = 0;
//volatile uint32_t timer1_capture0 = 0;
//volatile uint32_t timer2_capture0 = 0;
//volatile uint32_t timer3_capture0 = 0;
//volatile uint32_t timer0_capture1 = 0;
//volatile uint32_t timer1_capture1 = 0;
//volatile uint32_t timer2_capture1 = 0;
//volatile uint32_t timer3_capture1 = 0;

/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
void TIMER0_IRQHandler (void) 
{  
  if ( LPC_TIM0->IR & (0x1<<0) )
  {
    LPC_TIM0->IR = 0x1<<0;		/* clear interrupt flag */
    timer0_m0_counter++;
    Sinus_Generator();
    //Audio_Signal2 = Sinus_Generator2(1);
    //LPC_DAC->CR = ((Audio_Signal2) << 6) | DAC_BIAS;
  }
  if ( LPC_TIM0->IR & (0x1<<1) )
  {
    LPC_TIM0->IR = 0x1<<1;		/* clear interrupt flag */
    //timer0_m1_counter++;
  }
  if ( LPC_TIM0->IR & (0x1<<4) )
  {
	LPC_TIM0->IR = 0x1<<4;		/* clear interrupt flag */
	//timer0_capture0++;
  }
  if ( LPC_TIM0->IR & (0x1<<5) )
  {
	LPC_TIM0->IR = 0x1<<5;		/* clear interrupt flag */
	//timer0_capture1++;
  }
  return;
}

/******************************************************************************
** Function name:		Enable_Timer
**
** Descriptions:		Enable timer
**
** Returned value:		returns TRUE if successfull
** 
******************************************************************************/
_Bool Enable_Timer(void)
{
  LPC_TIM0->TCR = 1;
  return TRUE;
}

/******************************************************************************
** Function name:		Disable_Timer
**
** Descriptions:		Disable timer
**
** Returned value:		returns TRUE if successfull
** 
******************************************************************************/
_Bool Disable_Timer(void)
{
  LPC_TIM0->TCR = 0;
  return TRUE;
}

/******************************************************************************
** Function name:		Reset_Timer
**
** Descriptions:		Reset timer
**
** Returned value:		returns TRUE if successfull
** 
******************************************************************************/
_Bool Reset_Timer(void)
{
  uint32_t regVal;
  regVal = LPC_TIM0->TCR;
  regVal |= 0x02;
  LPC_TIM0->TCR = regVal;
  return TRUE;
}

/******************************************************************************
** Function name:		Init_Timer
**
** Descriptions:		Initialize timer, set timer interval, reset timer,
**						install timer interrupt handler
**
** parameters:			timer interval in microseconds
** Returned value:		returns TRUE if successfull
** 
******************************************************************************/
_Bool Init_Timer (uint32_t TimerInterval )
{
  //uint32_t pclkdiv, pclk;


	timer0_m0_counter = 0;
	//timer0_m1_counter = 0;
	//timer0_capture0 = 0;
	//timer0_capture1 = 0;
	LPC_SC->PCONP |= (0x01<<1);

	LPC_PINCON->PINSEL3 &= ~((0x3<<24));
	LPC_PINCON->PINSEL3 |= ((0x3<<24));

	LPC_TIM0->IR = 0x0F;         /* Clear MATx interrupt include DMA request */

	/* By default, the PCLKSELx value is zero, thus, the PCLK for
	all the peripherals is 1/4 of the SystemFrequency. */
	/* Bit 2~3 is for TIMER0 */
	LPC_SC -> PCLKSEL0 |= (0x01<<2);  //timer clk = sysclk =72Mhz
	LPC_TIM0->PR  = 72; /* set prescaler to get 1 M counts/sec */

	LPC_TIM0->MR0 = TimerInterval - 1; /*set the interval in useconds */

	LPC_TIM0->EMR &= ~(0xFF<<4);
	LPC_TIM0->EMR |= ((0x3<<4));

	LPC_TIM0->MCR = (0x3<<0);	/* Interrupt and Reset on MR0*/
	NVIC_EnableIRQ(TIMER0_IRQn);
	return (TRUE);

}

uint32_t Sinus_Generator2(uint8_t freq_index){
  static uint32_t sinus2000[50] = {512,576,639,700,758,812,862,906,943,974,998,1014,1022,1022,1014,998,974,943,906,862,812,758,700,639,576,512,447,384,323,265,211,161,117,80,49,25,9,1,1,9,25,49,80,117,161,211,265,323,384,447};
  static uint32_t sinus1200[83] = {512,550,589,627,664,701,736,770,803,834,863,890,915,937,958,975,990,1003,1012,1019,1022,1023,1021,1016,1008,997,983,967,948,926,903,877,848,818,787,753,718,682,645,608,569,531,492,454,415,378,341,305,270,236,205,175,146,120,97,75,56,40,26,15,7,2,0,1,4,11,20,33,48,65,86,108,133,160,189,220,253,287,322,359,396,434,473};

  static uint8_t i = 0;

  uint32_t* ptr;


  if(freq_index == 1){
	  ptr = sinus1200;

	  i = (i==82) ? 0 : i+1 ;
	  return *(ptr + i);
  }
  else{
	  ptr = sinus2000;

	  i = (i==49) ? 0 : i+1 ;
	  return *(ptr + i);

  }
}

/******************************************************************************
**                            End Of File
******************************************************************************/
