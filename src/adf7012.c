/*
 * test.c
 *
 *  Created on: Jun 25, 2014
 *      Author: kadir
 */


/* pecan copyright (C) 2012  KT5TK
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include "config.h"
#include <math.h>
#include "adf7012.h"
#include <stdint.h>
#include "common_variables.h"
#include "LPC17xx.h"
#include <string.h>


extern _Bool Init_Adf7012(void); //kadir
const int MAX_RES = 16;
//char res_adf7012[MAX_RES];
uint32_t powerlevel;


//Function Prototypes
void adf_reset_register_zero(void);
void adf_reset_register_one(void);
void adf_reset_register_two(void);
void adf_reset_register_three(void);
void adf_reset(void);
void adf_write_register_zero(void);
void adf_write_register_one(void);
void adf_write_register_two(void);
void adf_write_register_three(void);
void adf_write_register(uint8_t*);
_Bool adf_locked(void);
void ptt_off(void);
void setup(void);


//Function Prototypes
// Configuration storage structs =============================================
struct {
    struct {
        uint32_t  frequency_error_correction;
        uint8_t r_divider;
        uint8_t crystal_doubler;
        uint8_t crystal_oscillator_disable;
        uint8_t clock_out_divider;
        uint8_t vco_adjust;
        uint8_t output_divider;
    } r0;

    struct {
        uint32_t  fractional_n;
        uint8_t integer_n;
        uint8_t prescaler;
    } r1;

    struct {
        uint8_t mod_control;
        uint8_t gook;
        uint8_t power_amplifier_level;
        uint32_t  modulation_deviation;
        uint8_t gfsk_modulation_control;
        uint8_t index_counter;
    } r2;

    struct {
        uint8_t pll_enable;
        uint8_t pa_enable;
        uint8_t clkout_enable;
        uint8_t data_invert;
        uint8_t charge_pump_current;
        uint8_t bleed_up;
        uint8_t bleed_down;
        uint8_t vco_disable;
        uint8_t muxout;
        uint8_t ld_precision;
        uint8_t vco_bias;
        uint8_t pa_bias;
        uint8_t pll_test_mode;
        uint8_t sd_test_mode;
    } r3;
} adf_config;




// Write directly to AVR port in SPIwrite() instead of using digitalWrite()
//#define FAST_IO

// Configuration functions ===================================================

// Config resetting functions --------------------------------------------
void adf_reset_config(void)
{

    adf_reset_register_zero();
    adf_reset_register_one();
    adf_reset_register_two();
    adf_reset_register_three();

    adf_reset();




}

// Power up default settings are defined here:
//uint8_t register0[4] = {0x04, 0x11, 0xE0, 0x00};
//uint8_t register1[3] = {0x5B, 0x40, 0x01}      ;
//uint8_t register2[4] = {0x00, 0x00, 0x81, 0xEE};
//uint8_t register3[4] = {0x00, 0x45, 0x20, 0xFF};
void adf_reset_register_zero(void) {
    adf_config.r0.frequency_error_correction = 0;               // Don't bother for now...
    adf_config.r0.r_divider = ADF7012_CRYSTAL_DIVIDER;          // Whatever works best for 2m, 1.25m and 70 cm ham bands
    adf_config.r0.crystal_doubler = 0;                          // Who would want that? Lower f_pfd means finer channel steps.
    adf_config.r0.crystal_oscillator_disable = 0;               // Enable traditional xtal
    adf_config.r0.clock_out_divider = 2;                        // Don't bother for now...
    adf_config.r0.vco_adjust = 0;                               // Don't bother for now... (Will be automatically adjusted until PLL lock is achieved)
    adf_config.r0.output_divider = ADF_OUTPUT_DIVIDER_BY_4;     // Pre-set div 4 for 2m. Will be changed according tx frequency on the fly
}

void adf_reset_register_one(void) {
    adf_config.r1.integer_n = 109;                              // Pre-set for 144.390 MHz APRS. Will be changed according tx frequency on the fly
    adf_config.r1.fractional_n = 0;                          // Pre-set for 144.390 MHz APRS. Will be changed according tx frequency on the fly
    adf_config.r1.prescaler = ADF_PRESCALER_8_9;                // 8/9 requires an integer_n > 91; 4/5 only requires integer_n > 31
}

void adf_reset_register_two(void) {
    adf_config.r2.mod_control = ADF_MODULATION_OOK;             // For AFSK the modulation is done through the external VCXO we don't want any FM generated by the ADF7012 itself
    adf_config.r2.gook = 0;                                     // Whatever... This might give us a nicer swing in phase maybe...
    adf_config.r2.power_amplifier_level = 15;                   // 16 is about half maximum power. Output −20dBm at 0x0, and 13 dBm at 0x7E at 868 MHz
    adf_config.r2.modulation_deviation = 16;                    // 16 is about half maximum amplitude @ ASK.
    adf_config.r2.gfsk_modulation_control = 0;                  // Don't bother for now...
    adf_config.r2.index_counter = 0;                            // Don't bother for now...
}

void adf_reset_register_three(void) {
    adf_config.r3.pll_enable = 0;                               // Switch off PLL (will be switched on after Ureg is checked and confirmed ok)
    adf_config.r3.pa_enable = 0;                                // Switch off PA  (will be switched on when PLL lock is confirmed)
	adf_config.r3.clkout_enable = 1;                            // Clock out enable
    adf_config.r3.data_invert = 1;                              // Results in a TX signal when TXDATA input is low
    adf_config.r3.charge_pump_current = ADF_CP_CURRENT_2_1;     // 2.1 mA. This is the maximum
    adf_config.r3.bleed_up = 0;                                 // Don't worry, be happy...
    adf_config.r3.bleed_down = 0;                               // Dito
    adf_config.r3.vco_disable = 0;                              // VCO is on

    adf_config.r3.muxout = 4;                // Lights up the green LED if the ADF7012 is properly powered (changes to lock detection in a later stage)

    adf_config.r3.ld_precision = ADF_LD_PRECISION_3_CYCLES;     // What the heck? It is recommended that LDP be set to 1; 0 is more relaxed
    adf_config.r3.vco_bias = 5;                                 // In 0.5 mA steps; Default 6 means 3 mA; Maximum (15) is 8 mA
    adf_config.r3.pa_bias = 4;                                  // In 1 mA steps; Default 4 means 8 mA; Minimum (0) is 5 mA; Maximum (7) is 12 mA (Datasheet says uA which is bullshit)
    adf_config.r3.pll_test_mode = 0;
    adf_config.r3.sd_test_mode = 0;
}

void adf_reset(void) {

	ADF7021_CHIP_POWER_UP;
    //digitalWrite(ADF7012_TX_DATA_PIN, HIGH);
    //digitalWrite(SCKpin,  HIGH);
    //digitalWrite(MOSIpin, HIGH);

	Delay_ms(10);

}

// Configuration writing functions ---------------------------------------
void adf_write_config(void) {
    adf_write_register_zero();
    adf_write_register_one();
    adf_write_register_two();
    adf_write_register_three();
}

void adf_write_register_zero(void) {

    uint32_t reg =
        (0) |
        ((uint32_t)(adf_config.r0.frequency_error_correction & 0x7FF) << 2U) |
        ((uint32_t)(adf_config.r0.r_divider & 0xF ) << 13U) |
        ((uint32_t)(adf_config.r0.crystal_doubler & 0x1 ) << 17U) |
        ((uint32_t)(adf_config.r0.crystal_oscillator_disable & 0x1 ) << 18U) |
        ((uint32_t)(adf_config.r0.clock_out_divider & 0xF ) << 19U) |
        ((uint32_t)(adf_config.r0.vco_adjust & 0x3 ) << 23U) |
        ((uint32_t)(adf_config.r0.output_divider & 0x3 ) << 25U);


    uint8_t reg_ptr[4];
    memcpy(reg_ptr, &reg, 4);
    Reverse_Array(reg_ptr,4);
    Write_Adf7012_Reg(reg_ptr, 4);
}

void adf_write_register_one(void) {
    uint32_t reg =
        (1) |
        ((uint32_t)(adf_config.r1.fractional_n & 0xFFF) << 2) |
        ((uint32_t)(adf_config.r1.integer_n & 0xFF ) << 14) |
        ((uint32_t)(adf_config.r1.prescaler & 0x1 ) << 22);

    uint8_t reg_ptr[3];
    memcpy(reg_ptr, &reg, 3);
    Reverse_Array(reg_ptr,3);
    Write_Adf7012_Reg(reg_ptr, 3);
}

void adf_write_register_two(void) {
    uint32_t reg =
        (2) |
        ((uint32_t)(adf_config.r2.mod_control & 0x3 ) << 2) |
        ((uint32_t)(adf_config.r2.gook & 0x1 ) << 4) |
        ((uint32_t)(adf_config.r2.power_amplifier_level & 0x3F ) << 5) |
        ((uint32_t)(adf_config.r2.modulation_deviation & 0x1FF) << 11) |
        ((uint32_t)(adf_config.r2.gfsk_modulation_control & 0x7 ) << 20) |
        ((uint32_t)(adf_config.r2.index_counter & 0x3 ) << 23);


    uint8_t reg_ptr[4];
    memcpy(reg_ptr, &reg, 4);
    Reverse_Array(reg_ptr,4);
    Write_Adf7012_Reg(reg_ptr, 4);
}

void adf_write_register_three(void) {
    uint32_t reg =
        (3) |
        ((uint32_t)(adf_config.r3.pll_enable & 0x1 ) << 2) |
        ((uint32_t)(adf_config.r3.pa_enable & 0x1 ) << 3) |
        ((uint32_t)(adf_config.r3.clkout_enable & 0x1 ) << 4) |
        ((uint32_t)(adf_config.r3.data_invert & 0x1 ) << 5) |
        ((uint32_t)(adf_config.r3.charge_pump_current & 0x3 ) << 6) |
        ((uint32_t)(adf_config.r3.bleed_up & 0x1 ) << 8) |
        ((uint32_t)(adf_config.r3.bleed_down & 0x1 ) << 9) |
        ((uint32_t)(adf_config.r3.vco_disable & 0x1 ) << 10) |
        ((uint32_t)(adf_config.r3.muxout & 0xF ) << 11) |
        ((uint32_t)(adf_config.r3.ld_precision & 0x1 ) << 15) |
        ((uint32_t)(adf_config.r3.vco_bias & 0xF ) << 16) |
        ((uint32_t)(adf_config.r3.pa_bias & 0x7 ) << 20) |
        ((uint32_t)(adf_config.r3.pll_test_mode & 0x1F ) << 23) |
        ((uint32_t)(adf_config.r3.sd_test_mode & 0xF ) << 28);


    uint8_t reg_ptr[4];
    memcpy(reg_ptr, &reg, 4);
    Reverse_Array(reg_ptr,4);
    Write_Adf7012_Reg(reg_ptr, 4);
}

int adf_lock(void)
{
    // fiddle around with bias and adjust capacity until the vco locks
	Delay_ms(200);
    int adj = adf_config.r0.vco_adjust; // use default start values from setup
    int bias = adf_config.r3.vco_bias;  // or the updated ones that worked last time

    adf_config.r3.pll_enable = 1;
    adf_config.r3.muxout = ADF_MUXOUT_DIGITAL_LOCK;
    adf_write_config();
    Delay_ms(5);
    //adf_locked();

    while(!adf_locked()) {

        adf_config.r0.vco_adjust = adj;
        adf_config.r3.vco_bias = bias;
        adf_config.r3.muxout = ADF_MUXOUT_DIGITAL_LOCK;
        adf_write_config();
        Delay_ms(5);
        if(++bias == 14) {
            bias = 1;
            if(++adj == 4) {
               // Serial.println("Couldn't achieve PLL lock :( ");
                // Using best guess defaults:
                adf_config.r0.vco_adjust = 0;
                adf_config.r3.vco_bias = 5;

                return 0;
            }
        }
    }



    return 1;
}

_Bool adf_locked(void)
{
  uint32_t adc;
  if(!Read_Adf7012_Muxout(&adc))
	return FALSE; //ADC conversion is not successful

  Delay_ms(10);
  if (adc > 3000) //digital lock ok
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}


void set_freq(uint32_t freq)
{

  // Set the output divider according to recommended ranges given in ADF7012 datasheet
  // 2012-08-10 TK lowered the borders a bit in order to keep n high enough for 144, 222 and 430 MHz amateur bands
  // with a constant crystal divider of 8
  adf_config.r0.output_divider = ADF_OUTPUT_DIVIDER_BY_1;
  if (freq < 410000000) { adf_config.r0.output_divider = ADF_OUTPUT_DIVIDER_BY_2; };
  if (freq < 210000000) { adf_config.r0.output_divider = ADF_OUTPUT_DIVIDER_BY_4; };
  if (freq < 130000000) { adf_config.r0.output_divider = ADF_OUTPUT_DIVIDER_BY_8; };

  uint32_t f_pfd = ADF7012_CRYSTAL_FREQ / adf_config.r0.r_divider;

  uint32_t n = (uint32_t)(freq / f_pfd);

  float ratio = (float)freq / (float)f_pfd;
  float rest  = ratio - (float)n;


  uint32_t m = (uint32_t)(rest * 4096);

  adf_config.r1.integer_n = n;
  adf_config.r1.fractional_n = m;

}


void Radio_Setup()
{
  //pinMode(PTT_PIN, OUTPUT);

  Gpio_Config();


  adf_reset_config();
  set_freq(RADIO_FREQUENCY); // Set the default frequency
  adf_write_config();

  //digitalWrite(ADF7012_TX_DATA_PIN, LOW);

  Delay_ms(10);
 }


void Ptt_On()
{

  //digitalWrite(PTT_PIN, HIGH);
  //digitalWrite(ADF7012_TX_DATA_PIN, LOW);
  adf_config.r3.pa_enable = 0;
  adf_config.r2.power_amplifier_level = 0;
  adf_config.r3.muxout = ADF_MUXOUT_REG_READY;

  adf_write_config();
  Delay_ms(10);

  // Do we have good power on the ADF7012 voltage regulator?
    uint32_t adc;
    if(!Read_Adf7012_Muxout(&adc))
  	return; //ADC conversion is not successful

  if (adc < 3000)  // Power is bad
  {
    //Serial.println("ERROR: Can't power up the ADF7012!");
  }
  else              // Power is good apparently
  {

    adf_lock();
    adf_config.r3.pa_enable = 1;
    adf_config.r2.power_amplifier_level = 63; //63 is max power

    Delay_ms(10);
    adf_write_config();
    Delay_ms(1);

    uint32_t powerlevel;
    if(!Read_Adf7012_Muxout(&powerlevel))
  	return; //ADC conversion is not successful

      if (powerlevel > 2000)
      {
        powerlevel = 2000;
      }

  }
}

void Ptt_Off()
{

  adf_config.r3.pa_enable = 0;
  adf_config.r2.power_amplifier_level = 0;
  adf_write_config();
  Delay_ms(10);

  //digitalWrite(PTT_PIN, LOW);
  //digitalWrite(ADF7012_TX_DATA_PIN, LOW);

}

int get_powerlevel()
{
  return powerlevel;
}
