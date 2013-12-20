/*
 * modZig_controller.c
 *
 *  Created on: 20 дек. 2013 г.
 *      Author: 17095
 */
#include "contiki.h"
#include "net/rime.h"
#include "net/rime/mesh.h"

//#include "dev/button-sensor.h"
//#include "sys/etimer.h"

#include "tinyBus/tinybus.h"

#include <stdio.h>
#include <string.h>

#define MESSAGE "Hello"

static struct mesh_conn mesh;
static char master_mode;
rimeaddr_t master_addr;
/*---------------------------------------------------------------------------*/
//PROCESS(mesh_controller_process, "Mesh controller");
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
}

static void timedout(struct mesh_conn *c) {
	//oopsie
	//printf("packet timedout\n");
}

static void recv(struct mesh_conn *c, const rimeaddr_t *from, uint8_t hops) {
	/*
	 printf("Data received from %d.%d: %.*s (%d)\n",
	 from->u8[0], from->u8[1],
	 packetbuf_datalen(), (char *)packetbuf_dataptr(), packetbuf_datalen());

	 packetbuf_copyfrom(MESSAGE, strlen(MESSAGE));
	 mesh_send(&mesh, from);*/
	//saving incoming address
	if(!master_mode){
		memcpy(&master_addr, from, sizeof(rimeaddr_t));
	}

	tinybus_send((char *)packetbuf_dataptr(),packetbuf_datalen());
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
		if (master_mode){
			//looking for salve

		}else{
			//relaying to master
			packetbuf_copyfrom(ptr, size);
			mesh_send(&mesh, &master_addr);
		}
	return ret;
}

/*---------------------------------------------------------------------------*/
//PROCESS_THREAD(mesh_controller_process, ev, data)
///  PROCESS_EXITHANDLER(mesh_close(&mesh);)
//  PROCESS_BEGIN();
void modZig_controller_init(char mode) {


	mesh_open(&mesh, 132, &callbacks);
//	rimeaddr_t addr;
//
//	packetbuf_copyfrom(MESSAGE, strlen(MESSAGE));
//	addr.u8[0] = 186;
//	addr.u8[1] = 170;
//	mesh_send(&mesh, &addr);
	tinybus_set_recieve_cb(got_packet);

	tinybus_init(115200);
}

//PROCESS_THREAD(mesh_controller_process, ev, data)
//{
///  PROCESS_EXITHANDLER(mesh_close(&mesh);)
//  PROCESS_BEGIN();

//  PROCESS_END();
//}
/*---------------------------------------------------------------------------*/
