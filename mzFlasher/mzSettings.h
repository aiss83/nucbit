
#pragma pack(push,1)
#define RIMEADDR_SIZE 8
/**
      * Serial port Baudrate
      */
    typedef enum : char
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

	unsigned int mz_speed_val[] = {
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

typedef enum: char{
   MZ_SLAVE,	//0
   MZ_MASTER	//1
} mz_mode_e;


typedef union {
  unsigned char u8[RIMEADDR_SIZE];
} rimeaddr_t;

typedef struct{
	 rimeaddr_t rnode;
	 char num;
	 unsigned char mnodes;//first item of array actually
} mz_node_t;

#define CIB_SIZE_B 0x400

#define MAXTABLE ((CIB_SIZE_B - sizeof(mz_mode_e)- sizeof(char)- sizeof(mz_line_speed_t))/ (sizeof(mz_node_t)))
#define NODE_SHIFT(x) (sizeof(mz_node_t)+ x->num - 1)

typedef struct{
	mz_mode_e mode;
	mz_line_speed_t speed;
	char recNum;///< Number of node's redords, can't be more than 255 anyway
	mz_node_t nodeList;//first item of array actually
} mz_settings_t;

#pragma pack(pop)