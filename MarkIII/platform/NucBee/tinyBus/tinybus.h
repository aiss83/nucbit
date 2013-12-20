/*
 * tinybus.h
 *
 *  Created on: 20 дек. 2013 г.
 *      Author: 17095
 */

#ifndef TINYBUS_H_
#define TINYBUS_H_

void
tinybus_set_recieve_cb(int (*input)(unsigned char *ptr, unsigned int size));
int tinybus_send(unsigned char *ptr, unsigned int size);
int tinybus_init(unsigned long baud);

#endif /* TINYBUS_H_ */
