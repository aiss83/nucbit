/*
 * modZig_controller.c
 *
 *  Created on: 20 дек. 2013 г.
 *      Author: 17095
 */
#include "contiki.h"
#include "mmem.h"
#include "net/rime.h"
#include "net/rime/mesh.h"
#include "dev/leds.h"
#include "dev/stm32w-radio.h"
//#include "dev/button-sensor.h"
//#include "sys/etimer.h"
#include "modzig.h"
#include "mzConfig.h"
#include <stdio.h>
#include <string.h>

static struct mesh_conn mesh;
static mz_mode_e mode;
//static rimeaddr_t master_addr;
static rimeaddr_t target_addr;
static int out_overrun = 0;
static int in_overrun = 0;

static struct mmem databuf;
static unsigned char *localbuf;
#define DYNMEMSIZE (PAYLOADSIZE + (PACKETBUF_SIZE + PACKETBUF_HDR_SIZE) / 2 + 1)


static int tsize;
static char packet_sending = 0;
extern void mesh_inc_tout();

unsigned int mz_speed_val[] = { 100, ///< 110 bits/sec
		300,    ///< 300 bits/sec
		600,    ///< 600 bits/sec
		1200,   ///< 1200 bits/sec
		2400,   ///< 2400 bits/sec
		4800,   ///< 4800 bits/sec
		9600,   ///<9600 bits/sec
		14400,  ///< 14400 bits/sec
		19200,  ///< 19200 bits/sec (default)
		38400,  ///< 38400 bits/sec
		56000,  ///< 56000 bits/sec
		57600,  ///< 57600 bits/sec
		115200, ///< 115200 bits/sec
		128000, ///< 128000 bits/sec
		256000, ///< 256000 bits/sec
		460800, 921600, ///< 921600 bits/sec
		1250000 ///< 1250000 bits/sec
		};

/*
 *
 * nodes table - need to develop way to store them apart main program,
 * 	ex. in coffeFS or any other way
 *
 */

//static mz_node_t *mz_nodes = &(NODECONF->nodeList);
/*{ {
 //rime slave node
 .rnode = { { 186, 1 } },
 //MB slave node
 .mnode = { .node = 42 } }

 };
 */
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
	if (mode == MZ_SLAVE) {
		//memcpy(&target_addr, from, sizeof(rimeaddr_t));
		rimeaddr_copy(&target_addr, from);
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
		if (mode == MZ_MASTER) {
			unsigned char adr = ptr[0];
			//looking for slave
			//tinybus_add_node(&mz_nodes[i].mnode, mz_nodes[i].mnode.node);
			//XXX:bad node determination, need redo
			/*
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
			 */
			if (adr) {
				int i, j;
				char found = 0;
				mz_node_t *n = mzCONFIG->nodeList;
				for (i = 0; i < (mzCONFIG->recNum) && !found; i++) {
					for (j = 0; j < n->num && !found; j++) {
						if (n->mnodes[j] == adr) {
							//memcpy(&target_addr, &n->rnode, sizeof(rimeaddr_t));
							rimeaddr_copy(&target_addr, &n->rnode);

							found = 1;
						}
					}
					//verify shift calculation
					n = (mz_node_t *) (((char*) n) + NODE_SHIFT(n));
					//n += (sizeof(mz_node_t)+ (n->num - 1));
				}

				if (!found) {
					packet_sending = 0;					//canceling send
				}
			} else {					//0 mean broadcast
										//but broadcast not inmplemented
										//memcpy(&target_addr, &rimeaddr_bcast, sizeof(rimeaddr_t));
			}

		} else {
			//relaying to master
			//sadly we can't call mesh_send here, as we run in critical section
			//so we just marking dispatcher process polling and hoping for good

		}

		if (packet_sending)
			process_poll(&mesh_controller_process);
	} else {
		out_overrun++;
	//	leds_toggle(LEDS_CONF_BLUE);
	}

	return ret;
}

void modZig_controller_init() {
	//checking for valid config

	if (mmem_alloc(&databuf, DYNMEMSIZE)) {

		localbuf = (unsigned char*)MMEM_PTR(&databuf);
		packetbuf_setptr(localbuf+PAYLOADSIZE);

		if (mzCONFIG->mode != MZ_MASTER && mzCONFIG->mode != MZ_SLAVE) {
			halErrorHalt();
		}

		mode = mzCONFIG->mode;
		/*
		 * Patching radio power mode
		 */
		short pmode = 0 | (0 << 1)  //ext. transmitter
				| (1 << 0); //boost mode
		ST_RadioSetPowerMode(pmode);

		char power = 3;
		ST_RadioSetPower(power);

		mesh_open(&mesh, 123, &callbacks);
		tinybus_set_recieve_cb(got_packet);

		tinybus_init(mz_speed_val[mzCONFIG->speed]);
		process_start(&mesh_controller_process, NULL);

		//moved filtering into got_gacket
		/*
		 if (mode == MZ_MASTER) {
		 //master_mode = 1;
		 //setting up nodes filters
		 int i,j;
		 mz_node_t *n = mzCONFIG->nodeList;
		 for (i = 0; i < (mzCONFIG->recNum); i++) {
		 for(j=0;j < n->num;j++){
		 tinybus_add_node(n->mnodes[j]);
		 }
		 //verify shift calculation
		 n += (sizeof(mz_node_t)+ (n->num - 1));
		 }

		 } else if (mode == MZ_SLAVE) {
		 //nothing to do at this point
		 tinybus_setbypass(TRUE);		//disabling tinybus filters
		 } else {
		 while (1) {
		 //WTF????
		 }
		 }
		 */

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

