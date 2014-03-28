/*
 * flash_proto.h
 *
 *  Created on: 15 џэт. 2014 у.
 *      Author: 17095
 */

#ifndef FLASH_PROTO_H_
#define FLASH_PROTO_H_

#pragma pack(push, 1)
typedef enum : unsigned short{
	CMD_PING = 0,
	CMD_READ_CNF,
	CMD_WRITE_CNF,
	CMD_GET_MAC,
	//not comands, juss replys
	RPL_PONG= 0xff00,
	RPL_DATA_CNF ,
	RPL_DATA_STS,
	RPL_MAC,
	//
	RPL_UNKNOWN_CMD = 0xfffe,
	CMD_MAX = 0xffff
} pf_cmd_e;

typedef struct{
	pf_cmd_e cmd;
	unsigned char data[];
} proto_frame_t;

#pragma pack(pop)

#endif /* FLASH_PROTO_H_ */
