/*
 * spi.c
 *
 *  Created on: 02 сент. 2014 г.
 *      Author: 17095
 */

#include "spi.h"

#define SPICSPIN PORTx_PIN(PORTC, 1)

int spi2_init(int clk) {
	int ret = 0;

	halGpioConfig(SPICSPIN, GPIOCFG_OUT);

	SC2_MODE = 2; //SPI mode

	SC2_SPICFG = 0 | (0 << 5) //rx mode
			| (1 << 4) //master mode
			| (0 << 3) //repeat
			| (0 << 2) //lsb first
			| (0 << 1) //phase
			| (0 << 0) //polarity
			;

	SC2_TWICTRL2 = 1;

	//testing SPI on 1 MHz
	//that's will be LIN = 5, EXP = 1

	SC2_RATELIN = 5;
	SC2_RATEEXP = 1;

	return ret;
}

char spi2_xfer(char outbyte) {
	char inbyte = 0;
	//sending data
	SC2_DATA = outbyte;

	do {
		if (SC2_SPISTAT & SC_SPIRXVAL) {
			inbyte = SC2_DATA;
			break;
		}
		if (SC2_SPISTAT & SC_SPIRXOVF) {
			inbyte = SC2_DATA;
			continue;
		}
	} while (TRUE); //yep i know, funny

	return inbyte;
}

int spi2_read(char* data, int size) {
	int ret = 0;
	for (ret = 0; ret < size; ret++) {
		data[ret] = spi2_xfer(0xff);
	}
	return ret;
}

int spi2_write(char* data, int size) {
	int ret = 0;
	for (ret = 0; ret < size; ret++) {
		spi2_xfer(data[ret]);
	}
	return ret;
}


void spi2_enable_cs(){
	halGpioSet(SPICSPIN, TRUE);
}

void spi2_disable_cs(){
	halGpioSet(SPICSPIN, FALSE);
}
