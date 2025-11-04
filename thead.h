//File contains constants used to setup the network design

/************************************************************************************/
/****************************** PORT PARAMETERS *************************************/
/************************************************************************************/

#define NPORTPKTS				1		/* Number of Packets Per Port at the Node	*/
#define BPORTPKTS				1

#define DIMENSION1				(4)			//power of 2 required to reach max_nodes
#define CONC					(1)
#define XNUMPERDIM				(4)
#define YNUMPERDIM				(4)
#define ZNUMPERDIM				(4)
#define MAX_ROUTERS				(XNUMPERDIM*YNUMPERDIM)
#define MAX_ROUTERS_3D		    (XNUMPERDIM*YNUMPERDIM*ZNUMPERDIM)
#define MAX_ROUTERS_HC			(1 << DIMENSION1)
#define MAX_CPU					(MAX_ROUTERS*CONC)
#define MAX_CPU_3D				(MAX_ROUTERS_3D*CONC)
#define MAX_CPU_HC				(MAX_ROUTERS_HC*CONC)
#define RADIX					(4+CONC)
#define RADIX_3D				(6+CONC)
#define RADIX_HC				DIMENSION1+1
#define RADIX_FB				(XNUMPERDIM-1 + YNUMPERDIM-1 + CONC)
/************************************************************************************/
/******************************* CONSTANTS USED *************************************/
/************************************************************************************/

#define INPUT_BUFFER			0
#define OUTPUT_BUFFER			1
#define LOOKAHEAD_DEMUX			2
#define REGULAR_DEMUX			3
#define VIRTUAL_ALLOC_MUX		4
#define SWITCH_ALLOC_MUX		5
#define INPUT_DEMUX				6

/************************************************************************************/
/************************* POWER CONSTANTS USED *************************************/
/************************************************************************************/

#define BUFFER_POWER			1.912
#define LINK_POWER				2.402
#define CONGESTION_POWER		0.0070

#define CROSSBAR_POWER			2.16

/************************************************************************************/
/****************************** SIMULATION PARAMETERS *******************************/
/************************************************************************************/

#define LAMBDA				0.5		/* Rate of Traffic Arrival			*/
#define INTERARRIVAL		1.0		/* Inter-arrival Mean Time			*/
#define NPKTS				10000  	/* Number of Packets Simulated		*/
#define NCYCLES				10000.0	/* Number of Cycles Simulated		*/
#define VC					4		/* Virtual Channels					*/

#define INJECTTIME			1.0		/* Reduced Cycle Time				*/
#define BERNOULLI_RATE		0.2		/* Default Bernoulli Rate			*/
#define POISSON_RATE		LAMBDA	/* Rate of arrival of packets		*/
#define EXPONENTIAL_RATE	1/POISSION_RATE
									/* Inter-arrival Rate 				*/
#define PHITSZ				32		/* Number of bits per phit			*/
#define FLITSZ				4		/* Number of phits in a Flit		*/
#define IBUFSZ				16		/* Buffer Size in phits				*/
#define OBUFSZ				5
#define PKTSZINBITS			PHITSZ*FLITSZ
									/* Packet Size in Bits				*/
#define CREDITS				4
#define ROUTER_FREQ			0.1		/* in Ghz (100 MHz)					*/
#define ROUTER_LATENCY		1/ROUTER_FREQ
									/* Latency to move a Flit  			*/
#define CYCLETIME			0.125	/* Cycle Latency in nanoseconds		*/
#define ARB_LATENCY			1		/* Arbitration Latency (Mux)		*/
#define MUX_LATENCY			1		/* Switching Latency (Mux)			*/
#define DEMUX_LATENCY		1		/* Routing Latency (Demux)			*/
#define PORT_LATENCY		2.0		/* Receiving Port Latency			*/
#define DEMUXDELAY			4.0
#define MUXDELAY			4.0
#define ROUTE_DELAY			4.0
#define VIRTUAL_ALLOC_DELAY	4.0
#define SWITCH_ALLOC_DELAY	4.0
#define CREDIT_LOOP_DELAY	4.0
#define PKTSZ				4

#define BUFSZTOTAL			(IBUFSZ*VC)-OBUFSZ
/*#define BUFSZTOTAL			64*/

#define LINEMAX 			256

/*************************************************************************************/
/****************************** TRAFFIC PATTERNS *************************************/
/*************************************************************************************/

#define UNIDIRECTION_PING		0
#define BIDIRECTION_PING		1
#define UNIFORM_TRAFFIC			2
#define BIT_REVERSAL			3
#define BUTTERFLY				4
#define COMPLEMENT				5
#define	MATRIX_TRANSPOSE		6
#define PERFECT_SHUFFLE			7
#define NEIGHBOR				8
#define	HOTSPOT					9

/*#define NETWORK_CAPACITY_MESH	(PHITSZ/CYCLETIME)*2*/
#define NETWORK_CAPACITY_MESH	(PHITSZ/CYCLETIME)
#define NETWORK_CAPACITY_TORI	(PHITSZ/CYCLETIME)
#define NETWORK_LOAD			0.1

/************************************************************************************/
/********************************** ROUTING FUNCTIONS *******************************/
/************************************************************************************/

/*#define CURRENT_ROUTE			1*/

#define DOR_YX					0
#define DOR_XY					1
#define ROMM					2
#define	VALIANT					3
#define O1TURN					4
#define WF_MIN_AD				5
#define VIR_NET					6
#define VIR_NET0				7
#define VIR_NET1				8

#define BUFFER_DEPTH			0
#define FREE_VC					1

/************************************************************************************/
/********************************** SWITCHING FUNCTIONS *****************************/
/************************************************************************************/

#define CKTPROBE				10
#define CKTACK					11
#define CKTNACK					12
#define CKTDATA					13
#define WORMDATA				14

/************************************************************************************/
/********************************** TRAFFIC FUNCTIONS *******************************/
/************************************************************************************/

int UnidirectionalPing(int);
int BidirectionalPing(int);
int UniformTraffic(int);
int UniformTraffic(int);
int BitReversal(int);
int Butterfly(int);
int Complement(int);
int MatrixTranspose(int);
int PerfectShuffle(int);
int Neighbor(int);
int Tornado(int);
int Uniform(int);

/************************************************************************************/
/********************************** ROUTING FUNCTIONS *******************************/
/************************************************************************************/

int valiant_route( int, int );
int romm_route( int, int);

char* ToBin(int);
char* Negate(char*, int);
int ToInt(char*);
int LSB(int);

extern int VIRTUAL;
extern int CURRENT_ROUTE;