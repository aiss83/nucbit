/*
 * spi.h
 *
 *  Created on: 02 сент. 2014 г.
 *      Author: 17095
 */

#ifndef SPI_H_
#define SPI_H_

#include PLATFORM_HEADER
#include "hal/micro/micro-common.h"
#include "hal/micro/cortexm3/micro-common.h"


int spi2_init(int clk);

char spi2_xfer(char outbyte);
int spi2_read(char* data, int size);
int spi2_write(char* data, int size);
void spi2_enable_cs();
void spi2_disable_cs();

#endif /* SPI_H_ */
