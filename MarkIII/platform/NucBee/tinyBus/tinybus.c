/*
 * tinybus-reciever.c
 *
 *	Micro-implementation of modbus protocol
 *	As we don't need all parsing abilities of modbus stack - only receive-by-timeout and target address determination
 *
 *  Created on: 20 дек. 2013 г.
 *      Author: 17095
 */

#include "contiki.h"
#include "dev/uart1.h"
#include "sys/rtimer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tinybus_filter.h"

char tinybus_our_packet(unsigned char *ptr);

//static char packet_complete = 0;
//static char packet_going = 0;
static int (*packet_recieved_cb)(unsigned char *ptr, unsigned int size);
static int tinybus_input_byte(unsigned char c) ;
static rtimer_clock_t packet_tout = 0xffffffff;

static unsigned char busBuffer[BUS_BUFFERSIZE];
static int rsize = 0;
static struct rtimer rt;

void tinybus_set_recieve_cb(int (*input)(unsigned char *ptr, unsigned int size)) {
	//don't registering own uart recieve cb until we have no pstream recieve cb
	if (input != NULL) {
		packet_recieved_cb = input;
		uart1_set_input(tinybus_input_byte);
	} else {//disabling callbacks
		packet_recieved_cb = NULL;
		uart1_set_input(NULL);
	}
}

//packet timer callback with pretty crazy "critical section"
static char input_timeout(struct rtimer *t, void *ptr) {
	//packet_complete = 1;
	INTERRUPTS_OFF()
	;


	if (tinybus_our_packet(busBuffer))
		packet_recieved_cb(busBuffer, rsize);

	rsize = 0;
	INTERRUPTS_ON()
	;
	return 0;
}

//rtimer setup
void inline packet_timer_reset() {
	//if (rt.time)
	rtimer_set(&rt, RTIMER_NOW() + packet_tout, 1,
			(void (*)(struct rtimer *, void *)) input_timeout, NULL);
}

//uart input-handler callback
static int tinybus_input_byte(unsigned char c) {
	packet_timer_reset(); //resetting "watchdog"
	if (rsize < BUS_BUFFERSIZE) {
		busBuffer[rsize] = c;
		rsize++;
	}
	return 0;
}

//tinybus send
int tinybus_send(unsigned char *ptr, unsigned int size) {
	//int ret = 0;
	int i;
	//NOOOOOOOOOOO!!!!!!!!!!!!!!!!1
	for (i = 0; i < size; i++) {
		uart1_writeb(ptr[i]);
	}

	return i;
}

//
int tinybus_init(unsigned long baud) {
	int ret = 0;
	 tinybus_filter_init();
	 ///keep it here by now
	 tinybus_setbypass(TRUE);
	//uart setup
	uart1_init(baud);
	//calculating packet timer
	packet_tout = RTIMER_ARCH_SECOND / (baud / 9); //1 bit time
	packet_tout++;
	//packet_tout *= 9; //8 data bit + 1 stop bit //1 word time

	packet_tout = packet_tout * 3 + packet_tout / 2; //i hope this will work
	packet_tout++;
	//inserting receive callback

	return ret;
}
