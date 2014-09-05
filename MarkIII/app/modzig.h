/*
 * modzig.h
 *
 *  Created on: 23 дек. 2013 г.
 *      Author: 17095
 */

#ifndef MODZIG_H_
#define MODZIG_H_

#include "hal/micro/cortexm3/memmap.h"
#include "net/rime.h"
#include "tinyBus/tinybus.h"
/*
typedef enum{
	MZ_MASTER,
	MZ_SLAVE
} mz_mode_e;

typedef struct{
	 rimeaddr_t rnode;
	 nlist_t	mnode;
} mz_node_t;

#define MAXTABLE ((CIB_SIZE_B - sizeof(mz_mode_e)- sizeof(char))/ (sizeof(mz_node_t)))

typedef const struct{
	const mz_mode_e mode;
	const char recNum;///< Number of node's redords, can't be more than 255 anyway
	const mz_node_t nodeList;
} mz_settings_t;


//static const mz_settings_t *nodeConf = (mz_settings_t *)CONTIKI_MZ_SETTINGS_ADDR;

#define NODECONF ((const mz_settings_t *)CONTIKI_MZ_SETTINGS_ADDR)
*/
void modZig_controller_init();

#endif /* MODZIG_H_ */
