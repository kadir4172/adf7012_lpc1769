/* trackuino copyright (C) 2010  EA5HAV Javi
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

/* Credit to:
 *
 * Michael Smith for his Example of Audio generation with two timers and PWM:
 * http://www.arduino.cc/playground/Code/PCMAudio
 *
 * Ken Shirriff for his Great article on PWM:
 * http://arcfn.com/2009/07/secrets-of-arduino-pwm.html
 *
 * The large group of people who created the free AVR tools.
 * Documentation on interrupts:
 * http://www.nongnu.org/avr-libc/user-manual/group__avr__interrupts.html
 */

#include "config.h"
#include "audio_tone.h"
#include "utility.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "LPC17xx.h"                        /* LPC17xx definitions */
#include "dac.h"
#include "timer.h"
#include "adf7012.h"

const uint8_t sine_table[512] = {
  127, 129, 130, 132, 133, 135, 136, 138, 139, 141, 143, 144, 146, 147, 149, 150, 152, 153, 155, 156, 158,
  159, 161, 163, 164, 166, 167, 168, 170, 171, 173, 174, 176, 177, 179, 180, 182, 183, 184, 186, 187, 188,
  190, 191, 193, 194, 195, 197, 198, 199, 200, 202, 203, 204, 205, 207, 208, 209, 210, 211, 213, 214, 215,
  216, 217, 218, 219, 220, 221, 223, 224, 225, 226, 227, 228, 228, 229, 230, 231, 232, 233, 234, 235, 236,
  236, 237, 238, 239, 239, 240, 241, 242, 242, 243, 244, 244, 245, 245, 246, 247, 247, 248, 248, 249, 249,
  249, 250, 250, 251, 251, 251, 252, 252, 252, 253, 253, 253, 253, 254, 254, 254, 254, 254, 254, 254, 254,
  254, 254, 255, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 253, 253, 253, 253, 252, 252, 252, 251,
  251, 251, 250, 250, 249, 249, 249, 248, 248, 247, 247, 246, 245, 245, 244, 244, 243, 242, 242, 241, 240,
  239, 239, 238, 237, 236, 236, 235, 234, 233, 232, 231, 230, 229, 228, 228, 227, 226, 225, 224, 223, 221,
  220, 219, 218, 217, 216, 215, 214, 213, 211, 210, 209, 208, 207, 205, 204, 203, 202, 200, 199, 198, 197,
  195, 194, 193, 191, 190, 188, 187, 186, 184, 183, 182, 180, 179, 177, 176, 174, 173, 171, 170, 168, 167,
  166, 164, 163, 161, 159, 158, 156, 155, 153, 152, 150, 149, 147, 146, 144, 143, 141, 139, 138, 136, 135,
  133, 132, 130, 129, 127, 125, 124, 122, 121, 119, 118, 116, 115, 113, 111, 110, 108, 107, 105, 104, 102,
  101,  99,  98,  96,  95,  93,  91,  90,  88,  87,  86,  84,  83,  81,  80,  78,  77,  75,  74,  72,  71,
   70,  68,  67,  66,  64,  63,  61,  60,  59,  57,  56,  55,  54,  52,  51,  50,  49,  47,  46,  45,  44,
   43,  41,  40,  39,  38,  37,  36,  35,  34,  33,  31,  30,  29,  28,  27,  26,  26,  25,  24,  23,  22,
   21,  20,  19,  18,  18,  17,  16,  15,  15,  14,  13,  12,  12,  11,  10,  10,   9,   9,   8,   7,   7,
    6,   6,   5,   5,   5,   4,   4,   3,   3,   3,   2,   2,   2,   1,   1,   1,   1,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,
    2,   2,   2,   3,   3,   3,   4,   4,   5,   5,   5,   6,   6,   7,   7,   8,   9,   9,  10,  10,  11,
   12,  12,  13,  14,  15,  15,  16,  17,  18,  18,  19,  20,  21,  22,  23,  24,  25,  26,  26,  27,  28,
   29,  30,  31,  33,  34,  35,  36,  37,  38,  39,  40,  41,  43,  44,  45,  46,  47,  49,  50,  51,  52,
   54,  55,  56,  57,  59,  60,  61,  63,  64,  66,  67,  68,  70,  71,  72,  74,  75,  77,  78,  80,  81,
   83,  84,  86,  87,  88,  90,  91,  93,  95,  96,  98,  99, 101, 102, 104, 105, 107, 108, 110, 111, 113,
  115, 116, 118, 119, 121, 122, 124, 125
};


// This is a 8% scaled down version of the above
// in order to operate the PWM not too close to its extreme limits.
// Trying to prevent distortion this way.
/*
const uint8_t sine_table[512] = {
128,128,128,128,128,128,128,128,129,129,129,129,129,129,129,129,129,130,130,130,130,
130,130,130,130,131,131,131,131,131,131,131,131,132,132,132,132,132,132,132,132,132,
133,133,133,133,133,133,133,133,133,133,134,134,134,134,134,134,134,134,134,134,135,
135,135,135,135,135,135,135,135,135,135,135,136,136,136,136,136,136,136,136,136,136,
136,136,136,136,137,137,137,137,137,137,137,137,137,137,137,137,137,137,137,137,137,
137,137,137,137,137,137,138,138,138,138,138,138,138,138,138,138,138,138,138,138,138,
138,138,138,138,138,138,138,138,138,138,138,138,138,138,138,138,138,138,138,138,137,
137,137,137,137,137,137,137,137,137,137,137,137,137,137,137,137,137,137,137,137,137,
137,136,136,136,136,136,136,136,136,136,136,136,136,136,136,135,135,135,135,135,135,
135,135,135,135,135,135,134,134,134,134,134,134,134,134,134,134,133,133,133,133,133,
133,133,133,133,133,132,132,132,132,132,132,132,132,132,131,131,131,131,131,131,131,
131,130,130,130,130,130,130,130,130,129,129,129,129,129,129,129,129,129,128,128,128,
128,128,128,128,128,127,127,127,127,127,127,127,126,126,126,126,126,126,126,126,126,
125,125,125,125,125,125,125,125,124,124,124,124,124,124,124,124,123,123,123,123,123,
123,123,123,123,122,122,122,122,122,122,122,122,122,122,121,121,121,121,121,121,121,
121,121,121,120,120,120,120,120,120,120,120,120,120,120,120,119,119,119,119,119,119,
119,119,119,119,119,119,119,119,118,118,118,118,118,118,118,118,118,118,118,118,118,
118,118,118,118,118,118,118,118,118,118,117,117,117,117,117,117,117,117,117,117,117,
117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,
117,117,117,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,
118,118,118,118,118,119,119,119,119,119,119,119,119,119,119,119,119,119,119,120,120,
120,120,120,120,120,120,120,120,120,120,121,121,121,121,121,121,121,121,121,121,122,
122,122,122,122,122,122,122,122,122,123,123,123,123,123,123,123,123,123,124,124,124,
124,124,124,124,124,125,125,125,125,125,125,125,125,126,126,126,126,126,126,126,126,
126,127,127,127,127,127,127,127
};
*/
/* The sine_table is the carrier signal. To achieve phase continuity, each tone
 * starts at the index where the previous one left off. By changing the stride of
 * the index (phase_delta) we get 1200 or 2200 Hz. The PHASE_DELTA_XXXX values
 * can be calculated as:
 *
 * Fg = frequency of the output tone (1200 or 2200)
 * Fm = sampling rate (PLAYBACK_RATE_HZ)
 * Tt = sine table size (TABLE_SIZE)
 *
 * PHASE_DELTA_Fg = Tt*(Fg/Fm)
 */

//Globals
_Bool PTT_OFF = FALSE;
uint32_t modem_packet_size = 0;
uint8_t modem_packet[MODEM_MAX_PACKET];


// Source-specific
static const int TABLE_SIZE          = sizeof(sine_table);
static const uint32_t PLAYBACK_RATE   = 100000;    // 100KHz (10us lik timer baslatilacak)
static const int BAUD_RATE                 = 1200;
static  uint8_t SAMPLES_PER_BAUD ;

static uint32_t PHASE_DELTA_1200;//
static uint32_t PHASE_DELTA_2200;

static uint8_t current_byte;
static uint8_t current_sample_in_baud;    // 1 bit = SAMPLES_PER_BAUD samples

static _Bool MODEM_TRANSMITTING = FALSE;

static uint32_t phase_delta;                // 1200/2200 for standard AX.25
static uint32_t phase;                      // Fixed point 9.7 (2PI = TABLE_SIZE)
static uint32_t packet_pos;                 // Next bit to be sent out

void Modem_Init(){
	SAMPLES_PER_BAUD = (PLAYBACK_RATE / BAUD_RATE); // 52.083333333 / 26.041666667
	PHASE_DELTA_1200 = (((TABLE_SIZE * 1200L) << 7) / PLAYBACK_RATE); // Fixed point 9.7 // 1258 / 2516
	PHASE_DELTA_2200 = (((TABLE_SIZE * 2200L) << 7) / PLAYBACK_RATE); // 2306 / 4613
}

void Modem_Setup()
{
   Modem_Init();
   Radio_Setup();
}

_Bool Modem_Busy()
{
  return MODEM_TRANSMITTING;
}

int Modem_Get_Powerlevel()
{
  return Get_Powerlevel();
}

void Modem_Set_Tx_Freq(uint32_t freq)
{
  Set_Freq(freq);
}

void Modem_Flush_Frame()
{
  phase_delta = PHASE_DELTA_1200;
  phase = 0;
  packet_pos = 0;
  current_sample_in_baud = 0;
  MODEM_TRANSMITTING = TRUE;

  // Key the radio
  Ptt_On();

  Delay_ms(100);
  Reset_Timer();
  Init_Timer(10);                 //10us intervalinde timer0 baslat
  Enable_Timer();                 //Timer0 enable et
}

// This is called at PLAYBACK_RATE Hz to load the next sample.
void Sinus_Generator(void) {

	uint32_t Audio_Signal;

if (MODEM_TRANSMITTING) {

    // If done sending packet
    if (packet_pos == modem_packet_size) {
      MODEM_TRANSMITTING = FALSE;             // End of transmission
      //OCR2 = REST_DUTY;       // Output 0v (after DC coupling)

      //TIMSK2 &= ~_BV(TOIE2);  // Disable playback interrupt.
      Reset_Timer();
      Disable_Timer();
      Send_Vcxo_Signal(0); //DAC cikisini sifirlayalim
      //LPC_DAC->CR = 0;   //DAC cikisini sifirlayalim

      //LPC_TIM0->TCR = 0;
      PTT_OFF = TRUE;
      goto end_generator;           // Done, gather ISR stats
    }

    // If sent SAMPLES_PER_BAUD already, go to the next bit
    if (current_sample_in_baud == 0) {    // Load up next bit
      if ((packet_pos & 7) == 0)          // Load up next byte
        current_byte = modem_packet[packet_pos >> 3];
      else
        current_byte = current_byte / 2;  // ">>1" forces int conversion //ikiye bolmek hosdegil test edilsin kadir
      if ((current_byte & 1) == 0) {
        // Toggle tone (1200 <> 2200)
        phase_delta ^= (PHASE_DELTA_1200 ^ PHASE_DELTA_2200);
      }
    }

    phase += phase_delta;

    Audio_Signal = *(sine_table + ((phase >> 7) & (TABLE_SIZE - 1)));
    //LPC_DAC->CR = ((Audio_Signal) << 6) | DAC_BIAS;
    Send_Vcxo_Signal(((Audio_Signal << 6) | DAC_BIAS)); //DAC cikisina ornegi yazalim
    if(++current_sample_in_baud == SAMPLES_PER_BAUD) {
      current_sample_in_baud = 0;
      packet_pos++;
    }
  }

end_generator:

  return;
}
