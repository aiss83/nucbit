/*
 * mzConfig.c
 *
 *  Created on: 08 сент. 2014 г.
 *      Author: 17095
 */

#include "mzConfig.h"
#include "mmem.h"

mz_settings_t *mzCONFIG = 0;

static struct mmem configbuf;
static int conf_read();

int conf_init() {
	int ret = 0;

	ret = spi_flash_init();

	if (ret == 0) {
		//prepraring RAM
		if (mmem_alloc(&configbuf, MZ_CONFIGSIZE)) {
			mzCONFIG = (mz_settings_t *) MMEM_PTR(&configbuf);
		} else {
			ret = -1;
		}
	}

	//REading config from FLASH
	if (ret == 0)
		ret = conf_read();

	if (ret != 0) {
		halErrorHalt();
	}

	return ret;
}

static int conf_read() {
	int ret;
	//reading from flash
	ret = spi_flash_read(mzCONFIG, MZ_CONFIGADDR, MZ_CONFIGSIZE);

	if (ret == MZ_CONFIGSIZE)
		ret = 0;

	return ret;
}

int conf_save(char *data, int size) {
	int ret = 0;
	ret = spi_flash_write(data, MZ_CONFIGADDR, size);

	//updating config if succeeded
	if (ret == size)
		ret = conf_read();

	return ret;
}
