/*
 * mzConfig.h
 *
 *  Created on: 21 џэт. 2014 у.
 *      Author: 17095
 */

#ifndef MZCONFIG_H_
#define MZCONFIG_H_


#define MZ_CONFIGSIZE (MFB_PAGE_SIZE_B * 1)
#define MZ_CONFIGADDR (MFB_TOP - (MFB_PAGE_SIZE_B*2) - MZ_CONFIGSIZE+1)



/**
      * Serial port Baudrate
      */
    typedef enum
    {
        BaudUnknown = -1,     ///< Unknown
        Baud110     = 1,
        Baud300     ,
        Baud600     ,
        Baud1200    ,
        Baud2400    ,
        Baud4800    ,
        Baud9600    ,
        Baud14400   ,
        Baud19200   ,
        Baud38400   ,
        Baud56000   ,
        Baud57600   ,
        Baud115200  ,
        Baud128000  ,
        Baud256000  ,
		Baud460800  ,
        Baud921600  ,
        Baud1250000 ,
    } mz_line_speed_t;

extern unsigned int mz_speed_val[] ;/*= {
		 0,
		 100, ///< 110 bits/sec
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
		 460800,
		 921600, ///< 921600 bits/sec
		 1250000 ///< 1250000 bits/sec
	};
*/
typedef enum{
   MZ_SLAVE,	//0
   MZ_MASTER	//1
} mz_mode_e;

/*
typedef union {
  unsigned char u8[RIMEADDR_SIZE];
} rimeaddr_t;
*/
typedef struct{
	 rimeaddr_t rnode;
	 char num;
	 char mnodes[];
} mz_node_t;

#define NODE_SHIFT(x) (sizeof(mz_node_t)+ x->num)

#define MAXTABLE ((MZ_CONFIGSIZE- sizeof(mz_mode_e)- sizeof(char)- sizeof(mz_line_speed_t))/ (sizeof(mz_node_t)))

typedef struct{
	mz_mode_e mode;
	mz_line_speed_t speed;
	char recNum;///< Number of node's redords, can't be more than 255 anyway
	mz_node_t nodeList[];
} mz_settings_t;



#define mzCONFIG ((mz_settings_t*) MZ_CONFIGADDR )

#endif /* MZCONFIG_H_ */
