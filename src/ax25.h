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
void ax25_send_header(s_address addresses[], int num_addresses);

void ax25_send_byte(uint8_t byte);
void ax25_send_string(const char *string);
void ax25_send_footer();
void ax25_flush_frame();




#endif /* AX25_H_ */
