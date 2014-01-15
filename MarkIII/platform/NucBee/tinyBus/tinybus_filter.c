/*
 * packet_filter.c
 *
 *  Created on: 20 дек. 2013 г.
 *      Author: 17095
 */
#include <stdlib.h>
#include "contiki.h"
#include "tinybus_filter.h"
//#include "lib/list.h"

static nlist_t *nodes;
static nlist_t null_node;
static char bypass_mode = 0; //filter disabled

char tinybus_our_packet(unsigned char *ptr) {
	char ret = 0;
	//char match = 0;
	nlist_t *cur = nodes;
	if (!bypass_mode) {
		do {
			if (ptr[0] == cur->node) {
				ret = 1; //positive match
			}
		} while (cur->next != NULL && ret == 0);
	} else {
		ret = 1;
	}
	return ret;
}

int tinybus_filter_init() {
	int ret = 0;
	//list_init(nodes);
	nodes = &null_node;
	nodes->next = NULL;
	nodes->node = 0; //we anyway should process broadcasts
	return ret;
}

int tinybus_add_node(nlist_t *new, char node) {
	//nlist_t *new = malloc(sizeof(nlist_t));

	new->node = node;
	new->next = nodes;
	nodes = new;

	return new != NULL;
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
