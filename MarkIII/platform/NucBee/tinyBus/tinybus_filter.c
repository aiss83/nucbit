/*
 * packet_filter.c
 *
 *  Created on: 20 дек. 2013 г.
 *      Author: 17095
 */
#include <stdlib.h>
#include "contiki.h"
#include "mmem.h"
#include "tinybus_filter.h"
//#include "lib/list.h"

struct mmem data;

static char nodes[255];
static unsigned char num = 0;
static char bypass_mode = 0; //filter disabled

char tinybus_our_packet(unsigned char *ptr) {
	char ret = 0;
	unsigned char i;
	//char match = 0;
	nlist_t *cur = nodes;
	// 0 means broadcast address
	if (!bypass_mode || (ptr[0]==0)) {
		for (i=0; i< num && ret == 0; i++){
			if(nodes[i]==ptr[0]){
				ret = 1;
			}
		}
	} else {
		ret = 1;
	}
	return ret;
}

int tinybus_filter_init() {
	int ret = 0;

	return ret;
}

int tinybus_add_node(char node) {
	nodes[num] = node;
	num++;
	return 0;
}

int tinybus_del_node(char node) {
///XXX: if we gonna make nodelist truly dynamic - implement it
	int ret = 0;
	/*
	 nlist_t *cur = nodes;
	 nlist_t *prev = NULL;
	 do {
	 if (node == cur->node){
	 ret = 1;//positive match

	 if(prev){
	 prev->next = cur->next;
	 }else{
	 nodes = cur->next;
	 }
	 free(cur);
	 }
	 prev = cur;
	 }while (cur->next != NULL && ret == 0);
	 */
	return ret;
}

void tinybus_setbypass(char set) {
	bypass_mode = set;
}
