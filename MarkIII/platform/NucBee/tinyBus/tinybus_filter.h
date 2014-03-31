/*
 * tinybus_filter.h
 *
 *  Created on: 20 дек. 2013 г.
 *      Author: 17095
 */

#ifndef TINYBUS_FILTER_H_
#define TINYBUS_FILTER_H_

typedef struct node_list_struct {
  struct node_list_struct *next;
  char node;
} nlist_t;

int tinybus_filter_init();
int tinybus_add_node( char node);
int tinybus_del_node(char node);
void tinybus_setbypass(char set);

#endif /* TINYBUS_FILTER_H_ */
