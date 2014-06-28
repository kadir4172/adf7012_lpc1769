/*
 * ax25.h
 *
 *  Created on: Jun 28, 2014
 *      Author: kadir
 */

#ifndef AX25_H_
#define AX25_H_

#include <stdint.h>


typedef struct  {
	uint8_t callsign[7];
	uint8_t ssid;
} s_address;

//void ax25_send_header(struct s_address *addresses, int num_addresses);
void Ax25_Send_Header(s_address addresses[], int num_addresses);
void Ax25_Send_Byte(uint8_t byte);
void Ax25_Send_String(const char *string);
void Ax25_Send_Footer();
void Ax25_Flush_Frame();




#endif /* AX25_H_ */
