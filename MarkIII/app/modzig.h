/*
 * modzig.h
 *
 *  Created on: 23 дек. 2013 г.
 *      Author: 17095
 */

#ifndef MODZIG_H_
#define MODZIG_H_

#include "net/rime.h"
#include "tinyBus/tinybus.h"
typedef enum{
	MZ_MASTER,
	MZ_SLAVE
} mz_mode_e;

typedef struct{
	 rimeaddr_t rnode;
	 nlist_t	mnode;
} mz_node_t;

void modZig_controller_init(mz_mode_e mode);

#endif /* MODZIG_H_ */
