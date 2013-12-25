/*
 * modZig_controller.c
 *
 *  Created on: 20 дек. 2013 г.
 *      Author: 17095
 */
#include "contiki.h"
#include "net/rime.h"
#include "net/rime/mesh.h"
#include "dev/stm32w-radio.h"
//#include "dev/button-sensor.h"
//#include "sys/etimer.h"
#include "modzig.h"

#include <stdio.h>
#include <string.h>

static struct mesh_conn mesh;
static char master_mode;
//static rimeaddr_t master_addr;
static rimeaddr_t target_addr;
static int out_overrun = 0;
static int in_overrun = 0;
static unsigned char localbuf[256];
static int tsize;
static char packet_sending = 0;
extern void mesh_inc_tout();
/*
 *
 * nodes table - need to develop way to store them apart main program,
 * 	ex. in coffeFS or any other way
 *
 */
static mz_node_t mz_nodes[] = { {
//rime slave node
		.rnode = { { 186, 1 } },
		//MB slave node
		.mnode = { .node = 42 } }

};

/*---------------------------------------------------------------------------*/
PROCESS(mesh_controller_process, "Mesh controller");
//AUTOSTART_PROCESSES(&mesh_controller_process);
/*---------------------------------------------------------------------------*/

/*
 *
 * MESH callbacks
 *
 */
static void sent(struct mesh_conn *c) {
	//noting to do by now, may be some timeouts later
	// printf("packet sent\n");
	packet_sending = 0;
}

static void timedout(struct mesh_conn *c) {
	//oopsie
	//printf("packet timedout\n");
	packet_sending = 0;
	mesh_inc_tout();
}

static void recv(struct mesh_conn *c, const rimeaddr_t *from, uint8_t hops) {
	/*
	 printf("Data received from %d.%d: %.*s (%d)\n",
	 from->u8[0], from->u8[1],
	 packetbuf_datalen(), (char *)packetbuf_dataptr(), packetbuf_datalen());

	 packetbuf_copyfrom(MESSAGE, strlen(MESSAGE));
	 mesh_send(&mesh, from);*/
	//saving incoming address
	if (!master_mode) {
		memcpy(&target_addr, from, sizeof(rimeaddr_t));
	}
	INTERRUPTS_OFF()
	;
	if (!tinybus_send((unsigned char *) packetbuf_dataptr(),
			packetbuf_datalen())) {
		in_overrun++;
	}
	INTERRUPTS_ON()
	;
}

const static struct mesh_callbacks callbacks = { recv, sent, timedout };
/*---------------------------------------------------------------------------*/

/*
 *
 * Tinybus callbacks
 *
 */

static int got_packet(unsigned char *ptr, unsigned int size) {
	int ret = 0;
	//preparing packet
	if (!packet_sending) {
		packet_sending = 1;

		memcpy(localbuf, ptr, size);
		tsize = size;
		if (master_mode) {
			//looking for slave
#ifndef DEMO
			int i;
			for (i = 0; i < (sizeof(mz_nodes) / sizeof(mz_node_t)); i++) {
				//tinybus_add_node(&mz_nodes[i].mnode, mz_nodes[i].mnode.node);
				//XXX:bad node determination, need redo

				if (mz_nodes[i].mnode.node == ptr[0]) {
					memcpy(&target_addr, &mz_nodes[i].rnode,
							sizeof(rimeaddr_t));
					//sadly we can't call mesh_send here, as we run in critical section
					//so we just marking dispatcher process polling and hoping for good
					process_poll(&mesh_controller_process);
					break;
				} else {
					packet_sending = 0;					//canceling send
				}

			}
#else
			target_addr.u8[0] = 186;
			target_addr.u8[1] = 1;
			//sadly we can't call mesh_send here, as we run in critical section
			//so we just marking dispatcher process polling and hoping for good
			process_poll(&mesh_controller_process);
#endif
		} else {
			//relaying to master
			//sadly we can't call mesh_send here, as we run in critical section
			//so we just marking dispatcher process polling and hoping for good
			process_poll(&mesh_controller_process);
		}
	} else {
		out_overrun++;
	}

	return ret;
}

void modZig_controller_init(mz_mode_e mode) {
	master_mode = 0;
	/*
	 * Patching radio power mode
	 */
	short pmode = 0
			| (0 << 1)  //ext. transmitter
			| (1 << 0); //boost mode
	ST_RadioSetPowerMode(pmode);

	char power = 3;
	ST_RadioSetPower(power);

	mesh_open(&mesh, 132, &callbacks);
	tinybus_set_recieve_cb(got_packet);

	tinybus_init(115200);
	process_start(&mesh_controller_process, NULL);

	if (mode == MZ_MASTER) {
		master_mode = 1;
		//setting up nodes filters
		int i;
		for (i = 0; i < (sizeof(mz_nodes) / sizeof(mz_node_t)); i++) {
			tinybus_add_node(&mz_nodes[i].mnode, mz_nodes[i].mnode.node);
		}

	} else if (mode == MZ_SLAVE) {
		//nothing to do at this point
		tinybus_setbypass(TRUE);		//disabling tinybus filters
	} else {
		while (1) {
			//WTF????
		}
	}

}
/*---------------------------------------------------------------------------*/
//PROCESS_THREAD(mesh_controller_process, ev, data)
///  PROCESS_EXITHANDLER(mesh_close(&mesh);)
//  PROCESS_BEGIN();
PROCESS_THREAD(mesh_controller_process, ev, data) {
	PROCESS_EXITHANDLER(mesh_close(&mesh)
	;
	)
	PROCESS_BEGIN()
		;

		while (1) {

			PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
			packetbuf_copyfrom(localbuf, tsize);
			mesh_send(&mesh, &target_addr);

		}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/

