/*
 * common_variables.h
 *
 *  Created on: Jun 14, 2014
 *      Author: kadir
 */

#ifndef COMMON_VARIABLES_H_
#define COMMON_VARIABLES_H_
#include <stdint.h>

extern uint32_t Set_Delay (uint32_t);
extern uint8_t Check_Delay(uint32_t);
extern _Bool Delay_ms(uint32_t);
extern uint32_t timeout_check;
extern uint8_t timeout_flag;

extern _Bool Write_Adf7012_Reg(uint8_t*, uint8_t);
extern _Bool Gpio_Config(void);
extern _Bool Read_Adf7012_Muxout(uint32_t*);
extern _Bool Reverse_Array(uint8_t* ,uint8_t);
extern void Radio_Setup(void);
extern void Ptt_On(void);
extern void Ptt_Off(void);
extern int Get_Powerlevel(void);
extern void Set_Freq(uint32_t);

#define ADF7021_LOAD_REGISTER_ENABLE   LPC_GPIO0->FIOCLR |= (0x1<<0)  //P0.0 LE pini belirlendi
#define ADF7021_LOAD_REGISTER_DISABLE  LPC_GPIO0->FIOSET |= (0x1<<0)

#define ADF7021_CHIP_POWER_UP    LPC_GPIO0->FIOSET |= (0x1<<1)  //P0.1 CE  pini olarak belirlendi
#define ADF7021_CHIP_POWER_DOWN  LPC_GPIO0->FIOCLR |= (0x1<<1)

#define ADC_CHANNEL 7

#endif /* COMMON_VARIABLES_H_ */
