// Modified from Rice Parallel Testbed Simulator
// Torus connections, mesh routing
// Avinash Karanth, Ohio University

#include "../../sim.h"
#include <stdlib.h>
#include "../../thead.h"// Topology Parameters
#include <time.h>		// Track wall time
#include <sys/stat.h>	// For folder generating
#include <sys/types.h>	// For folder generating
#include <errno.h>		// For folder generating

/* Measurement Structure */
typedef struct MEASURE MEASURE;
typedef struct MEASURE *measureptr;
struct MEASURE {
	int send;
	int recv;
	double latency;
};
measureptr measure[MAX_CPU_3D];

/* Local Variables */
int pktsz = 0;
int Traffic = 0;
double bernoulli_rate = 0.0;
double poisson_rate = 0.0;
double ncycles = 0.0;
double injecttime[MAX_CPU_3D];
int count = 0;
int recvcount = 0;
double generate_rate = 0.0;
int hop = 0;
double avg_power = 0.0;
double throughput = 0.0;
double inter = 1.0;
int npkts = NPKTS;
int CURRENT_ROUTE;
int VIRTUAL;

/* Global variables to increment	*/
int bufnum = 0;
int iportnum = 0;
int oportnum = 0;
int demuxnum = 0;
int muxnum = 0;

// Output file Variables (Latency)
double avglat = 0.0; // used in UserRecieve()
double curlat = 0.0;
double maxlat = 0.0;
double minlat = 1000000.0;
FILE* fl; // File output for latency
int hoptype[2]; // used in Routing to count type of each link traversed

/* Structure to maintain internal switches */
typedef struct SWITCHSET SWITCHSET;
typedef struct SWITCHSET *switchptr;
struct SWITCHSET{
	int xcord;							/* x co-ordinate of the switch within group	*/
	int ycord;							/* y co-ordinate of the group				*/
	int zcord;
	int switchid;						/* ID of the switch							*/
	BUFFER *input_buffer[(VC)*(RADIX)];	/* Input Buffers of the switch (VC)			*/
	BUFFER *output_buffer[RADIX];		/* Output Buffer of the switch				*/
	MUX *input_mux[RADIX];				/* Input Multiplexer (Route_MUX)			*/
	MUX *output_mux[RADIX];				/* Output Multiplexer (Regular_MUX)			*/
	DEMUX *input_demux[RADIX];			/* Input Demultiplexer (Look-Ahead Router)	*/
	DEMUX *output_demux[RADIX];			/* Input Demultiplexer (Regular Router)		*/
	IPORT *iport[CONC];					/* Input port to the node					*/
	OPORT *oport[CONC];					/* Output port to the node					*/
};
switchptr switches[MAX_ROUTERS_3D];

/* Local Functions for Torus Network */
int FindXcord(int );					/* Find the x co-ordinate					*/
int FindYcord(int );					/* Find the y co-ordinate					*/
int FindZcord(int );
void intraconnections(int );		/* Create the switches						*/
int GetSwitchId(int , int , int );			/* Get the switch ID from co-ordniates		*/
int power(int,int);

//*****************minez**************** Routing Function *******************************//
// Hypercube uses e-cube routing
int router(src,dest,id)
int *src;
int *dest;
int id;
{
	int demuxret, skipcount, k, conc_pair, i;
	int current_router, cur_xoffset, cur_yoffset, cur_zoffset;
	int dest_router, dest_xoffset, dest_yoffset, dest_zoffset;
	int src_router, src_xoffset, src_yoffset, src_zoffset;
	int xidentity, diff, pos_skip, neg_skip;

	current_router = id/((2)*(RADIX));
	cur_xoffset = FindXcord(current_router);
	cur_yoffset = FindYcord(current_router);
	cur_zoffset = FindZcord(current_router);

	dest_router = *dest/CONC;
	dest_xoffset = FindXcord(dest_router);
	dest_yoffset = FindYcord(dest_router);
	dest_zoffset = FindZcord(dest_router);

	src_router = *src/CONC;
	src_xoffset = FindXcord(src_router);
	src_yoffset = FindYcord(src_router);
	src_zoffset = FindZcord(dest_router);

	demuxret = 0;

	// Try to get e-cube for 3D hypercube... need cur_zoffset
	// DOR: ROUTES AS A MESH, For Torus need to use the wrap around links
	if(current_router == dest_router) // Rout to the OPORT
	{
		demuxret = 4 + *dest%CONC;
	}
	else if(cur_xoffset != dest_xoffset) // ROUTE x
	{
		if(cur_xoffset < dest_xoffset)
			demuxret = 0;
		else if(cur_xoffset > dest_xoffset)
			demuxret = 1;
		else
			YS__errmsg("Routing: Should not get here x\n");

	}
	else if(cur_yoffset != dest_yoffset) // ROUTE y
	{
		if(cur_yoffset < dest_yoffset)
			demuxret = 2;
		else if(cur_yoffset > dest_yoffset)
			demuxret = 3;
		else
			YS__errmsg("Routing: Should not get here y\n");
	}
	else if(cur_zoffset != dest_zoffset)
	{
		if(cur_zoffset < dest_zoffset)
			demuxret = 4;
		else if(cur_zoffset > dest_zoffset)
			demuxret = 5;
		else
			YS__errmsg("Routing: Should not get here z\n");
	}
	else
	{
		YS__errmsg("Routing: Should not get here\n");
	}

	//printf("Routing %d->%d Cur:%d Port:%d\n", *src, *dest, cur, demuxret );

	// Keep track of Router and Link utiliztion
	if(demuxret < 6)	// +x, -x, +y, -y, +z, -z
		hoptype[1]++;
	else 				// OPORT
		hoptype[0]++;

	return demuxret;
}

//************************************ Send Process *************************************//
void UserEventS()
{
	IPORT *inport;
	PACKET *pkt;
	PKTDATA *pktdata;
	int index, dest;
	long seqno = 0;
	double senddelay = 0.0;
	int xsrc, ysrc, xdest, ydest, xoffset, yoffset;
	double diff = 0.0;
	int retval,i,pktsz1,retval1, tempcpu;

	index = ActivityArgSize(ME);
	inport = (IPORT*)ActivityGetArg(ME);

	// Cases to manage packet transmission, state is set in EventRescedTime(time, state)
	switch (EventGetState()) {

		case 0: /* To send or not to send */
				if( ncycles < GetSimTime() )
				{
					EventReschedTime(0.0, 3);
					return;
				}

				retval = RandBernoulli(bernoulli_rate);
				//retval = 1;
				if( retval > 0 )
					EventReschedTime(0.0, 1);
				else
					EventReschedTime(0.0, 2);
				return;

		case 1: // Send a packet
				if( IPortSpace(inport) != NPORTPKTS ) {
					EventReschedSema(IPortSemaphore(inport), 1);
					return;
				}

				// Choose a Destination Node based on the traffec pattern
				switch(Traffic) {
					case 0: //Random
							do {
								dest = RandUniformInt(0, MAX_CPU_3D - 1 );
							}while(dest == index);


							break;
					case 1: // Non-Random/Hotspot
							retval1 = RandBernoulli(0.25);
							if( retval1 > 0 ) {
								do {
									dest = RandUniformInt(0, MAX_CPU_3D - 1 );
								}while(dest == index);
							}
							else {
								do {
									dest = RandUniformInt(0, MAX_CPU_3D/4);
								}while(dest == index);
							}
							break;
					case 2:
							dest = BitReversal(index);
							break;
					case 3:
							dest = Butterfly(index);
							break;
					case 4:
							dest = Complement(index);
							break;
					case 5:
							dest = MatrixTranspose(index);
							break;
					case 6:
							dest = PerfectShuffle(index);
							break;
					case 7:
							dest = Neighbor(index);
							break;
					case 8:
							dest = Tornado(index);
							break;
					default:
							YS__errmsg("Traffic Type Undefined\n");
							break;
				}

				seqno = index + MAX_CPU_3D * (NPKTS - npkts);
				measure[index]->send = measure[index]->send + 1;
				count++;
				npkts--;

				// Make the packet and send into the IPORT
				for( i = 0; i < pktsz/FLITSZ; i++ )
				{
					pkt = NewPacket(seqno,NULL,FLITSZ);
					pktdata = PacketGetData(pkt);
					pktdata->createtime = injecttime[index];
					pktdata->pkttype = i;
					pktdata->packetsize = pktsz;
					pktdata->intercpu = tempcpu;
					senddelay = PacketSend(pkt, inport, index, dest, i, pktsz, tempcpu);
				}

		case 2: // Decide when this event will run next and reschedule it
				injecttime[index] = injecttime[index] + inter;
				if( injecttime[index] > GetSimTime() )
				{
					diff = injecttime[index] - GetSimTime();
					EventReschedTime(diff, 0); // Run again 'diff' cycles from now
					return;
				}
				else
				{
					EventReschedTime(0.0, 0); //Run again this cycle
					return;
				}

		case 3: // Terminate this event
				EventSetDelFlag();
				return;
	}
}

//******************************* Receive Process ********************************//
void UserEventR()
{
	OPORT *outport;
	PACKET *pkt;
	PKTDATA *pktdata;
	int index,i,j;

	index = ActivityArgSize(ME);
	outport = (OPORT*)ActivityGetArg(ME);

	/* Receive packets */
	if (OPortPackets(outport)) {
		pkt = PacketReceive(outport);
		pktdata = PacketGetData(pkt);
		if(pktdata->destcpu != index ) {
			printf("Receiver %d\n", index);
			YS__errmsg("Incorrect destination received\n");
		}

		//printf("RECEIVING packet %d %d %d time %g\n", pktdata->srccpu, index, pktdata->seqno, GetSimTime());

		if( (pktdata->pkttype == (pktdata->packetsize - 1)/FLITSZ) ) {
			measure[pktdata->srccpu]->latency = measure[pktdata->srccpu]->latency + (GetSimTime() - pktdata->createtime);
			measure[pktdata->srccpu]->recv = measure[pktdata->srccpu]->recv + 1;
			recvcount++;
			curlat = (GetSimTime() - pktdata->createtime);
			avglat = avglat + curlat;

			// Stat update and print to file
			if(curlat > maxlat)
				maxlat = curlat;
			if(curlat < minlat)
				minlat = curlat;
			if( recvcount%100 == 0)
			{
				fprintf(fl,"%g\t%g\t%g\t%g\n", avglat/100.0, maxlat, minlat, GetSimTime());
				fflush(fl); // Makes the file update every time a line is written
				avglat = 0.0;
				maxlat = 0.0;
				minlat = 1000000.0;
				count = 0;
			}

			avg_power = pktdata->powers + avg_power;
			PacketFree(pkt);
		}
		else
			PacketFree(pkt);
	}

	EventReschedSema(OPortSemaphore(outport));

	return;
}

// Creating the network with parameters
void UserMain(argc, argv)
int argc;
char** argv;
{
//******************************** Variable Initialization ******************************//
	EVENT* event;
	int num_switch, iports, previous, next,i, j, partial_send, total, curswitch, next_switch;
	char namestr[31];
	int ax, ay, az, sx, sy, sz, var, k, m, skip_count, iter;
	int total_send = 0;
	int total_recv = 0;
	double total_latency = 0.0;
	double network_load, netcap, byte_node_cycle, partial_latency, network_loadA, bits_node_cycle;
	double util = 0.0;
	int avg_ibufs, avg_xbar, avg_lnks;
	int avg_rcs, avg_vcs, avg_swtchs;

	// Start clock for timing simulation
	clock_t begin, end;
	double time_spent;

	begin = clock();

	network_loadA = 0.0;

//******************************** Command Line Arguements ******************************//
	if (argc > 1)
		pktsz = atoi(argv[1]);

	if (argc > 2)
		network_load = ( (double)atoi(argv[2]))/10.0;

	if (argc > 3)
		network_loadA = ( (double)atoi(argv[3]))/100.0;

	if (argc > 4)
		ncycles = (double)atoi(argv[4]);

	if (argc > 5)
		Traffic = atoi(argv[5]);

	network_load = network_load + network_loadA;

	if( pktsz <= 0 )
		pktsz = PKTSZ;

	if( ( network_load < 0 ) || ( network_load > 1 ) )
		network_load = NETWORK_LOAD;

	if( ncycles <= 0 )
		ncycles = NCYCLES;

	netcap = ((NETWORK_CAPACITY_MESH)*network_load);
  	byte_node_cycle = (CYCLETIME*netcap)/8.0;
  	generate_rate = (PHITSZ*(double)(pktsz))/(8.0*byte_node_cycle);

  	if( generate_rate > 1.0 )
  	{
  		bernoulli_rate = 1.0/generate_rate;
  		poisson_rate = 0.0;
  	}
  	else
  	{
  		poisson_rate = 1.0/generate_rate;
  		bernoulli_rate = 0.0;
  	}

//******************************** Print Parameters to Terminal ******************************//
	  printf("****************BEGIN SIMULATION***************\n");
      printf("MAXIMUM SIZE = %d\n", MAX_CPU_3D);
      printf("Packet Size = %d\n", pktsz);
      printf("Network Load = %g\n", network_load);
      printf("Bernoulli Rate = %g\n", bernoulli_rate);
      printf("Cycles to be simulated = %g\n", ncycles);
	  printf("\n***********HYPERCUBE***********\n");
	  printf("Maximum number of nodes %d\n", MAX_CPU_3D);
	  printf("Maximum number of Routers %d\n", MAX_ROUTERS_3D);
	  printf("Radix of the network %d\n", RADIX);

    switch(Traffic)
    {
		case 0:
				printf("Uniform Traffic Distribution!!\n");
				break;
		case 1:
				printf("Non-Uniform Traffic Distribution\n");
				break;
		case 2:
				printf("BitReversal\n");
				break;
		case 3:
				printf("Butterfly\n");
				break;
		case 4:
				printf("Complement\n");
				break;
		case 5:
				printf("Matrix Transpose\n");
				break;
		case 6:
				printf("Perfect Shuffle\n");
				break;
		case 7:
				printf("Neigbor\n");
				break;
		case 8:
				printf("Worst Case Traffic: Tornado\n");
				break;
		default:
				YS__errmsg("Should not get here");
	}

//******************************** Initialize Random Variables ******************************//
  	printf("Initializing random variables...\n");
	YacRandomInit();
  	SetSysRand(YacRandom);

//********************************** Malloc Measure Struct **********************************//
  	printf("Malloc Measure Struct...\n");
	for( i = 0; i < MAX_CPU_3D; i++ )
  	{
		measure[i] = malloc(sizeof(MEASURE));

		measure[i]->send = 0;
		measure[i]->recv = 0;
		measure[i]->latency = 0.0;
  	}

//********************************** Network Connections **********************************//
	printf("Building routers...\n");
	for( i = 0; i < MAX_ROUTERS_3D; i++ ) // Build the Routers
	{
		intraconnections(i);
		injecttime[i] = 0.0;
	}

	// Interconnect the routers
	printf("Interconnecting the routers...\n");
	for( i = 0; i < MAX_ROUTERS_3D; i++ )
	{
		// This is a HYPERCUBE Topology, Doesn't include wrap around links
		// Do the connections in +x and -x directions
		// Do the connections in +y and -y directions
		// Do for each switch, mux to buf connections
		// Directions: 0 = +x
		// Directions: 1 = -x
		// Directions: 2 = +y
		// Directions: 3 = -y
		// Directions: 4 = +z
		// Directions: 5 = -z

		printf("Do +x...\n");
		k = 0;
		// +x dimension
		ax = GetSwitchId(((switches[i]->xcord + 1)%(XNUMPERDIM)), switches[i]->ycord, switches[i]->zcord);
		NetworkConnect(switches[i]->output_buffer[k], switches[ax]->input_demux[k], 0, 0);
		DemuxCreditBuffer(switches[ax]->input_demux[k], switches[i]->output_buffer[k]);
		k++;
		printf("Do -x...\n");
		// -x dimension
		if( (switches[i]->xcord - 1) < 0 )
			var = XNUMPERDIM - 1;
		else
			var = switches[i]->xcord - 1;
		sx = GetSwitchId(var, switches[i]->ycord, switches[i]->zcord);
		NetworkConnect(switches[i]->output_buffer[k], switches[sx]->input_demux[k], 0, 0);
		DemuxCreditBuffer(switches[sx]->input_demux[k], switches[i]->output_buffer[k]);
		k++;
		printf("Do +y...\n");
		// +y dimension
		ay = GetSwitchId(switches[i]->xcord, ((switches[i]->ycord + 1)%(YNUMPERDIM)), switches[i]->zcord );
		NetworkConnect(switches[i]->output_buffer[k], switches[ay]->input_demux[k], 0, 0);
		DemuxCreditBuffer(switches[ay]->input_demux[k], switches[i]->output_buffer[k]);
		k++;
		printf("Do -y...\n");
		// -y dimension
		if( (switches[i]->ycord - 1) < 0 )
			var = YNUMPERDIM - 1;
		else
			var = switches[i]->ycord - 1;
		sy = GetSwitchId(switches[i]->xcord, var, switches[i]->zcord);
		NetworkConnect(switches[i]->output_buffer[k], switches[sy]->input_demux[k], 0, 0);
		DemuxCreditBuffer(switches[sy]->input_demux[k], switches[i]->output_buffer[k]);
		k++;
		printf("Do +z...\n");
		// +z dimension
		az = GetSwitchId(switches[i]->xcord, switches[i]->ycord, ((switches[i]->zcord + 1)%(ZNUMPERDIM)));
		NetworkConnect(switches[i]->output_buffer[k], switches[az]->input_demux[k], 0, 0);
		DemuxCreditBuffer(switches[az]->input_demux[k], switches[i]->output_buffer[k]);
		k++;
		printf("Do -z...\n");
		// -z dimension
		if( (switches[i]->zcord - 1) < 0 )
			var = ZNUMPERDIM - 1;
		else
			var = switches[i]->zcord - 1;
		sz = GetSwitchId(switches[i]->xcord, switches[i]->ycord, var);
		NetworkConnect(switches[i]->output_buffer[k], switches[sz]->input_demux[k], 0, 0);
		DemuxCreditBuffer(switches[sz]->input_demux[k], switches[i]->output_buffer[k]);
		k++;
		printf("Done with that.....\n");
	}

//********************************** Send and Recieve Events **********************************//
	for( i = 0; i < MAX_ROUTERS_3D; i++)
	{
		for( j = 0; j < CONC; j++)
		{
			sprintf(namestr, "UserSend%d", i*CONC+j);
			event = NewEvent(namestr,UserEventS,0);					/* Create sender process 0       */
			ActivitySetArg(event,switches[i]->iport[j],i*CONC+j);	/* Pass process its id           */
  			ActivitySchedTime(event, 0.0, INDEPENDENT);				/* Schedule process              */
		}
		printf("recv events...\n");
	}
	printf("donzeo...\n");
	for( i = 0; i < MAX_ROUTERS_3D; i++)
	{
		for( j = 0; j < CONC; j++)
		{
			sprintf(namestr, "UserRecv%d", i*CONC+j);
			event = NewEvent(namestr,UserEventR,0);					/* Create sender process 0       */
			ActivitySetArg(event,switches[i]->oport[j],i*CONC+j);	/* Pass process its id           */
  			ActivitySchedTime(event, 0.0, INDEPENDENT);				/* Schedule process              */
		}
	}
	printf("donzeo2...\n");

//********************************** Latency File Output Initialization **********************************//
	char filename[75];
	int err;
  	sprintf(filename, "latency");
  	err = mkdir(filename, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  	if(err && errno!=EEXIST)
		YS__errmsg("Folder not made and does not exist\n");

	sprintf(filename, "latency/mesh_AvgLat_T%dL%dC%d.txt", Traffic, (int)(network_load*10), (int)ncycles);
	fl = fopen(filename, "w");
	if(!fl)
	{
		YS__errmsg("File did not open latency/mesh\n");
	}
	fprintf(fl,"Ave Lat\tMax Lat\tMin Lat\tCycle\n");
	printf("file stuff...\n");

//********************************** Program Start **********************************//
	NetworkCollectStats(NETTIME,NOHIST,0.0,0.0);
	NetworkCollectStats(BLKTIME,NOHIST,0.0,0.0);
	NetworkCollectStats(OPORTTIME,NOHIST,0.0,0.0);
	NetworkCollectStats(MOVETIME,NOHIST,0.0,0.0);
	NetworkCollectStats(LIFETIME,NOHIST,0.0,0.0);
	printf("net stuff...\n");

	DriverRun(0.0);  // Start the simulation, on return simulation is complete
	printf("driver stuff...\n");
//**************************** Latency Calculation ******************************//
	total_send = 0;
	total_recv = 0;
	total_latency = 0.0;
	partial_send = 0;
	partial_latency = 0.0;
	for( i = 0; i < MAX_CPU_3D; i++ )
	{
		total_send = total_send + measure[i]->send;
		total_recv = total_recv + measure[i]->recv;
		total_latency = total_latency + measure[i]->latency;
	}

//**************************** Print Stats to Terminal ******************************//
	printf("End Simulation %g\n", GetSimTime() );
	printf("******************************************************\n");
	printf("Sent Packets %d \n", total_send);
	printf("Received Packets %d \n", total_recv);

	throughput = (total_send*pktsz)/(ncycles*(double)(MAX_CPU_3D));

	printf("Achieved Throughput %g\n", throughput);
	printf("Average Latency %g\n", (total_latency/(double)total_send) );


	//************* Program wall time print *************//
	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("\n\nProgram took: %f seconds\n", time_spent);


//********************************* File Output *********************************//
	// First set up the folder if it does not exist
	sprintf(filename, "results");
  	err = mkdir(filename, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  	if(err && errno!=EEXIST)
		YS__errmsg("Folder not made and does not exist\n");

	// Open a readable results file for this simulation only
	FILE* fp;
	FILE* fp1;
	sprintf(filename, "results/mesh_out_T%dL%dC%d.txt", Traffic, (int)(network_load*10), (int)ncycles);
	fp = fopen(filename, "w");
	if(!fp)
	{
		YS__errmsg("File did not open results/mesh\n");
	}

	// Open the shared results file and append this result
	sprintf(filename, "results/mesh_excel.txt");
	fp1 = fopen(filename, "a");
	if(!fp1)
	{
		YS__errmsg("File did not open results/excel\n");
	}

	// Print to readable file
	fprintf(fp, "****************** Start *************************\n");
	fprintf(fp, "Max CPUs = %d\n", MAX_CPU_3D);
	fprintf(fp, "Radix = %d\n", RADIX);
	fprintf(fp, "CONC = %d\n\n", CONC);
	fprintf(fp, "Traffic = %d\n", Traffic);
	fprintf(fp, "Packet Size = %d\n", pktsz);
	fprintf(fp, "Network Load = %g\n", network_load);
	fprintf(fp, "Cycles to be simulated = %g\n", ncycles);
	fprintf(fp, "End Simulation %g\n\n", GetSimTime() );
	fprintf(fp, "Sent Packets %d\n", total_send);
	fprintf(fp, "Received Packets %d\n\n", total_recv);
	fprintf(fp, "Achieved Throughput %g\n", throughput);
	fprintf(fp, "Average Latency %g\n\n", (total_latency/(double)total_send) );
	fprintf(fp, "Router Traversals: %d\n", hoptype[0]/2+hoptype[1]/2);
	fprintf(fp, "Average Router Traversals: %g\n\n", ((double)(hoptype[0]/2+hoptype[1]/2))/total_send);
	fprintf(fp, "Link Traversals: %d\n", hoptype[1]/2);
	fprintf(fp, "Average Network Hops: %g\n\n", ((double)(hoptype[1]/2))/total_send);
	fprintf(fp, "OPORT link: %d\n\n", hoptype[0]/2);
	fprintf(fp, "Program took: %g seconds\n", time_spent);
	fprintf(fp, "******************** End *************************\n");

	// Print as list to import in excel
	fprintf(fp1, "%d\t", MAX_CPU_3D);
	fprintf(fp1, "%d\t", Traffic);
	fprintf(fp1, "%g\t", network_load);
	fprintf(fp1, "%g\t", ncycles);
	fprintf(fp1, "%g\t", GetSimTime());
	fprintf(fp1, "%d\t", total_send);
	fprintf(fp1, "%d\t", total_recv);
	fprintf(fp1, "%g\t", throughput);
	fprintf(fp1, "%g\t", (total_latency/(double)total_send));
	fprintf(fp1, "%d\t", hoptype[0]/2+hoptype[1]/2);
	fprintf(fp1, "%g\t", ((double)(hoptype[0]/2+hoptype[1]/2))/total_send);
	fprintf(fp1, "%d\t", hoptype[1]/2);
	fprintf(fp1, "%g\t", ((double)(hoptype[1]/2))/total_send);
	fprintf(fp1, "%d\t", hoptype[0]/2);
	fprintf(fp1, "%g\t", time_spent);
	fprintf(fp1, "\n");

	//close files
	fclose(fp);
	fclose(fp1);
	fclose(fl); // latency file used during sim in UserRecieve()
//********************************* End File Output *********************************//

	//Free malloc switches struct
	for(i=0; i<MAX_ROUTERS_3D; i++)
	{
		free(switches[i]);
	}

	//Free malloc measure struct
	for( i = 0; i < MAX_CPU_3D; i++ )
  	{
		free(measure[i]);
  	}

	NetworkStatRept();
}


// Router intra-connections (Build a router) minez
void intraconnections(int index)
{
	BUFFER	*buf0, *buf1;
	MUX		*mux0;
	DEMUX	*demux0;
	IPORT	*iport0;
	OPORT	*oport0;
	SWITCHSET *switchlocal;
	int i,j,k;
	int curbuf, endbuf;

	// Malloc a switch
	switches[index] = malloc(sizeof(SWITCHSET));

	switches[index]->switchid = index;
	switches[index]->xcord = FindXcord(index);
	switches[index]->ycord = FindYcord(index);
	switches[index]->zcord = FindZcord(index);

	//printf("index %d xcord %d, ycord %d\n", index, switches[index].xcord, switches[index].ycord);

	/* Look-Ahead Routing Demuxes */
	for( i = 0; i < (RADIX); i++ )
	{
		demux0 = NewDemux(demuxnum++, VC, router, LOOKAHEAD_DEMUX );
		switches[index]->input_demux[i] = demux0;
	}

	/* Regular Routing Demuxes (RC) */
	for( i = 0; i < (RADIX); i++ )
	{
		demux0 = NewDemux(demuxnum++, RADIX, router, REGULAR_DEMUX );
		switches[index]->output_demux[i] = demux0;
	}

	/* Routing/Virtual Channel Allocating Muxes (VA)  */
	for( i = 0; i < (RADIX); i++ )
	{
		mux0 = NewMux(muxnum++, VC, VIRTUAL_ALLOC_MUX );
		switches[index]->input_mux[i] = mux0;
	}

	/* Switch Allocating Muxes (SA)  */
	for( i = 0; i < (RADIX); i++ )
	{
		mux0 = NewMux(muxnum++, RADIX, SWITCH_ALLOC_MUX );
		switches[index]->output_mux[i] = mux0;
	}

	/***************************************************/
	/* Intra-Switch Connections: Component Connections */
	k = 0;
	for( i = 0; i < (RADIX); i++ )
	{
		for( j = 0; j < (VC); j++ )
		{
			/* LookAhead Router Demux to Input Virtual Channel Buffers */
			buf0 = NewBuffer(bufnum++, IBUFSZ, INPUT_BUFFER);
			switches[index]->input_buffer[k] = buf0;
			NetworkConnect(switches[index]->input_demux[i], switches[index]->input_buffer[k], j, 0);
			BufferCreditDemux(switches[index]->input_buffer[k],switches[index]->input_demux[i]);

			/* Virtual Channel Buffers to Input Virtual Allocating Mux */
			NetworkConnect(switches[index]->input_buffer[k], switches[index]->input_mux[i], 0, j);
			k++;
		}
		/* Input Virtual Allocating Mux to Regular Routing Demux */
		NetworkConnect(switches[index]->input_mux[i], switches[index]->output_demux[i], 0, 0);

		buf1 = NewBuffer(bufnum++, OBUFSZ, OUTPUT_BUFFER );
		switches[index]->output_buffer[i] = buf1;
		/* Output Switch Allocating Mux to Output Buffers */
		NetworkConnect(switches[index]->output_mux[i], switches[index]->output_buffer[i], 0, 0);
	}

	/* Intra-Switch Connections: Switch Connections */
	for( i = 0; i < (RADIX); i++ )
	{
		for( j = 0; j < (RADIX); j++ )
		{
			NetworkConnect(switches[index]->output_demux[i], switches[index]->output_mux[j], j, i);
		}
	}

	/* Intra-Switch Connections: Node Connections */
	for( i = 0; i < CONC; i++ )
	{
		iport0 = NewIPort(iportnum++, NPORTPKTS);
		oport0 = NewOPort(oportnum++, NPORTPKTS);
		switches[index]->iport[i] = iport0;
		switches[index]->oport[i] = oport0;
		k = ((RADIX) - (CONC) + i);
		NetworkConnect(switches[index]->iport[i], switches[index]->input_demux[k], 0, 0);
		NetworkConnect(switches[index]->output_buffer[k], switches[index]->oport[i], 0, 0);

		//printf("demux %d oports %d buf %d \n", switches[index].input_demux[k], k, switches[index].output_buffer[k] );
	}

	//return switches[index];
}

//**************************** Router Coordinate Funcions ****************************//
// Identifies the location of the router offset in the group
int FindXcord(int identity)
{
	// x = identity % WIDTH
	int xcord = (identity%XNUMPERDIM);
	return xcord;
}

// Identifies the location of the group offset in the system
int FindYcord(int identity)
{
	// y = (identity/WIDTH) % HEIGHT
	int ycord = (identity/XNUMPERDIM) % YNUMPERDIM;
	return ycord;
}

//Identifies the location of the Z coordinate in the system
int FindZcord(int identity)
{
	// identity / (WIDTH*HEIGHT)
	int zcord = identity / (XNUMPERDIM * YNUMPERDIM);
	return zcord;
}

// 3 dimensional to 1 dimension
int GetSwitchId(int cordx, int cordy, int cordz)
{
    int switchid = ((cordy*XNUMPERDIM) + cordx + (cordz*XNUMPERDIM*YNUMPERDIM));
    return switchid;
}

//**************************** Utility Funcions ****************************//
int power(int base, int n) // base^n
{
	int p;

	for(p = 1; n > 0; --n )
	{
		p = p * base;
	}
	return p;
}

/***************************** Permutation Pattern *****************************/
/********************************* Bit Reversal ********************************/

int BitReversal(int source)
{
	int dest, src, i, sbit;
	int bin_dest[DIMENSION1];

	src = source;
	for( i = DIMENSION1-1; i >= 0; i-- )
	{
		sbit = src%2;
		bin_dest[i] = sbit;
		src = src/2;
	}

	dest = 0;
	for( i = 0; i < DIMENSION1; i++ )
		dest = dest + ((bin_dest[i])*(power(2,i)));

	return dest;
}

/***************************** Permutation Pattern *****************************/
/********************************** Butterfly **********************************/

int Butterfly(int source)
{
	int dest, src, i, sbit, temp;
	int bin_dest[DIMENSION1];

	src = source;
	for( i = 0; i < DIMENSION1; i++ )
	{
		sbit = src%2;
		bin_dest[i] = sbit;
		src = src/2;
	}

	temp = bin_dest[0];
	bin_dest[0] = bin_dest[DIMENSION1-1];
	bin_dest[DIMENSION1-1] = temp;

	dest = 0;
	for( i = 0; i < DIMENSION1; i++ )
		dest = dest + ((bin_dest[i])*(power(2,i)));

	return dest;
}

/***************************** Permutation Pattern *****************************/
/********************************** Complement *********************************/

int Complement(int source)
{
	int dest, src, i, sbit;
	int bin_dest[DIMENSION1];

	src = source;
	for( i = 0; i < DIMENSION1; i++ )
	{
		sbit = src%2;
		bin_dest[i] = ( (sbit == 0) ? 1 : 0 );
		src = src/2;
	}

	dest = 0;
	for( i = 0; i < DIMENSION1; i++ )
		dest = dest + ((bin_dest[i])*(power(2,i)));

	return dest;
}

/***************************** Permutation Pattern *****************************/
/******************************* Matrix Transpose ******************************/

int MatrixTranspose(int source)
{
	int dest, src, i,j, sbit;
	int bin_src[DIMENSION1], bin_dest[DIMENSION1];

	src = source;
	for( i = 0; i < DIMENSION1; i++ )
	{
		sbit = src%2;
		bin_src[i] = sbit;
		src = src/2;
	}

	j = DIMENSION1 - 1;
	for( i = (DIMENSION1/2)-1; i >= 0; i-- )
	{
		bin_dest[j] = bin_src[i];
		j--;
	}

	for( i = DIMENSION1 - 1; i >= DIMENSION1/2; i-- )
	{
		bin_dest[j] = bin_src[i];
		j--;
	}

	dest = 0;
	for( i = 0; i < DIMENSION1; i++ )
		dest = dest + ((bin_dest[i])*(power(2,i)));

	return dest;
}

/***************************** Permutation Pattern *****************************/
/******************************* Perfect Shuffle *******************************/

int PerfectShuffle(int source)
{
	int dest1, dest, src, i, sbit, j;
	int bin_dest[DIMENSION1], bin_src[DIMENSION1];

	src = source;
	for( i = 0; i < DIMENSION1; i++ )
	{
		sbit = src%2;
		bin_src[i] = sbit;
		src = src/2;
	}

	j = 1;
	for( i = 0; i < DIMENSION1 - 1; i++ )
	{
		bin_dest[j] = bin_src[i];
		j++;
	}
	bin_dest[0] = bin_src[DIMENSION1-1];

	dest = 0;
	for( i = 0; i < DIMENSION1; i++ )
		dest = dest + ((bin_dest[i])*(power(2,i)));

	return dest;
}

/***************************** Permutation Pattern *****************************/
/********************************** Neighbor ***********************************/

int Neighbor(int source)
{
	int dest;

	if( source%2 != 0 )
		dest = source - 1;
	else
		dest = source + 1;

	return dest;
}

/*************************** Worst-Case Traffic Pattern *************************/
/*********************************** Tornado ************************************/
/* Should work for 8-ary, 2-cube networks */
// FIX ME FOR 3D!!
int Tornado(int source)
{
	int dest, xsrc, ysrc, zsrc, ydest;

	xsrc = FindXcord( source );
	ysrc = FindYcord( source );
	zsrc = FindZcord( source );

	ydest = (ysrc + (YNUMPERDIM - 1)/2)%(YNUMPERDIM);
	dest = GetSwitchId(xsrc, ydest, zsrc);

	return dest;
}

/***************************** Routing Variations  *****************************/
/********************************** VALIANT ************************************/

int valiant_route( int source, int dest )
{
	int tempcpu;

	do
	{
		tempcpu = RandUniformInt(0, MAX_CPU_3D - 1 );
	}while( tempcpu == source );

	return tempcpu;
}

/***************************** Routing Variations  *****************************/
/************************************ ROMM *************************************/
// THIS IS UNUSED AND I DONT CARE!
int romm_route( int source, int dest )
{
	int tempcpu;
	int xsrc, ysrc, xdest, ydest, xtemp, ytemp, xlarge, xsmall, ylarge, ysmall;
	int set = 0;

	xsrc = FindXcord( source );
	ysrc = FindYcord( source );
	xdest = FindXcord( dest );
	ydest = FindYcord( dest );

	do
	{
		tempcpu = RandUniformInt(0, MAX_CPU_3D - 1 );

		/*if( tempcpu != source ) {*/
		xtemp = FindXcord( tempcpu );
		ytemp = FindYcord( tempcpu );

		if( xsrc >= xdest )
		{
			xlarge = xsrc;
			xsmall = xdest;
		}
		else
		{
			xlarge = xdest;
			xsmall = xsrc;
		}

		if( (xtemp >= xsmall) && (xtemp <= xlarge) )
		{
			if( ysrc >= ydest )
			{
				ylarge = ysrc;
				ysmall = ydest;
			}
			else
			{
				ylarge = ydest;
				ysmall = ysrc;
			}

			if( (ytemp >= ysmall) && (ytemp <= ylarge) )
			{
				set = 1;
				break;
			}
		}
		/*tempcpu = source;
		}*/
	}while( set == 0 );

	return tempcpu;
}
