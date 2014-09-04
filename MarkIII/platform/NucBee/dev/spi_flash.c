/*
 * spi_flash.c
 *
 *  Created on: 03 сент. 2014 г.
 *      Author: 17095
 */

#include "spi.h"
#include "spi_flash.h"

int spi_flash_init() {
	int ret = 0;

	return ret;
}

static inline int fl_makeaddr(int adr) {
	int ret = 0;

	int pagen, shift;

	pagen = adr % 512;

	shift = adr - (pagen * 512);

	//fine shift
	ret = (shift & 0x3ff) << 12 | (pagen & 0xfff);

	return ret;
}

int spi_flash_read(char *data, int adr, int size) {
	spi2_enable_cs();

	int tmp = CMD_CNTREAD_HF;
	spi2_write((char*) &tmp, 1);

	tmp = fl_makeaddr(adr);
	spi2_write((char*) &tmp, 4); // with extra dummy-byte

	tmp = spi2_read(data, size);

	spi2_disable_cs();

	return tmp;
}

int spi_flash_write(char *data, int adr, int size) {
	spi2_enable_cs();

	int tmp = CMD_MAIN_READ;
	spi2_write((char*) &tmp, 1);

	tmp = fl_makeaddr(adr);
	spi2_write((char*) &tmp, 4); // with extra dummy-byte

	tmp = spi2_write(data, size);

	spi2_disable_cs();

	return tmp;
}
