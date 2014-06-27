/****************************************************************************
 *   $Id:: adc.c 6089 2011-01-06 04:38:09Z nxp12832                         $
 *   Project: NXP LPC17xx ADC example
 *
 *   Description:
 *     This file contains ADC code example which include ADC 
 *     initialization, ADC interrupt handler, and APIs for ADC
 *     reading.
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
#include "adc.h"
#include "common_variables.h"

volatile uint32_t ADCValue;

/*****************************************************************************
** Function name:		ADCInit
**
** Descriptions:		initialize ADC channel
**
** parameters:			ADC clock rate
** Returned value:		None
** 
*****************************************************************************/
void ADCInit( uint32_t ADC_Clk )
{
  uint32_t  pclkdiv, pclk;

  /* Enable CLOCK into ADC controller */
  LPC_SC->PCONP |= (1 << 12);


	ADCValue = 0x0;


  /* all the related pins are set to ADC inputs, AD0.7 */
  LPC_PINCON->PINSEL0 &= ~0x00000030;	/* P0.2, A0.7, function 10 */
  LPC_PINCON->PINSEL0 |= 0x00000020;

  LPC_PINCON->PINMODE0 &= ~0x00000030;
  LPC_PINCON->PINMODE0 |= 0x00000020;


  /* By default, the PCLKSELx value is zero, thus, the PCLK for
  all the peripherals is 1/4 of the SystemFrequency. */
  /* Bit 24~25 is for ADC */
  pclkdiv = (LPC_SC->PCLKSEL0 >> 24) & 0x03;
  switch ( pclkdiv )
  {
	case 0x00:
	default:
	  pclk = SystemFrequency/4;
	break;
	case 0x01:
	  pclk = SystemFrequency;
	break; 
	case 0x02:
	  pclk = SystemFrequency/2;
	break; 
	case 0x03:
	  pclk = SystemFrequency/8;
	break;
  }

  LPC_ADC->CR = ( 0x01 << 0 ) |  /* SEL=1,select channel 0~7 on ADC0 */
		( ( pclk  / ADC_Clk - 1 ) << 8 ) |  /* CLKDIV = Fpclk / ADC_Clk - 1 */ 
		( 0 << 16 ) | 		/* BURST = 0, no BURST, software controlled */
		( 0 << 17 ) |  		/* CLKS = 0, 11 clocks/10 bits */
		( 1 << 21 ) |  		/* PDN = 1, normal operation */
		( 0 << 24 ) |  		/* START = 0 A/D conversion stops */
		( 0 << 27 );		/* EDGE = 0 (CAP/MAT singal falling,trigger A/D conversion) */ 


  return;
}

/*****************************************************************************
** Function name:		ADCRead
**
** Descriptions:		Read ADC channel
**
** parameters:			Channel number
** Returned value:		Value read, if interrupt driven, return channel #
** 
*****************************************************************************/
uint32_t ADCRead( uint8_t channelNum )
{

  uint32_t regVal, ADC_Data;

  /* channel number is 0 through 7 */
  if ( channelNum >= ADC_NUM )
  {
	channelNum = 0;		/* reset channel number to 0 */
  }
  LPC_ADC->CR &= 0xFFFFFF00;
  LPC_ADC->CR |= (1 << 24) | (1 << channelNum);	
				/* switch channel,start A/D convert */

  timeout_check = Set_Delay(1); //10ms lik timeout exception yaratalim
  while ( 1 )			/* wait until end of A/D convert */
  {
	regVal = LPC_ADC->DR[channelNum];
	/* read result of A/D conversion */
	if ( regVal & ADC_DONE )
	{
	  break;
	}

	if(Check_Delay(timeout_check)){
	  timeout_flag |= 0xFF;
	  break;
    }
  }

  LPC_ADC->CR &= 0xF8FFFFFF;	/* stop ADC now */
  if ( regVal & ADC_OVERRUN )	/* save data when it's not overrun, otherwise, return zero */
  {
	return ( 0 );
  }
  ADC_Data = ( regVal >> 4 ) & 0xFFF;
  return ( ADC_Data );	/* return A/D conversion value */

}

/*********************************************************************************
**                            End Of File
*********************************************************************************/
