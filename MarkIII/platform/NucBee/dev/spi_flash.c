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

static sflast_stsreg_t fl_status() {
	spi2_enable_cs();
	sflast_stsreg_t ret;
	int tmp = CMD_READ_STS;
	spi2_write((char*) &tmp, 1);
	spi2_read((char*) &ret, 1); // with extra dummy-byte

	spi2_disable_cs();
	return (ret);
}

static inline void fl_wait_rdy(){
	while(!fl_status().ready);
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
	fl_wait_rdy();
	spi2_enable_cs();

	int tmp = CMD_CNTREAD_HF;
	spi2_write((char*) &tmp, 1);

	tmp = fl_makeaddr(adr);
	spi2_write((char*) &tmp, 4); // with extra dummy-byte

	tmp = spi2_read(data, size);

	spi2_disable_cs();
	return tmp;
}

static int spi_flash_write_page( int page, char *data,int size) {
	fl_wait_rdy();
	spi2_enable_cs();

	int tmp = CMD_MAIN_THROUGH_BUF1WRITE;
	spi2_write((char*) &tmp, 1);

	tmp = 0 | (page <<8); //block address will be always 0

	spi2_write((char*) &tmp, 3);
	tmp = spi2_write(data, size);

	spi2_disable_cs();
	return tmp;
}


int spi_flash_write(char *data, int adr, int size) {
	int tmp = 0;
	short pnum, cpage, wsize;
	int shift,pos;

	pnum = size / FLPAGESIZE;
	if (size % FLPAGESIZE)
		pnum++;

	cpage = adr / FLPAGESIZE;
	shift = adr - (cpage*FLPAGESIZE);

	for (pos = 0;cpage <= cpage+pnum; cpage++){
		wsize = min(FLPAGESIZE, size);
		tmp = spi_flash_write_page(cpage, data[pos], wsize);

		size = size - wsize;

	}

	return tmp;
}

