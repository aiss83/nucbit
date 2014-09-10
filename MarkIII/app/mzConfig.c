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

int conf_init() {
	int ret = 0;
	//prepraring RAM
	if (mmem_alloc(&configbuf, MZ_CONFIGSIZE)) {
		mzCONFIG = (mz_settings_t *)MMEM_PTR(&configbuf);
	} else {
		ret = -1;
	}

	//reading from flash
	if (ret == 0 ){
		ret = spi_flash_read(mzCONFIG, MZ_CONFIGADDR, MZ_CONFIGSIZE);
	}

	return ret;
}


int conf_save(char *data, int size){
	int ret = 0;

	return ret;
}
