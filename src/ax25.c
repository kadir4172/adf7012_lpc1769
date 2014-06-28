/*
 * ax25.c
 *
 *  Created on: Jun 28, 2014
 *      Author: kadir
 */


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

#include "ax25.h"
#include "config.h"
#include "audio_tone.h"

//Globals
uint16_t crc;
int16_t ones_in_a_row;


static void Update_Crc(uint8_t bit)
{
  crc ^= bit;
  if (crc & 1)
    crc = (crc >> 1) ^ 0x8408;  // X-modem CRC poly
  else
    crc = crc >> 1;
}

static void Send_Byte(uint8_t byte)
{
  int i;
  for (i = 0; i < 8; i++) {
    Update_Crc((byte >> i) & 1);
    if ((byte >> i) & 1) {
      // Next bit is a '1'
      if (modem_packet_size >= MODEM_MAX_PACKET * 8)  // Prevent buffer overrun
        return;
      modem_packet[modem_packet_size >> 3] |= (1 << (modem_packet_size & 7));
      modem_packet_size++;
      if (++ones_in_a_row < 5) continue;
    }
    // Next bit is a '0' or a zero padding after 5 ones in a row
    if (modem_packet_size >= MODEM_MAX_PACKET * 8)    // Prevent buffer overrun
      return;
    modem_packet[modem_packet_size >> 3] &= ~(1 << (modem_packet_size & 7));
    modem_packet_size++;
    ones_in_a_row = 0;
  }
}

// Exported functions
void Ax25_Send_Byte(uint8_t byte)
{
  // Wrap around send_byte, but prints debug info
  Send_Byte(byte);

}

void Ax25_Send_Sync()
{
  uint8_t byte = 0x00;
  int i;
  for (i = 0; i < 8; i++, modem_packet_size++) {
    if (modem_packet_size >= MODEM_MAX_PACKET * 8)  // Prevent buffer overrun
      return;
    if ((byte >> i) & 1)
      modem_packet[modem_packet_size >> 3] |= (1 << (modem_packet_size & 7));
    else
      modem_packet[modem_packet_size >> 3] &= ~(1 << (modem_packet_size & 7));
  }
}

void Ax25_Send_Flag()
{
  uint8_t byte = 0x7e;
  int i;
  for (i = 0; i < 8; i++, modem_packet_size++) {
    if (modem_packet_size >= MODEM_MAX_PACKET * 8)  // Prevent buffer overrun
      return;
    if ((byte >> i) & 1)
      modem_packet[modem_packet_size >> 3] |= (1 << (modem_packet_size & 7));
    else
      modem_packet[modem_packet_size >> 3] &= ~(1 << (modem_packet_size & 7));
  }
}

void Ax25_Send_String(const char *string)
{
  int i;
  for (i = 0; string[i]; i++) {
    Ax25_Send_Byte(string[i]);
  }
}

void Ax25_Send_Header(s_address addresses[], int num_addresses)
{
  int i, j;
  modem_packet_size = 0;
  ones_in_a_row = 0;
  crc = 0xffff;

  // Send sync ("a bunch of 0s")
  for (i = 0; i < 60; i++)
  {
    Ax25_Send_Sync();
  }

  //start the actual frame. Send 3 of them (one empty frame and the real start)
  for (i = 0; i < 3; i++)
  {
    Ax25_Send_Flag();
  }

  for (i = 0; i < num_addresses; i++) {
    // Transmit callsign
    for (j = 0; addresses[i].callsign[j]; j++)
      Send_Byte(addresses[i].callsign[j] << 1);
    // Transmit pad
    for ( ; j < 6; j++)
      Send_Byte(' ' << 1);
    // Transmit SSID. Termination signaled with last bit = 1
    if (i == num_addresses - 1)
      Send_Byte(('0' + addresses[i].ssid) << 1 | 1);
    else
      Send_Byte(('0' + addresses[i].ssid) << 1);
  }

  // Control field: 3 = APRS-UI frame
  Send_Byte(0x03);

  // Protocol ID: 0xf0 = no layer 3 data
  Send_Byte(0xf0);
}

void Ax25_Send_Footer()
{
  // Save the crc so that it can be treated it atomically
  uint16_t final_crc = crc;

  // Send the CRC
  Send_Byte(~(final_crc & 0xff));
  final_crc >>= 8;
  Send_Byte(~(final_crc & 0xff));

  // Signal the end of frame
  Ax25_Send_Flag();

}

void Ax25_Flush_Frame()
{
  // Key the transmitter and send the frame
  Modem_Flush_Frame();
}


