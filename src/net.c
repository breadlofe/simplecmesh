    /*****************************************************************************/
    /*****************************************************************************/
    /**                    Copyright 1993 by Rice University                    **/
    /**                              Houston, Texas                             **/
    /**                                                                         **/
    /**                           All Rights Reserved                           **/
    /**                                                                         **/
    /**    Permission to use, copy, modify, and distribute this software and    **/
    /**    its documentation  for  any research  purpose and  without fee is    **/
    /**    hereby granted, provided that the  above copyright  notice appear    **/
    /**    in all  copies and  that both  that  copyright  notice  and  this    **/
    /**    permission  notice appear in supporting  documentation,  and that    **/
    /**    the name of Rice University not be  used  in  advertising  or  in    **/
    /**    publicity pertaining to  distribution  of  the  software  without    **/
    /**    specific, written prior permission.  The inclusion of this  soft-    **/
    /**    ware  or its  documentation in  any  commercial  product  without    **/
    /**    specific, written prior permission is prohibited.                    **/
    /**                                                                         **/
    /**    RICE  UNIVERSITY  DISCLAIMS  ALL  WARRANTIES WITH  REGARD TO THIS    **/
    /**    SOFTWARE,  INCLUDING  ALL IMPLIED WARRANTIES  OF  MERCHANTABILITY    **/
    /**    AND FITNESS.  IN NO EVENT SHALL RICE UNIVERSITY BE LIABLE FOR ANY    **/
    /**    SPECIAL, INDIRECT  OR CONSEQUENTIAL DAMAGES  OR ANY DAMAGES WHAT-    **/
    /**    SOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN    **/
    /**    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS  ACTION, ARISING    **/
    /**    OUT OF  OR IN  CONNECTION  WITH  THE  USE  OR PERFORMANCE OF THIS    **/
    /**    SOFTWARE.                                                            **/
    /*****************************************************************************/
    /*****************************************************************************/

#include <math.h>
#include "../incl/simsys.h"
#include "../incl/tr.net.h"
#include "../thead.h"
#include <assert.h>

/**************************************************************************************\
********************************** NETSIM Operations ***********************************
****************************************************************************************
**                                                                                    **
**      This file contains code that implements the NETSIM extension of YACSIM.       **
**      The netsim modules are: Buffers, Multiplexers, Demultiplexers, Input Ports,   **
**      Output Port, and Duplexers.  The structures for these modules are defined     **
**      in the file simsys.h.  Their operations are defined in this file.             **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

/**************************************************************************************\
*                          Constants used only within this file                        *
\**************************************************************************************/

#define Head_Start              0   /* Head Event State */
#define Next_Module             1
#define Head_Move               2
#define Wait_Mux                3
#define Wakeup_Mux              4
#define Before_Head_Move        5
#define Before_Next_Module      6

#define Tail_Start              0   /* Tail Event States */
#define Tail_Shift              1
#define Tail_Move               2
#define Signal_Port             3
#define Tail_Done               4

#define Route_Mux				10
#define Virtual_Mux				11
#define Credit_Mux				12
#define Switch_Mux				13
#define Credit_After_Mux		14
#define Release_Virtual_Alloc	15

/**************************************************************************************\
*                          Functions used only within this file                        *
\**************************************************************************************/

int     YS__BufferHeadPut();
void    YS__BufferTailPut();
void    YS__BufferHeadGet();
void    YS__BufferTailGet();
void    YS__BufferNewSpace();
void    YS__HeadEvent();
void    YS__TailEvent();
void    YS__SignalMux();
int     YS__ShiftBubbles();
void    YS__DeliverPacket();
void    YS__PacketStatus();
int 	YS__Congestion();
void	YS__CreditFlowControl();
void	YS__VirtualChannelRelease();
void	YS__IBVirtualChannelRelease();
void	YS__IBCreditFlowControl();
void	YS__DynamicBuffer();
void	YS__StaticBuffer();
void	YS__CreditEvent();

void	DemuxCreditBuffer();
void 	BufferCreditDemux();
int		GetLinkUtil();
int		CongestionCounter();
double	PowerReturn();


/**************************************************************************************\
*                          Variables global to this file only                          *
\**************************************************************************************/

static double cycletime  = 1.0;  /* All other times a multiple of the cycle time      */
static double flitdelay  = 1.0;  /* Time to move a flit from one buffer to another    */
static double demuxdelay = 0.0;  /* Routing delay for a demux                         */
static double muxdelay   = 0.0;  /* Transfer delay for a mux                          */
static double arbdelay   = 0.0;  /* Arbitration delay for a mux                       */
static double pktdelay   = 0.0;  /* Time to move a pkt in/out of a port               */
static double delta      = 0.5;  /* Offset time between sample and alter phases       */
static int bufthresh     =   1;  /* Buffer threshold                                  */
static int bufthresh2	 =   4;
static int waitontail  = WAIT; /* WAIT: head waits on tail; NOWAIT: it doesn't wait */
static STATREC *nettimestat   = NULL;
static STATREC *blktimestat   = NULL;
static STATREC *oporttimestat = NULL;
static STATREC *movetimestat  = NULL;
static STATREC *lifetimestat  = NULL;
extern int CURRENT_ROUTE;
/**************************************************************************************\
************************************ MUX Operations ************************************
****************************************************************************************
**                                                                                    **
**      Multiplexers control access to a shared data path by performing arbitra-      **
**      tion whenever two or more packets want to use the path.  Multiplexers         **
**      have one output terminal and two or more input terminals.  Packets            **
**      request use of the link connected to the ouput terminal through one of the    **
**      input terminals.  The arbitration is implemented by a semaphore which all     **
**      packets wait on when they want to use the path.  Currently the semaphore      **
**      uses a FIFO queue.                                                            **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

MUX *NewMux(id,fanin,mtype)   /* Creates & returns a pointer to a new multiplexer     */
int    id;                    /* User assigned integer ID                             */
int    fanin;                 /* Number of input terminals for the mux                */
int	   mtype;				  /* Virtual Alloc/Switch Alloc Mux						  */
{
   MUX *mux;
   char namestr[31];

   TRACE_MUX_new;             /* Creating new mux ...                                 */

   if (fanin < 2) YS__errmsg("NewMux called with fanin less than 2");

   mux = (MUX*) malloc(sizeof(MUX));
   if (!mux) YS__errmsg("malloc failed in NewMux");

   mux->id = id;              /* ID used for debugging                                */
   mux->type = MUXTYPE;       /* Identifies module as a multiplexer                   */
   mux->fan_in = fanin;       /* Number of input terminals on the mux                 */
   sprintf(namestr,"MuxSema%d",id);
   mux->arbsema =             /* Semaphore used for arbitration for the output term.  */
      NewSemaphore(namestr,1);
   mux->nextmodule = NULL;    /* Pointer to the next module connected to this one     */
   mux->index = 0;            /* Identifies input terminal if next module is a mux    */
   mux->muxptr = NULL;        /* Pointer to the preceeding mux, if any                */
   mux->muxtype = mtype;

   return mux;
}

/**************************************************************************************\
*********************************** DEMUX Operations ***********************************
****************************************************************************************
**                                                                                    **
**      Demultiplexers perform routing by moving packets from one input terminal      **
**      to one of several output terminals.  The user must provide a pointer to a     **
**      routing function when a demultiplexer is created.  This function must take    **
**      three arguments and return an integer.  When a packet arrives at a demux,     **
**      this function is called with pointers to the IDs of the source processor      **
**      that sent the packet and the destination processor that will receive          **
**      the packet, and the interger ID of the demultiplexer itself.  The return      **
**      value is the index of the demultiplexer's output terminal through which the   **
**      packet leaves the demultiplexer.                                              **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

DEMUX *NewDemux(id,fanout,routingfcn,dtype)
int    id;                             /* User assigned integer id                    */
int    fanout;                         /* Number of output terminals on the demux     */
rtfunc routingfcn;                     /* Routing function, determines output term.   */
int	   dtype;						   /* Regular/LookAhead Router					  */
{
   DEMUX *demux;
   int j;

   TRACE_DEMUX_new;                    /* Creating new demux ...                      */

   if (fanout < 2) YS__errmsg("NewDemux called with fanout less than 2");

   demux = (DEMUX*)malloc(sizeof(DEMUX));
   if (!demux) YS__errmsg("malloc failed in NewDemux");

   demux->id = id;                     /* ID used for routing and debugging           */
   demux->type = DEMUXTYPE;            /* Identifies module as a demultiplexer        */
   demux->fan_out = fanout;            /* Numver of output terminals on the demux     */
   demux->router = routingfcn;         /* Called as routingfcn(&src, &dest, current)  */
   demux->nextmodule =                 /* Need an array of next module pointers       */
      (MODULE**)malloc(fanout*sizeof(MODULE*));
   demux->index =                      /* Need an array of next module pointers       */
      (int*)malloc(fanout*sizeof(int));
   for (j=0;j<fanout;j++) {            /* No outputs connected initially              */
      *((demux->nextmodule)+j) = NULL;
   }
   demux->demuxtype = dtype;
   /*demux->prevmodule = (MODULE*)malloc(sizeof(MODULE));*/

   demux->util = 0;

   return demux;
}

/**************************************************************************************\
*********************************** PORT Operations ************************************
****************************************************************************************
**                                                                                    **
**      Packets enter a network through input ports and leave through output ports.   **
**      Ports are the interface between networks and their external environment.      **
**      Ports are finite FIFO queues of packets (unlike buffers that are queues of    **
**      flits).  Ports can be used by sending and receiving processes to synchron-    **
**      ize their access to the port.  Sending processes can sleep at a full port     **
**      until space becomes available, and receiving processes can sleep at an        **
**      empty output port until a packet arrives.  All flits of a packet must be      **
**      in an output port before the packet can be removed.  Insertion of a packet    **
**      into an input port moves all flits of the packet into the port before any     **
**      of them move into the network.                                                **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

IPORT *NewIPort(id,sz)              /* Creates and returns a pointer to a new iport   */
int   id;                           /* User assigned integer id for the port          */
int   sz;                           /* Maximum # of packets the port can hold         */
{
   IPORT *port;
   char namestr[31];

   TRACE_IPORT_new;                 /* Creating new input port ...                    */

   if (sz <= 0) YS__errmsg("Input port size must be greater than 0");

   port = (IPORT*)malloc(sizeof(IPORT));
   if (!port) YS__errmsg("malloc failed in NewIPort");

   port->id = id;                   /* User assigned id used for debugging            */
   port->type = IPORTTYPE;          /* Identifies this module as an iport             */
   port->nextmodule = NULL;         /* Iport not connected yet                        */
   sprintf(namestr,"PortRdy%d",id);
   port->qfree = sz;                           /* Port empty to start                 */
   port->portrdy = NewSemaphore(namestr,sz);   /* Semvalue = # of free pkt positions */
   /* Sending processes can wait in the PortReady semaphore until the port has space  */

   sprintf(namestr,"NetRdy%d",id);
   port->netrdy = NewSemaphore(namestr,1);     /* 1 when net ready for pkt from iport */
   /* A packet's head event will wait in NetReady until it can  enter the network     */

   return port;
}

/**************************************************************************************/

SEMAPHORE *IPortSemaphore(iptr)     /* Returns a pointer to an iport's PortReady sema */
IPORT *iptr;                        /* Pointer to the iport                           */
{
   return iptr->portrdy;
}

/**************************************************************************************/

int IPortSpace(port)                /* Returns # of free packet spaces in an iport    */
IPORT *port;                        /* Pointer to the iport                           */
{
   return port->qfree;
}

/**************************************************************************************/

int IPortGetId(port)                 /* Returns user assigned id of an iport          */
IPORT *port;                         /* Pointer to the iport                          */
{
   return port->id;
}

/**************************************************************************************/

OPORT *NewOPort(id,sz)              /* Creates and returns a pointer to a new oport   */
int    id;                          /* User assigned integer id for the port          */
int sz;                             /* Maximum # of packets the port can hold         */
{
   OPORT *port;
   char namestr[31];

   TRACE_OPORT_new;                 /* Creating new output port ...                   */

   if (sz <= 0) YS__errmsg("Input port size must be greater than 0");

   port = (OPORT*) malloc(sizeof(OPORT));
   if (!port) YS__errmsg("malloc failed in NewOPort");

   port->id = id;                   /* User assigned id for debugging                 */
   port->type = OPORTTYPE;          /* Identifies this module as an oport             */
   port->qhead = NULL;              /* Pointers for queue of packets in the port      */
   port->qtail = NULL;              /*   waiting to be received by a process          */
   port->count = 0;                 /* # of packets in the queue, 0 to start          */
   port->power = 0.0;
   port->link = 0;
   port->crossbar = 0;
   port->input_buffer = 0;

   sprintf(namestr,"FreeSpace%d",id);
   port->freespace = NewSemaphore(namestr,sz);

   /* FreeSpace semaphore value = # of packets the port can receive from the network  */
   /* Head events wait in this semaphore's queue until port space is avaialable       */

   sprintf(namestr,"PacketAvail%d",id);
   port->pktavail = NewSemaphore(namestr,0);

   /* PacketAvail semaphore value = # of packets in the port.  Receiving processes    */
   /* can wait in this semaphore's queue until packets arrive at the port             */

   return port;
}

/**************************************************************************************/

SEMAPHORE *OPortSemaphore(optr)      /* Returns a pointer to an oport's semaphore     */
OPORT *optr;                         /* Pointer to the oport                          */
{
   return optr->pktavail;
}

/**************************************************************************************/

int OPortPackets(port)               /* Returns # of packets available in an oport    */
OPORT *port;                         /* Pointer to the oport                          */
{
   int retval;

   retval = port->count;             /* Available packets at port                     */
   TRACE_OPORT_available;            /* Checking for packet at port ...               */

   return retval;
}

/**************************************************************************************/

int OPortGetId(port)                 /* Returns the user assigned ID of an oport      */
OPORT *port;                         /* Pointer to the oport                          */
{
   return port->id;
}

/**************************************************************************************\
********************************** BUFFER Operations ***********************************
****************************************************************************************
**                                                                                    **
**      Buffers are finite queues that hold flits.  They are inplemented as FIFO      **
**      queues.  NewBuffer() is the only operation on buffers available to users.     **
**      It creates a new finite buffer of a given size.  A threshold for the buffer   **
**      must also be specified.  It sets the number of buffer positions that must     **
**      be free before a flit can enter the buffer.                                   **
**                                                                                    **
**      The way buffers are implemente is that the only elements that are actually    **
**      linked together in their queues are head and tail flits.   The locations      **
**      of the flits between the head and tail flits is determined by several counts  **
**      in the buffer and packet structures, rather that putting an element in the    **
**      linked list for every flit.                                                   **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

BUFFER *NewBuffer(id,sz,btype) /* Creates & returns a pointer to a new network buffer */
int    id;                     /* User assigned integer ID for the buffer             */
int    sz;                     /* Maximum # of flits the buffer can hold              */
int	   btype;
{
   BUFFER *buf;
   char namestr[31];
   int i;

   TRACE_BUFFER_new;           /* Creating network buffer ...                         */
   buf = (BUFFER*) malloc(sizeof(BUFFER));
   if (!buf) YS__errmsg("malloc failed in NewBuffer");

   buf->id = id;               /* ID used for debugging                               */
   buf->type = BUFFERTYPE;     /* Identifies this module as a buffer                  */
   buf->size = sz;             /* Maximum number of flits the buffer can hold         */
   buf->head = NULL;           /* Pointer to head or tail flit at head of buffer      */
   buf->tail = NULL;           /* Pointer to head or tail flit at tail of buffer      */
   buf->free = sz;             /* All buffer slots free to start                      */
   buf->nextmodule = NULL;     /* Next module connected to buffer output terminal     */
   buf->index = 0;             /* Identifies input termial if next module is a MUX    */
   buf->destination = NULL;    /* Pointer to the next buffer or oport tail will enter */
   buf->muxptr= NULL;          /* Pointer to the previous MUX head passed thru        */
   buf->WaitingHead = NULL;    /* Pointer to the packet waiting to enter the buffer   */
   buf->buftype = btype;	   /* Specifies whether the buffer is input/output		  */
   buf->prevmodule = NULL;	   /* Previous module needed to connect to demux for credits */

//printf("%d %d\n", id, buf->buftype);

   if( buf->buftype == INPUT_BUFFER ) {
	   buf->output_port = -1;
	   buf->output_vc = -1;
	   buf->output_seqno = -1;
	   buf->output_credit = CREDITS;
	   buf->output_alloc = -1;

	   /* This semaphore is used when waiting for credits */
	   sprintf(namestr,"CreditSema%d",id);
	   buf->bufcredit_sema = NewSemaphore(namestr,1);
	   buf->outputbuf = NULL;
   }
   else {
	   buf->vc_data.input_port = (int*)malloc(VC*sizeof(int));
	   buf->vc_data.input_vc = (int*)malloc(VC*sizeof(int));
	   buf->vc_data.input_credit = (int*)malloc(VC*sizeof(int));
	   buf->vc_data.input_seqno = (int*)malloc(VC*sizeof(int));
	   buf->vc_data.inputbuf = (MODULE**)malloc(VC*sizeof(MODULE*));
	   buf->vc_data.creditime = (double*)malloc(VC*sizeof(double));

	   buf->vc_data.congestion = 0;
	   buf->vc_data.keep_track = 0;

	   for( i = 0; i < VC; i++ ) {
		   buf->vc_data.input_port[i] = -1;
		   buf->vc_data.input_vc[i] = -1;
		   buf->vc_data.input_seqno[i] = -1;
		   buf->vc_data.input_credit[i] = CREDITS;
		   *((buf->vc_data.inputbuf)+i) = NULL;
		   buf->vc_data.creditime[i] = 0.0;
	   }

	   sprintf(namestr, "VAlloc%d", id);
	   buf->va_alloc = NewSemaphore(namestr,1);
	   sprintf(namestr,"VASema%d",id);
	   buf->bufva_sema = NewSemaphore(namestr,1);
	   sprintf(namestr,"CCSema%d",id);
	   buf->bufcc_sema = NewSemaphore(namestr,1);
   }
   return buf;
}

/**************************************************************************************/

void YS__BufferHeadGet(buf)       /* Removes a head flit from head of buffer   */
BUFFER *buf;                             /* Pointer to the buffer                     */
{
   PACKET *pkt;

   if( buf->head == NULL )
   	return;
   buf->free++;                           /* Take head out, freeing a flit slot       */
   pkt = buf->head;                       /* Get pointer to head's packet             */
   buf->head = pkt->headnext;             /* Readjust the buffer's head pointer       */
   pkt->headnext = NULL;                  /* Head flit no longer in a buffer          */

   if (buf->head == NULL) {              /* This was the only event in the buffer     */
      buf->tail = NULL;                  /* Therefore the packet's tail is in another */
   }
   else {                                /* Packet's tail in same buffer head was in  */
      pkt->tailoffset =                  /* That tail now first element in buffer     */
         pkt->data.pktsize - 2;          /*    and is offset from the front           */
   }

   /* Head event will wake it up head's sleeping tail after returning from HeadGet    */
   YS__BufferNewSpace(buf);              /* Check for waiting head or tail            */
   return;
}

/**************************************************************************************/

int YS__BufferHeadPut(buf,pkt) /* Appends head of a packet to a buffer         */
BUFFER *buf;                          /* Pointer to the buffer                        */
PACKET *pkt;                          /* Pointer to the packet                        */
{
	DEMUX *newdemux;
	OPORT* oport;
	BUFFER *output_prevbuf, *ibuf;
	int temp, k;

	if(buf->type != BUFFERTYPE)
	{
		YS__errmsg("Non-buffer called to head put\n");
	}

	pkt->headbuf = (MODULE*)buf;   /* Headbuf points to the head's buffer              */
	pkt->headnext = NULL;          /* Head will be at end of buffer, nothing behind it */
	buf->free--;                   /* One fewer free flit slot in that buffer          */
	buf->tailtype = HEADTYPE;      /* Last event in buffer is a head event             */

	if (buf->head == NULL) {       /* Inserting head flit into an empty buffer         */
		buf->head = pkt;            /* Packet's head will be only element in the buffer */
		buf->tail = pkt;
		TRACE_PACKET_show;          /* Debugging trace                                  */
		return 1;                   /* Head flit entered an empty buffer                */
	}                              /* Head is at the front of the buffer               */
	else {                         /* Buffer had at least one tail flit in it          */
		buf->tail->tailnext = pkt;  /* Append the head flit to the buffer               */
		buf->tail = pkt;            /* Head is at the back of the buffer                */
		TRACE_PACKET_show;          /* Debugging trace                                  */
		return 0;                   /* Head flit entered a nonempty buffer              */
	}                              /* Head flit behind other flits in the buffer       */
}

/**************************************************************************************/

void YS__BufferTailGet(buf)       /* Removes a tail flit from head of buffer   */
BUFFER *buf;                             /* Pointer to the buffer                     */
{
   PACKET *pkt;
   BUFFER *newbuf, *output_prevbuf, *input_prevbuf;
   DEMUX *newdemux;
   int k = 0;
   int prev_vcindex,j, i, temp;

   buf->free++;                           /* Departing tail creates a free flit slot  */
   pkt = buf->head;                       /* Get pointer to tail's packet             */
   buf->head = pkt->tailnext;             /* New head of buffer                       */
   pkt->tailnext = NULL;                  /* Tail not in a buffer now                 */
   buf->destination = NULL;               /* Dest. no longer needed to route tail     */
   if (buf->head == NULL) {               /* This was the only event in the buffer    */
      buf->tail = NULL;                   /* Therefore the buffer is now empty        */
   }
   else { /* Buffer not empty, blocked head waiting behind departing tail             */
      ActivitySchedTime(buf->head->headev,delta,INDEPENDENT);
   }

   YS__IBCreditFlowControl(buf, pkt);
   YS__IBVirtualChannelRelease(buf,pkt);

   YS__BufferNewSpace(buf);               /* Check for waiting head or tail           */

   return;
}

/**************************************************************************************/

void YS__BufferTailPut(buf,pkt)  /* Appends tail of a packet to a buffer       */
BUFFER *buf;                            /* Pointer to the buffer                      */
PACKET *pkt;                            /* Pointer to the event                       */
{
   pkt->tailbuf = (MODULE*)buf;     /* Tailbuf points to the tails buffer             */
   pkt->tailnext = NULL;            /* Tail will be at buffer tail, nothing behind it */
   buf->free--;                     /* One fewer free flit slot in that buffer        */
   buf->tailtype = TAILTYPE;        /* Last event in buffer is a tail event           */

   if (buf->head == NULL) {         /* Inserting tail flit into an empty queue        */
      pkt->tailoffset =             /* There may be interior flits in the buffer      */
         buf->size-buf->free-1;     /* Position of tail flit from head, starting at 0 */
      buf->head = pkt;              /* Since the tail flit is only flit in buffer     */
      buf->tail = pkt;
   }
   else {                           /* Packets head must be in this buffer            */
      pkt->tailoffset = -1;         /* -1 means that this tail is not first in buffer */
      pkt->headnext = pkt;          /* Append the tail flit to the buffer             */
      buf->tail = pkt;              /* Tail is now the last event in the buffer       */
   }

   if (pkt->waitingfortail) // && pkt->module->type != OPORTTYPE) {
   {
      TRACE_TAIL_tailarrives;
      ActivitySchedTime(pkt->headev,delta,INDEPENDENT);
      pkt->waitingfortail = 0;
   }

   TRACE_PACKET_show;               /* Debugging trace                                */
   return;
}

/**************************************************************************************/

void YS__BufferNewSpace(buf)      /* Activates flits waiting to enter buffer   */
BUFFER *buf;                             /* Pointer to the buffer                     */
{
   PACKET *pkt;                          /* Pointer to the event                      */

   /* Check for a head event now ready to enter the buffer                            */

   if (buf->WaitingHead &&               /* There is a head waiting to enter          */
       buf->free >= bufthresh)           /* & there is space in the buffer            */
   {
      TRACE_HEAD_freetomove;             /* Head of packet free to enter module       */
      ActivitySchedTime(buf->WaitingHead,flitdelay,INDEPENDENT);
      buf->WaitingHead = NULL;           /* Just in case it is set                    */
   }

   /* Check for a sleeping tail event in a preceeding buffer that should wake up      */
   if (buf->tail != NULL &&              /* There is a last event in bufffer          */
       buf->tailtype == HEADTYPE)        /*    and it is a head                       */
   {
      pkt = buf->tail;                   /* Get a pointer to that head's pkt          */
      if (pkt->SleepingTail != NULL) {   /* That packts tail is sleeping              */
         TRACE_TAIL_wakes;               /* Tail of packet wakes up                   */
         ActivitySchedTime(pkt->SleepingTail, delta, INDEPENDENT);
         pkt->SleepingTail = NULL;       /* Tail not waiting anymore                  */
      }
   }
}

/**************************************************************************************\
********************************** NETWORK Operations **********************************
****************************************************************************************
**                                                                                    **
**      The network operations are used to interconnect the network modules to        **
**      form a network, and to specify the network delays and routing mode.  Four     **
**      routing modes are implemented:  Detailed wormhole routing, approximate        **
**      wormhole routing, virtual cut through routing and store and forward routing.  **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

void NetworkConnect(source, dest, src_index, dest_index) /* Connects two modules      */
MODULE *source;                    /* Pointer to the sending module                   */
MODULE *dest;                      /* Pointer to the receiving module                 */
int    src_index;                  /* Ouput terminal, only used for demux senders     */
int    dest_index;                 /* Input terminal, only used for mux receivers     */
{
   int    typ;
   BUFFER *buf;
   MUX    *mux;
   DEMUX  *demux;
   IPORT  *iport;

   typ = source->type;                /* Module type of source                        */
   switch (typ) {

      case BUFFERTYPE:
         TRACE_NETWORK_connectbuf;    /* Connecting buffer to ...                     */
         buf = (BUFFER*)source;       /* Cast source to type BUFFER                   */
         if (dest->type == MUXTYPE)   /* Destination is a MUX                         */
            buf->index = dest_index;  /* Buffer must know which input terminal to use */
         buf->nextmodule = dest;      /* Make the connection                          */
         //if( buf->buftype == OUTPUT_BUFFER)
        // printf("Connexion src %d I/O %d dest %d %d\n", buf->id, buf->buftype, dest->type, dest->id);
         break;

      case MUXTYPE:
         TRACE_NETWORK_connectmux;    /* Connecting multiplexer to ...                */
         mux = (MUX*)source;          /* Cast source to type MUX                      */
         if (dest->type == MUXTYPE)   /* Destination is a MUX                         */
            mux->index = dest_index;  /* Mux must know which input terminal to use    */
         mux->nextmodule = dest;      /* Make the connection                          */
         break;

      case DEMUXTYPE:
         TRACE_NETWORK_connectdemux;  /* Connecting demultiplexer to ...              */
         demux = (DEMUX*)source;      /* Cast source to type DEMUX                    */
         if (dest->type == MUXTYPE) {  /* Destination is a MUX                         */
            *((demux->index)+src_index) =          /* Index of input terminal to use  */
               dest_index;                         /*    on the next module           */
		   }
         *((demux->nextmodule)+src_index) = dest;  /* Make the connection             */
         break;

      case IPORTTYPE:
         TRACE_NETWORK_connectiport;  /* Connecting input port to ...                 */
         iport = (IPORT*)source;      /* Cast source to type IPORT                    */
         if (dest->type == MUXTYPE)   /* Destination is a MUX                         */
            iport->index = dest_index;/* IPort must know which input terminal to use  */
         iport->nextmodule = dest;    /* Make the connection                          */
         break;

      default: YS__errmsg("error in specifying network");
   }
}

/**************************************************************************************/

void NetworkSetCycleTime(x) /* Sets the cycle time. All other times a multiple of it  */
double x;                   /* Delay value: time to move flit between buffers         */
{
   cycletime = x;
   delta = cycletime/2;
}

/**************************************************************************************/

void NetworkSetFlitDelay(i)   /* Sets the flit delay                                  */
int i;                        /* Delay value: time to move flit between buffers       */
{
   flitdelay = (double)(i*cycletime);        /* Value is a multiple of cycle time     */
}

/**************************************************************************************/

void NetworkSetMuxDelay(i)    /* Sets the MUX delay                                   */
int i;                        /* Delay value: time for head flit to move thru mux     */
{
   muxdelay = (double)(i*cycletime);         /* Value is multiple of cycle time       */
}

/**************************************************************************************/

void NetworkSetArbDelay(i)     /* Sets the multiplexer arbitration time               */
int i;                         /* Delay value: Time to perform arbitration            */
{
   arbdelay = (double)(i*cycletime);         /* Value is multiple of cycle time       */
}  /* This delay should probably be some function of the number of arbitrating heads  */

/**************************************************************************************/

void NetworkSetDemuxDelay(i)   /* Sets the demultplexer routing delay                 */
int i;                         /* Delay value: time to perform routing                */
{
   demuxdelay = (double)(i*cycletime);         /* Value is multiple of flit delay     */
}

/**************************************************************************************/

void NetworkSetPktDelay(i)    /* Sets the packet delay                                */
int i;                        /* Delay value: time to move flit between buffers       */
{
   pktdelay = (double)(i*cycletime);         /* Value is a multiple of cycle time     */
}

/**************************************************************************************/

void NetworkSetThresh(i)       /* Sets buffer threshold                               */
int i;                         /* New threshold value                                 */
{
   bufthresh = i;
}

/**************************************************************************************/

void NetworkSetWFT(i)
int i;
{
   waitontail = i;
}


/**************************************************************************************\
********************************** PACKET Operations ***********************************
****************************************************************************************
**                                                                                    **
**      Packets are the objects that move through a network.  They are implemented    **
**      as events that are created when a packet is created, that work their way      **
**      through the network, delaying to simulate the passage of time, and then       **
**      going away when the packet reaches its destiantion.  Operations are pro-      **
**      vided for the user to create new packets, destroy received packets, send      **
**      packets into the network, receive packets from the network, extract data      **
**      from a packet, and display the status of a packet.  The events that control   **
**      packet movement in a network are where most of the work of NETSIM is done.    **
**      There is one for the head flit of a packet and one for its tail flit.         **
**      Thus only two events are required for each packet no matter how many flits    **
**      are in the packet.                                                            **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

PACKET *NewPacket(seqno,msgptr,sz)   /* Creates & returns a ptr to a packet           */
int     seqno;                       /* Sequence number of packet within its message  */
MESSAGE *msgptr;                     /* Pointer to the packet's message               */
int     sz;                          /* Number of flits in the packet                 */
{
   PACKET *pkt;

   pkt = (PACKET*)YS__PoolGetObj(&YS__PktPool);

   /* Initialize user accessible data; other fields set in PacketSend()               */

   pkt->data.seqno = seqno;            /* Used for identifying packets of a message   */
   pkt->data.mesgptr = msgptr;         /* Used only if packet is part of a message    */
   pkt->data.pktsize = sz;             /* Number of flits in the packet               */
   pkt->data.createtime = YS__Simtime; /* Time the packet was created                 */
   pkt->data.blktime = 0.0;            /* Time spent blocked in the network           */
   pkt->data.oporttime = 0.0;          /* Time spent in an output port                */
   pkt->data.nettime = 0.0;            /* Time spent in the network                   */

   TRACE_PACKET_new;                   /* Creating new packet ...                     */
   return pkt;
}

/**************************************************************************************/

double PacketSend(pkt,port,src,dest,type, size, tag)
PACKET *pkt;                           /* Pointer to the packet to send               */
IPORT  *port;                          /* Pointer to the network input port           */
int    src;                            /* ID of source CPU                            */
int    dest;                           /* ID of destination CPU                       */
int	   type;
int	   size;
int	   tag;
{
   EVENT *headevent;
   EVENT *tailevent;
   double x;
   char namestr[31];
   DEMUX *newdemux;
   BUFFER *ibuf;
   int k, set;

   TRACE_IPORT_send1;                   /* Attempting to send packet ...              */

   if (port->qfree <= 0) {              /* Port full, not ready for a new packet      */
      TRACE_IPORT_send2;                /* - Port full, send fails                    */
      return -1.0;
   }

   else {                               /* Port accepts the packet                    */

      pkt->data.srccpu = src;           /* Sending cpu or port                        */
      pkt->data.destcpu = dest;         /* Receiving cpu or port                      */

      pkt->data.vcindex = 0;			/* From Processor, input index = 0			  */

      pkt->module = (MODULE*)port;      /* Packet in the port                         */
      pkt->tailbuf = (MODULE*)port;     /* Tail in the port                           */
      pkt->headbuf = (MODULE*)port;     /* Head in the port                           */
      pkt->tailoffset = -1;             /* -1: tail flit not front element in a buf   */
      pkt->lastiport = port;            /* Remember the entry port                    */
      pkt->waitingfortail = 0;
      pkt->muxptr = NULL;               /* Ptr to list of muxes head passed through   */
      pkt->data.pkttype = type;
      pkt->data.packetsize = size;
      pkt->data.routeA = 0;
      pkt->data.powers = 0.0;
      pkt->data.ibufs = 0;
      pkt->data.xbar = 0;
      pkt->data.lnk = 0;
      pkt->data.vastage = 0;
      pkt->data.rcstage = 0;
      pkt->data.sastage = 0;
      pkt->data.tag = tag;

      port->qfree--;                    /* One fewer port packet slot                 */
      if (port->qfree < SemaphoreValue(port->portrdy)) /* Portrdy can not be > qfree  */
         SemaphoreDecr(port->portrdy);
         /* Ready could be < qfree if a process, maybe this one, has waited at and    */
         /* decremented semaphore portrdy, prior to the activation of this routine    */

      /* Create the head event */

      sprintf(namestr,"Head%d",pkt->data.seqno);
      headevent = NewEvent(namestr,YS__HeadEvent,NODELETE,0);
      ActivitySetArg(headevent,pkt,HEADTYPE);       /* Gives head access to pkt info  */
      pkt->headev = headevent;

      /* Create the tail event */

      sprintf(namestr,"Tail%d",pkt->data.seqno);
      tailevent = NewEvent(namestr,YS__TailEvent,NODELETE,0);
      ActivitySetArg(tailevent,pkt,TAILTYPE);       /* Gives tail access to pkt info  */
      pkt->SleepingTail = tailevent;                /* Tail waiting for head to signal*/

      /* Synchronize the head event with the network cycle time and schedule it */

      if (cycletime > 0.0) {
        x = fmod(YS__Simtime,cycletime);
        if (x != 0.0) x = cycletime - x;
      }
      else x = 0.0;

      pkt->data.nettime =  /* Measures time from head entering to tail leaving net    */
         pkt->data.nettime - (YS__Simtime + x); /* Will add later Simtime to get diff */

      x = x + (pkt->data.pktsize)*pktdelay;       /* Plus time for pkt to enter port  */
      ActivitySchedTime(headevent,x,INDEPENDENT); /* Head wakes up when pkt in port   */

       /* Ports use saf flit control       */
      TRACE_IPORT_send3;                /* - Port ready, packet queued ...            */
      return x;                         /* Returns time to move packet into buffer    */
   }                                    /* Caller can use this time to delay          */
}

/**************************************************************************************/

PACKET *PacketReceive(port)    /* Receives a packet from a network output port        */
OPORT  *port;                  /* Pointer to the port                                 */
{
	//printf("IN PACKET RECIEVE pkt(%d)\n", port->id);

   PACKET   *pkt = NULL;
   ACTIVITY *aptr;
   double   x;

   TRACE_OPORT_receive1;                          /* Attempting to receive a packet   */

   if ( port->count > 0) {                        /* A packet is available            */
      pkt = port->qhead;                          /* Take it out of the port's queue  */
      port->qhead = pkt->next;
      pkt->next = NULL;
      if (port->qhead == NULL) port->qtail = NULL;
      port->count--;                              /* Decrement number of avail. pkts  */

      TRACE_OPORT_receive2;                       /* Received packet ...              */

      if (SemaphoreValue(port->pktavail) > port->count) /* Pktavail must be <= count  */
      {
         SemaphoreDecr(port->pktavail);
         printf("\nHERE\n\n");
      }
          /* Pktavail could be < count if another process has waited at & decremented */
          /* semaphore pktavail, but has not yet called packet receive                */

      /* Synchronize packets removal from port  with the network cycle time           */

      if (cycletime > 0.0) {
         x = fmod(YS__Simtime,cycletime);
         if (x != 0.0) x = cycletime - x;
      }
      else x = 0.0;

      /* The following code segement is essentially a Semaphore Signal operation with */
      /* the scheduling of a released activity delayed by x to synchronize with head  */
      /* movements and by flitdelay for time to move head flit into the oport queue   */
      /* The scheduled head wakes up at Head_Move                                     */

      if (port->freespace->size == 0)  {           /* Semaphore queue is empty        */
         port->freespace->val++;                   /* Increment its value             */
      }
      else  {                                     /* Queue not empty                 */
         aptr = (ACTIVITY*)YS__QueueGetHead(port->freespace);    /* Get head of queue */
         ActivitySchedTime(aptr, x +flitdelay - delta, INDEPENDENT); /* & schedule it */
         //printf("\nHERE in packet recieve: pkt(%d), event(%d-%s)\n\n", pkt->data.seqno, aptr->id, aptr->name);
      }


      pkt->data.oporttime =           /* Update the accumlation of oport waiting time */
         pkt->data.oporttime + YS__Simtime + x;

      x = x + (pkt->data.pktsize)*pktdelay;    /* Plus time for pkt to leave port     */

      pkt->data.recvtime = x;                  /* Time to remove packet from port     */
                                               /* Caller can use this time to delay   */
      pkt->data.nettime =                      /* Update time pkt was in the network  */
         pkt->data.nettime + YS__Simtime + x;  /* Include time to move it out of port */
   }
   else {
      TRACE_OPORT_receive3;                    /* No packets available                */
   }

	port->power = port->power + pkt->data.powers;
	port->crossbar = port->crossbar + pkt->data.xbar;
	port->input_buffer = port->input_buffer + pkt->data.ibufs;
	port->link = port->link + pkt->data.lnk;
	port->route = port->route + pkt->data.rcstage;
	port->vc = port->vc + pkt->data.vastage;
	port->swtch = port->swtch + pkt->data.sastage;
   return pkt;                                 /* Returns NULL if no packet available */
}

/**************************************************************************************/

PKTDATA *PacketGetData(pkt)   /* Returns a pointer to a packet's user accessible data */
PACKET *pkt;
{
   return &(pkt->data);
}

/**************************************************************************************/

void PacketFree(pkt)          /* Returns a packet to the pool of free packets         */
PACKET *pkt;
{
   /* Update those network statistics records that have been activated */

   if (nettimestat != NULL)
      StatrecUpdate(nettimestat,pkt->data.nettime,1.0);
   if (blktimestat != NULL)
      StatrecUpdate(blktimestat,pkt->data.blktime,1.0);
   if (oporttimestat != NULL)
      StatrecUpdate(oporttimestat,pkt->data.oporttime,1.0);
   if (movetimestat != NULL)
      StatrecUpdate(movetimestat,
         pkt->data.nettime - pkt->data.blktime - pkt->data.oporttime,1.0);
   if (lifetimestat != NULL)
      StatrecUpdate(lifetimestat,YS__Simtime - pkt->data.createtime,1.0);

   YS__PoolReturnObj(&YS__PktPool, pkt);
}

/**************************************************************************************/

void YS__PacketStatus(pkt)    /* Prints the status of a packet                        */
PACKET *pkt;                  /* Pointer to the packet                                */
{
   MODULE *bp;
   int    flits;
   int    i;

   bp = pkt->tailbuf;
   flits = 0;
   TracePrintTag("Packet","Packet %d status:",pkt->data.seqno);
   if (pkt->tailbuf != pkt->headbuf) {
      TracePrintTag("Packet"," %d/%d",bp->id,pkt->tailoffset+1);
      flits = pkt->tailoffset+1;
      if (bp->type == IPORTTYPE) bp = ((IPORT*)bp)->destination;
      else if (bp->type == BUFFERTYPE) bp = ((BUFFER*)bp)->destination;
      while (bp != pkt->headbuf) {
         i = ((BUFFER*)bp)->size - ((BUFFER*)bp)->free;
         TracePrintTag("Packet"," %d/%d",bp->id,i);
         flits = flits  + i;
         bp = ((BUFFER*)bp)->destination;
      }
   }
   TracePrintTag("Packet"," %d/%d\n",bp->id,pkt->data.pktsize-flits);
}

/**************************************************************************************/

void YS__HeadEvent()
{
	BUFFER    *buf, *buf_trace, *newbuf, *obuf;
	MUX       *mux, *mux_trace;
	DEMUX     *demux, *demux_trace;
	PACKET    *pkt;
	IPORT     *iport;
	OPORT     *oport;

	MODULE    *curmod;
	int       curtype, vc, temp, ret;
	int       athead = 0;
	int		  k = 0;
	double	  x = 0.0;

	pkt = (PACKET*)ActivityGetArg(ME);

//	if(pkt->data.seqno == 203)
//	{
//		printf("IM THE PACKET (203)\n");//, Event_id = %d\n",  pkt->headev->id); //, Event_name = %s\n", pkt->headev->id, pkt->headev->name);
		//if(GetSimTime() > 35)
			//YS__EventListPrint();
//	}


	//printf("src %d dest %d module %d seqno %d type %d size %d Event %d\n",
	//	pkt->data.srccpu, pkt->data.destcpu, pkt->module->id, pkt->data.seqno,
	//	pkt->data.pkttype, pkt->data.packetsize, EventGetState() );

	switch (EventGetState()) {

		case Head_Start: {
			iport = (IPORT*)pkt->module;
			if (SemaphoreValue(iport->netrdy) <= 0) {
				pkt->data.blktime = pkt->data.blktime - YS__Simtime;
				//printf("Waiitng for sema iport\n");
				EventReschedSema(iport->netrdy,Before_Next_Module);
				return;
			}
			else {
				TRACE_HEAD_iportrdy;
				SemaphoreDecr(iport->netrdy);
				goto label_Next_Module;
			}
		}

		case Before_Next_Module: {
			pkt->data.blktime = pkt->data.blktime + YS__Simtime;
		}

		case Next_Module: label_Next_Module: {
			if ( waitontail == WAIT && pkt->headbuf != pkt->tailbuf) {
				TRACE_HEAD_chkwft;
				//printf("Wait on tail\n");
				EventSetState(ME, Next_Module);
				pkt->waitingfortail = 1;
				return;
			}

			TRACE_HEAD_nextmod;
			curmod = pkt->module;
			curtype = curmod->type;
			//printf(" cur mod %d cur type %d \n", curmod->id, curtype);

			if (curtype == BUFFERTYPE) {
				//printf("Checking for buffer\n");
				pkt->module = ((BUFFER*)curmod)->nextmodule;
				//printf("In Buffer %d\n", pkt->module->id);
			}
         	else if (curtype == MUXTYPE) {
         		pkt->module = ((MUX*)curmod)->nextmodule;
  				//printf("In MuX %d\n", pkt->module->id);
			}
         	else if (curtype == DEMUXTYPE) {
				pkt->module = *((((DEMUX*)(curmod))->nextmodule)+(pkt->index));
				//printf("In DEMUX %d index %d\n", pkt->module->id, pkt->index);
				TRACE_HEAD_demuxport;
			}
			else if (curtype == IPORTTYPE) {
				pkt->module = ((IPORT*)curmod)->nextmodule;
				//printf("In IPORT %d\n", pkt->module->id);
			}
			else YS__errmsg("Invalid current module type");

			/********************* LOOKAHEAD ROUTER & REGULAR ROUTER ***************************/
			if (pkt->module->type == DEMUXTYPE) {
				demux = (DEMUX*)pkt->module;
				TRACE_HEAD_nextdemux1;

				pkt->data.route = (demux->router)(&(pkt->data.srccpu),&(pkt->data.destcpu),demux->id);
				//printf("returning %d in demux %d\n", pkt->data.route, demux->id);

				/* Check if the demux is the look ahead router */
				if(demux->demuxtype == LOOKAHEAD_DEMUX) {
					pkt->index = pkt->data.vcindex;
					demux->util = demux->util + 1;
					//if( (demux->id)%(RADIX) != RADIX ) {
					//	pkt->data.powers = pkt->data.powers + LINK_POWER;
					//	pkt->data.lnk = pkt->data.lnk + 1;
					//}
				}
				else {
				/* Regular router */
					pkt->index = pkt->data.route;
				}

				TRACE_HEAD_nextdemux2;
				EventReschedTime(demuxdelay, Next_Module);
				//printf("Route %d vcindex %d\n", pkt->data.route, pkt->data.vcindex);

				return;
			}

			/********************* VIRTUAL CHANNEL & SWITCH ALLOCATION *************************/
			else if (pkt->module->type == MUXTYPE) {
				mux = (MUX*)(pkt->module);
				//printf("MUX id %d\n", mux->id);
				TRACE_HEAD_nextmux;

				/* Regular Switching Required for all Flits (Head and Body Flits) */
				if( mux->muxtype == SWITCH_ALLOC_MUX ) {
					//printf("SMUX id %d\n", mux->id);
					EventReschedTime(0.0, Switch_Mux);
					return;
				}

				/* Else, check for routing/VA if head, else only for credit */
				if( pkt->data.pkttype == 0 && mux->muxtype == VIRTUAL_ALLOC_MUX ) {
					//printf("VMUX id %d\n", mux->id);
					EventReschedTime(0.0, Route_Mux);
					return;
				}
				else {
					EventReschedTime(0.0, Credit_Mux);
					//EventReschedTime(0.0, Switch_Mux);
					return;
				}
			}

			/*************************** BUFFER ALLOCATION **************************************/
			else if (pkt->module->type == BUFFERTYPE) {
				//printf("BUF id %d\n", pkt->module->id);
				buf = (BUFFER*)(pkt->module);
				//printf("BUF id %d\n", buf->id);
				TRACE_HEAD_nextbuf;

				if (pkt->headbuf->type == IPORTTYPE)
				   ((IPORT*)pkt->headbuf)->destination = pkt->module;
            	else if (pkt->headbuf->type == BUFFERTYPE)
            	   ((BUFFER*)pkt->headbuf)->destination = pkt->module;

            	buf->muxptr = pkt->muxptr;
            	pkt->muxptr = NULL;

				/* Needed for keeping track of the input buffer traversing */
            	pkt->carrybuf = (MODULE*)buf;

            	if (buf->free >= bufthresh) {
					TRACE_HEAD_spaceavail;
            		EventReschedTime(flitdelay-delta,Head_Move);
            		buf->WaitingHead = NULL;
            	}
            	else {
            		TRACE_HEAD_buffull;
            		demux_trace = (DEMUX*)buf->prevmodule;

            		buf->WaitingHead = pkt->headev;
            		pkt->data.blktime = pkt->data.blktime - YS__Simtime;
            		EventSetState(NULL, Before_Head_Move);
            	}
            	return;
         	}

			else if (pkt->module->type == OPORTTYPE) {
				//printf("in oport\n");
				oport = (OPORT*)(pkt->module);
				TRACE_HEAD_nextoport;

				if (pkt->headbuf->type == IPORTTYPE)
					((IPORT*)pkt->headbuf)->destination = pkt->module;
				else if (pkt->headbuf->type == BUFFERTYPE)
					((BUFFER*)pkt->headbuf)->destination = pkt->module;

				oport->muxptr = pkt->muxptr;
				pkt->muxptr = NULL;

				if (SemaphoreValue(oport->freespace) > 0) {
					TRACE_HEAD_oportrdy;
					SemaphoreDecr(oport->freespace);
					EventReschedTime(flitdelay-delta, Head_Move);
				}
				else {
					TRACE_HEAD_oportwait;
					pkt->data.blktime = pkt->data.blktime - YS__Simtime;
					EventReschedSema(oport->freespace,Before_Head_Move);
				}
				return;
			}

			else YS__errmsg("Invalid network module type");
		}

		case Route_Mux: {
			mux = (MUX*)(pkt->module);
			demux_trace = (DEMUX*)(mux->nextmodule);
			/*pkt->data.route = (demux_trace->router)(&(pkt->data.srccpu),
			&(pkt->data.destcpu),demux_trace->id);*/
			pkt->data.rcstage++;
			EventReschedTime(ROUTE_DELAY, Virtual_Mux);
			return;
		}

		case Virtual_Mux: {
			mux = (MUX*)pkt->module;
			//printf("MUX %d %d\n", mux->type, mux->id);
			demux_trace = (DEMUX*)(mux->nextmodule);
			//printf("DEMUX %d %d\n", demux_trace->type, demux_trace->id);
			mux_trace = (MUX*)(*((demux_trace->nextmodule) + pkt->data.route));
			//printf("Radix issue at VMUx %d\n", mux_trace->id);
			buf_trace = (BUFFER*)(mux_trace->nextmodule);

			//x = fmod(YS__Simtime, flitdelay);
			//if( x == delta ) {
			//	EventReschedTime(delta, Virtual_Mux );
			//	return;
			//}

			pkt->data.vastage++;
			do {
				k = RandUniformInt(0, VC - 1 );
			}while(k > VC-1);
			k = 0;
			do {
				if( (buf_trace->vc_data.input_vc[k] == -1) ) {
					/* Make sure only 1 VC will get allocated per output port */
					if( SemaphoreValue(buf_trace->va_alloc) != 1 ) {
						EventReschedTime(VIRTUAL_ALLOC_DELAY, Virtual_Mux);
						return;
					}
					SemaphoreDecr(buf_trace->va_alloc);

					/* When you run out of VCs */
					if( SemaphoreValue(buf_trace->bufva_sema) > 0 ) {
						SemaphoreDecr(buf_trace->bufva_sema);
					}

					/* Setup all data on the output buffer */
					buf_trace->vc_data.input_vc[k] = pkt->data.vcindex;
					buf_trace->vc_data.input_seqno[k] = pkt->data.seqno;
					buf_trace->vc_data.input_port[k] = ((demux_trace->id)%(RADIX));
					*((buf_trace->vc_data.inputbuf)+k) = pkt->carrybuf;
					buf_trace->vc_data.keep_track++;
					if( buf_trace->vc_data.keep_track >= VC )
						buf_trace->vc_data.keep_track = 0;

					/* Setup all data on the input buffer */
					pkt->data.vcindex = k;

					newbuf = (BUFFER*)pkt->carrybuf;
					newbuf->output_vc = pkt->data.vcindex;
					newbuf->output_port = pkt->data.route;
					newbuf->output_seqno = pkt->data.seqno;
					newbuf->output_credit = buf_trace->vc_data.input_credit[k];
					newbuf->output_alloc = -1;
					newbuf->outputbuf = (MODULE*)buf_trace;

					//if( buf_trace->id == 29 )
					/*if( (buf_trace->id == 149) || (buf_trace->id == 171) )*/
					//printf("Allocating VC %d pkt %d outputbuf %d credit %d time %g inputbuf %d\n",
					//k, pkt->data.seqno, buf_trace->id, buf_trace->vc_data.input_credit[k],
					//YS__Simtime, newbuf->id );

					EventReschedTime(VIRTUAL_ALLOC_DELAY, Release_Virtual_Alloc);
					return;
				}
				else
					k++;
			}while( k < VC );

			//if( buf_trace->id == 29)
			//printf("Waiting at %d for VC Alloc %d seq %d\n", newbuf->id, buf_trace->id,
			//			pkt->data.seqno);
			EventReschedSema(buf_trace->bufva_sema, Virtual_Mux);
			return;
		}

		case Release_Virtual_Alloc: {
			mux = (MUX*)pkt->module;
			buf = (BUFFER*)pkt->carrybuf;

			demux_trace = (DEMUX*)(mux->nextmodule);
			mux_trace = (MUX*)(*((demux_trace->nextmodule) + pkt->data.route));
			buf_trace = (BUFFER*)(mux_trace->nextmodule);
			if( SemaphoreValue(buf_trace->va_alloc) != 1 ) {
				SemaphoreSignal(buf_trace->va_alloc);
				SemaphoreSet(buf_trace->va_alloc);
			}
			//EventReschedTime(SWITCH_ALLOC_DELAY, Credit_Mux);
			EventReschedTime(SWITCH_ALLOC_DELAY, Switch_Mux);
			return;
		}

		case Credit_Mux: {
			mux = (MUX*)pkt->module;
			buf = (BUFFER*)pkt->carrybuf;
			pkt->data.vcindex = buf->output_vc;
			pkt->data.route = buf->output_port;

			demux_trace = (DEMUX*)(mux->nextmodule);
			mux_trace = (MUX*)(*((demux_trace->nextmodule) + pkt->data.route));
			buf_trace = (BUFFER*)(mux_trace->nextmodule);

			ret = YS__Congestion(buf, buf_trace, pkt);
			if( ret != 0 ) {
				//printf("NO CREDIT buf %d pkt %d time %g\n",
				//	buf->id, pkt->data.seqno, YS__Simtime);
				EventReschedSema(buf->bufcredit_sema, Credit_After_Mux);
				return;
			}

			if( SemaphoreValue(buf->bufcredit_sema) > 0 )
				SemaphoreDecr(buf->bufcredit_sema);

			if( pkt->data.route < (RADIX-CONC) ) {
				buf->output_credit = buf->output_credit - 1;
				newbuf = ((BUFFER*)buf->outputbuf);
				newbuf->vc_data.input_credit[buf->output_vc] =
					newbuf->vc_data.input_credit[buf->output_vc] - 1;
				EventReschedTime(0.0, Switch_Mux);
				//printf("Consuming credit %d buf %d newbuf %d pkt seq %d time %g\n",
				//	newbuf->vc_data.input_credit[buf->output_vc], buf->id, newbuf->id, pkt->data.seqno, YS__Simtime);
				return;
			}
			else {
				EventReschedTime(0.0, Switch_Mux);
				return;
			}
		}

		/* This case is written simply for timing purposes and can be combined with previous */
		case Credit_After_Mux: {
			mux = (MUX*)pkt->module;
			buf = (BUFFER*)pkt->carrybuf;
			pkt->data.vcindex = buf->output_vc;
			pkt->data.route = buf->output_port;

			demux_trace = (DEMUX*)(mux->nextmodule);
			mux_trace = (MUX*)(*((demux_trace->nextmodule) + pkt->data.route));
			buf_trace = (BUFFER*)(mux_trace->nextmodule);

			x = fmod(YS__Simtime, flitdelay);
			if( x == delta ) {
				EventReschedTime( delta, Credit_After_Mux );
				return;
			}

			ret = YS__Congestion(buf, buf_trace, pkt);
			if( ret != 0 ) {
				EventReschedSema(buf->bufcredit_sema, Credit_After_Mux);
				return;
			}

			if( SemaphoreValue(buf->bufcredit_sema) > 0 )
				SemaphoreDecr(buf->bufcredit_sema);

			if( pkt->data.route < (RADIX-CONC) ) {
				buf->output_credit = buf->output_credit - 1;
				newbuf = (BUFFER*)buf->outputbuf;
				newbuf->vc_data.input_credit[buf->output_vc] =
					newbuf->vc_data.input_credit[buf->output_vc] - 1;
				EventReschedTime(0.0, Switch_Mux);
				return;
			}
			else {
				EventReschedTime(0.0, Switch_Mux);
				return;
			}

		}

		case Switch_Mux: {
			mux = (MUX*)pkt->module;
			pkt->data.sastage++;
			if (SemaphoreValue(mux->arbsema) > 0) {
				SemaphoreDecr(mux->arbsema);
	            mux->muxptr = pkt->muxptr;
            	pkt->muxptr = mux;

            	TRACE_HEAD_muxfree;

            	EventReschedTime(muxdelay, Next_Module);
            }
            else {
				TRACE_HEAD_muxblocked;
				EventReschedSema(mux->arbsema, Wakeup_Mux);
				pkt->data.blktime = pkt->data.blktime - YS__Simtime;
			}
			return;
		}

		case Wakeup_Mux: {
			mux = (MUX*)pkt->module;
			//printf("In Wake Mux %d at time %g pkt %d Sema %d\n",
			//mux->id, YS__Simtime, pkt->data.seqno, pkt->data.pkttype );

			TRACE_HEAD_muxwakeup;

			mux->muxptr = pkt->muxptr;
			pkt->muxptr = mux;

			/*pkt->data.powers = ARBITRATION_POWER + pkt->data.powers;*/

			EventReschedTime(delta+muxdelay,Next_Module);
			pkt->data.blktime = pkt->data.blktime + YS__Simtime + delta;
			return;
		}

		case Before_Head_Move: {
			pkt->data.blktime = pkt->data.blktime + YS__Simtime + delta - flitdelay;
		}

		case Head_Move: {
			buf = (BUFFER*)pkt->headbuf;

			// Possible cover for the problem but does not actually figure out why it happens
			if (buf->type == OPORTTYPE) {
            	EventSetDelFlag();
				printf("OPORT should not be here: pkt_seqnum %d\n", pkt->data.seqno);
            	return;
            }

			//printf("In head move?\n");
			if (buf->type == IPORTTYPE) {
				TRACE_HEAD_fromiport;
				pkt->tailoffset = pkt->data.pktsize - 2;
			}
			else {
				TRACE_HEAD_frombuf;
				YS__BufferHeadGet(buf);
			}

			if (pkt->SleepingTail != NULL) {
				TRACE_TAIL_wakes;
				ActivitySchedTime(pkt->SleepingTail, delta, INDEPENDENT);
				pkt->SleepingTail = NULL;
			}

			if (pkt->module->type == OPORTTYPE) {
				//printf("At opoort\n");
				TRACE_HEAD_tooport;
            	pkt->headev = NULL;
            	pkt->headbuf = pkt->module;
            	EventSetDelFlag();

            	return;
            }

            TRACE_HEAD_tobuf;
            athead = YS__BufferHeadPut(pkt->module,pkt);

            if (athead) {
				TRACE_HEAD_athead;
				EventReschedTime(delta,Next_Module);

				/*if(pkt->data.seqno == 203)
				{
					printf("IM THE PACKET (203), Event_id = %d",  pkt->headev->id); //, Event_name = %s\n", pkt->headev->id, pkt->headev->name);
					if(GetSimTime() > 35)
						YS__EventListPrint();
				}*/

				return;
			}
			else {
				TRACE_HEAD_notathead;
				pkt->data.blktime = pkt->data.blktime - (YS__Simtime + delta);
				EventSetState(ME, Before_Next_Module);
				return;
			}
		}
	}
}

/**************************************************************************************/

void YS__TailEvent()     /* This event controls the movement of tail flits     */

/* Each time a packet is sent into a network with PacketSend(), an event with this    */
/* body is created and scheduled.  It is responsible for moving the tail flit through */
/* the network following the path established by the head flit.  It also manages the  */
/* the distribution of the flits between the head and tail flits.  A pointer to the   */
/* flit's packet structure is passed to the event as a its argument.  The event's     */
/* argsize is set to TAILTYPE to indicate its type                                    */

{
   PACKET *pkt;
   MODULE *curmod;
   MODULE *nxtmod;
   BUFFER *buf;
   OPORT  *oport;
   int    moveflag;

   pkt = (PACKET*)ActivityGetArg(ME);      /* Get a pointer to the tail's packet      */

   switch(EventGetState()) {               /* Resume at last point of suspension      */

      case Tail_Start: label_Tail_Start:

         moveflag = YS__ShiftBubbles(pkt);    /* Adjust bubbles in packet             */

         if (moveflag == 2 &&                 /* No bubbles between tail & head flits */
             pkt->headev != NULL)             /* Head flit still in a buffer          */
         {
            TRACE_TAIL_nobubbles;             /* Tail of pkt has caught up with head  */
             EventSetState(NULL, Tail_Start); /* Tail sleeps until head moves         */
             pkt->SleepingTail =              /* Pointer used by head to wake up tail */
                (EVENT*)(ActivityGetMyPtr()); /*    when it moves                     */
             return;
         }

         if (moveflag == 0) {                   /* Tail doesn't move                  */
            TRACE_TAIL_nomove;
            EventReschedTime(flitdelay,Tail_Start);
         }
         else if (moveflag == -1) {             /* Tail moves within its buffer       */
            TRACE_TAIL_shift;
            EventReschedTime(flitdelay-delta,Tail_Shift);
         }
         else if (moveflag == 1) {              /* Tail moves between buffers         */
            TRACE_TAIL_move;
            EventReschedTime(flitdelay-delta,Tail_Move);
         }
         else YS__errmsg("Incorrect return value from ShiftBubbles");

         return;

      case Tail_Shift:

         curmod = pkt->tailbuf;                /* Module that tail is in              */
         if (curmod->type == BUFFERTYPE) {     /* Tail in a buffer                    */
            buf = (BUFFER*)curmod;             /* Cast to type BUFFER                 */
            buf->free++;                       /* One more free flit slot in buf      */
            YS__BufferNewSpace(buf);           /* Chk for waiting head or tail        */
         }
         EventReschedTime(delta,Tail_Start);
         return;

      case Tail_Move:

         curmod = pkt->tailbuf;                   /* Module that tail is in           */
         if (curmod->type == IPORTTYPE)           /* Tail in an input port            */
            nxtmod = ((IPORT*)pkt->tailbuf)->destination;  /* Cast module to IPORT    */
         else                                     /* Tail in a buffer                 */
            nxtmod = ((BUFFER*)pkt->tailbuf)->destination; /* Cast module to BUF      */

         /* Nxtmod now points to the next buffer or oport the tail will encounter     */
         if (curmod->type == BUFFERTYPE && nxtmod->type == BUFFERTYPE) {
            TRACE_TAIL_buftobuf;             /* Tail moves from buf to buf            */
            YS__BufferTailGet(curmod);       /* Take tail out of its buffer           */
            YS__BufferTailPut(nxtmod,pkt);   /* & put it in the next buffer           */
            YS__SignalMux(nxtmod);           /* Tail passed, free muxes               */
            EventReschedTime(delta,Tail_Start);
            return;                          /* Delay for flit delay                  */
         }

         if (curmod->type == BUFFERTYPE && nxtmod->type == OPORTTYPE) {
            TRACE_TAIL_buftoport;            /* Tail moves from buf to oport          */
            YS__BufferTailGet(curmod);       /* Take tail out of its buffer           */
            YS__SignalMux(nxtmod);           /* Tail passed, free muxes               */
            pkt->tailbuf = nxtmod;           /* Tail in an oport now                  */
            EventReschedTime(delta,Tail_Done);
            return;                          /* Delay for flit delay                  */
         }

         if (curmod->type == IPORTTYPE && nxtmod->type == BUFFERTYPE) {

            if (pkt->data.mesgptr) {            /* This packet associated with a mesg */
               pkt->data.mesgptr->pktosend--;   /* One more packet of message sent    */
               if ( pkt->data.mesgptr->pktosend == 0 &&  /* This was the last packet  */
                  pkt->data.mesgptr->blockflag == BLOCK_UNTIL_SENT) /* Sender blocked */
                     ActivitySchedTime(pkt->data.mesgptr->source,   /* Wake up sender */
                                       delta,INDEPENDENT);
            }

            TRACE_TAIL_porttobuf;            /* Tail moves from iport to buf          */
            YS__BufferTailPut(nxtmod,pkt);   /* Put tail in next buffer               */
            YS__SignalMux(nxtmod);           /* Tail passed, free muxes               */
            EventReschedTime(delta, Signal_Port);
            pkt->lastiport->qfree++;         /* One fewer packet in the port          */
            return;
         }

         if (curmod->type == IPORTTYPE && nxtmod->type == OPORTTYPE) {

            if (pkt->data.mesgptr) {            /* This packet associated with a mesg */
               pkt->data.mesgptr->pktosend--;   /* One more packet of message sent    */
               if ( pkt->data.mesgptr->pktosend == 0 &&  /* This was the last packet  */
                  pkt->data.mesgptr->blockflag == BLOCK_UNTIL_SENT) /* Sender blocked */
                     ActivitySchedTime(pkt->data.mesgptr->source,   /* Wake up sender */
                                       delta,INDEPENDENT);
            }

            TRACE_TAIL_porttoport;           /* Tail moves from iport to oport        */
            YS__SignalMux(nxtmod);           /* Tail passed, free muxes               */
            pkt->tailbuf = nxtmod;           /* Tail in an oport now                  */
            EventReschedTime(delta,Tail_Done);
            return;
         }

      case Signal_Port: /* Tail leaves an iport & enters a buffer                     */

         SemaphoreSignal(pkt->lastiport->netrdy);   /* Net ready for more pkts        */
         SemaphoreSignal(pkt->lastiport->portrdy);  /* One more free packet position  */
         pkt->lastiport = NULL;                     /* Tail no longer in an iport     */

         goto label_Tail_Start;

      case Tail_Done: /* Tail enters oport and leaves the network                     */
         if (pkt->lastiport != NULL) {    /* Tail in an iport                         */
            pkt->lastiport->qfree++;      /* One fewer packet in the port             */
            SemaphoreSignal(pkt->lastiport->netrdy);  /* Net ready for more packets   */
            SemaphoreSignal(pkt->lastiport->portrdy); /* One more free pkt position   */
         }

         oport = (OPORT*)(pkt->tailbuf);   /* Get a pointer to the oport              */

         if (oport->qhead == NULL) {       /* Insert packet input an empty port queue */
            oport->qhead = pkt;
            oport->qtail = pkt;
            pkt->next = NULL;
         }
         else {                            /* Append to tail of a nonempty port queue */
            oport->qtail->next = pkt;
            oport->qtail = pkt;
            pkt->next = NULL;
         }

         TRACE_TAIL_done;                  /* Packet queued at output port            */
         oport->count++;                   /* count = # of packets available in port  */

         pkt->data.oporttime =             /* Packet enters the output port, so start */
            pkt->data.oporttime - YS__Simtime; /* counting time in oport              */

         SemaphoreSignal(oport->pktavail); /* Releases  activities waiting on packets */

         EventSetDelFlag();                /* Tail event dies                         */
         return;
   }
}

/**************************************************************************************/

void YS__SignalMux(modptr)          /* Signals muxes that tail passes through  */
MODULE *modptr;                            /* Ptr to buffer or oport containing head  */
{
   MUX *mxptr;
   MUX *mxtemp;

   if (modptr->type == BUFFERTYPE) {       /* Module is a buffer                      */
      mxptr = ((BUFFER*)(modptr))->muxptr; /* Cast module pointer and get pointer to  */
      ((BUFFER*)(modptr))->muxptr = NULL;  /*    preceeding mux, if any               */
   }

   else  {                                 /* Module is an oport                      */
      mxptr = ((OPORT*)(modptr))->muxptr;  /* Cast module pointer and get pointer to  */
      ((OPORT*)(modptr))->muxptr = NULL;   /*    preceeding mux, if any               */
   }
   while (mxptr != NULL) {                 /* Follow list of preceeding muxes         */
      TRACE_TAIL_signalmux;                /* Packet signals mux semaphore            */
      SemaphoreSignal(mxptr->arbsema);     /* Signal this mux                         */
      mxtemp = mxptr;                      /* Remember this mux                       */
      mxptr = mxptr->muxptr;               /* Get the next mux in the list            */
      mxtemp->muxptr = NULL;               /* Clear the previous mux pointer          */
   }
}

/**************************************************************************************/

int YS__ShiftBubbles(pkt)  /* Manages flits between a packet's the head & tail */
PACKET *pkt;                      /* Pointer to the packet                            */
{
   MODULE *curmod;                /* Pointer to the mocule the current                */
   MODULE *nxtmod;                /* Pointer to the module that follows it            */
   BUFFER *buf;                   /* Curmod cast to type buffer                       */
   int moveflit;                  /* Flag to indicate a flit is to switch buffers     */
   int nextfree;                  /* # of free slots in the next buffer               */
   int retval;                    /* -1: tail shifts, 1: tail moves, 0: tail ~ move   */
   int zbub = 1;                  /* Bubble count was zero when routine started       */

   if (pkt->headbuf == pkt->tailbuf)  /* Head and tail in same buffer,                */
      return 2;                       /* no bubbles between them to shift             */

   /* Since head and tail are in different buffers, there can be no other head or     */
   /* flits ahead of the tail in its buffer.  There may be internal flits ahead of it */

   curmod = pkt->tailbuf;             /* Get a pointer to the tail flits buffer       */
   buf = (BUFFER*)curmod;             /* Cast it to type BUFFER                       */

   if (pkt->tailbuf->type == IPORTTYPE)              /* Tail still in an iport        */
      nxtmod = ((IPORT*)pkt->tailbuf)->destination;  /* Get ptr to the next module    */
   else                                              /* Tail is in a buffer           */
      nxtmod = ((BUFFER*)pkt->tailbuf)->destination; /* Get ptr to the next module    */

   if (nxtmod->type == BUFFERTYPE)                   /* Next module is a buffer       */
      nextfree = ((BUFFER*)nxtmod)->free;            /* # free slots in next buffer   */
   else                                              /* Next module is an oport       */
      nextfree = -1;                                 /* Oports can hold a whole pkt   */

   if (nextfree != 0) {          /* There is space for in the next buffer             */
      if (pkt->tailoffset > 0) { /* There is an internal flit ahead of tail           */
         retval = -1;            /* Tail shifts within its buffer                     */
         zbub = 0;               /* Bubble count was not zero at start                */
         moveflit = 1;           /* Tells the next buffer that a flit has moved in    */
         pkt->tailoffset--;      /* All flits shift, moving tail closer to front      */
      }
      else {                     /* Tail at front of its buffer, no flits ahead of it */
         retval = 1;             /* Tail moves between buffers                        */
         zbub = 0;               /* Bubble count was not zero at start                */
         moveflit = 0;           /* Notify the next buffer not to expect a flit       */
      }
   }
   else {                        /* No flits can move out of this buffer              */
      retval = 0;                /* Tail doesn't move                                 */
      moveflit = 0;              /* Notify the next buffer not to expect a flit       */
   }                             /*   because flit will be moved later by TailPut     */

   curmod = nxtmod;            /* Look at the next buffer between head and tail       */
   buf = (BUFFER*)curmod;      /* Cast it to type BUFFER                              */

   while (curmod != pkt->headbuf) {  /* Follow list of buffers between head & tail    */

      nxtmod = ((BUFFER*)curmod)->destination; /* Get a ptr to the next module        */
      if (nxtmod->type == BUFFERTYPE)          /* Next module is a buffer             */
         nextfree = ((BUFFER*)nxtmod)->free;   /* # free slots in next buffer         */
      else                                     /* Nest module is an oport             */
         nextfree = -1;                        /* Oports can hold a whole pkt         */
      buf = (BUFFER*)curmod;                   /* Cast module to type BUFFER          */

      if (buf->free < buf->size &&      /* There is a flit in this buffer             */
          nextfree != 0)                /*    & space for it in the next buffer       */
      {
         zbub = 0;                      /* Bubble count was not zero at start         */
         if (!moveflit) {               /* No flit moved in from previous buffer      */
            buf->free++;                /* Therefore one free slot created            */
         }
         /* Else a flit moves in and one moves out leaving free and bubbles unchanged */
         moveflit = 1;                  /* Tells next buffer that it got a flit       */
      }
      else {                            /* No flits can move out of this buffer       */
         if (moveflit) {                /* But one moved in                           */
            zbub = 0;                   /* Bubble count was not zero at start         */
            buf->free--;                /* Therefore, one fewer flit slot             */
         }
         moveflit = 0;                  /* Notify next buffer not to expect a flit    */
      }
      curmod = nxtmod;                  /* Get a pointer to the next module & repeat  */
      buf = (BUFFER*)curmod;            /* Cast it to type BUFFER                     */
   }

   /* We have reached the head flit's buffer */

   if (curmod->type != OPORTTYPE) {     /* Head's buffer is really a buffer           */
      if (moveflit) {                   /*   and a flit moved into it                 */
         buf->free--;                   /* Therefore, one fewer free flit slot        */
      }
   }
   if (zbub) return 2;                  /* Tail has caught up with its head           */
   else return retval;
}

/**************************************************************************************\
******************************** Statistics Operations *********************************
****************************************************************************************
**                                                                                    **
**      The following statistics can be automatically collected on the states of      **
**      the packets as they move through a network:                                   **
**           - Total time spent in the network (packet latency)                       **
**           - Time blocked by multiplexers or full buffers                           **
**           - Time in an output port waiting to be received                          **
**           - Total time that at least one flit was moving in the network            **
**      All of theses statistics are collected in YACSIM statistics records, and      **
**      therefore, all statrec statistics such as means and histograms are            **
**      avaiable.
**
****************************************************************************************
\**************************************************************************************/

void NetworkCollectStats(type,histflg,nbin,low,high)
                /* Activates automatic statistics collection for the network          */
int type;       /* NETTIME, BLKTIME, OPORTTIME, MOVETIME                              */
int histflg;    /* HIST or NOHIST                                                     */
int nbin;       /* Number of histogram bins                                           */
double low;     /* Max value of the low bin                                           */
double high;    /* Min value of the high bin                                          */
{
   if (type == NETTIME) {           /* Collect total time in net stats                */
      if (nettimestat == NULL) {    /* Stat collection not yet inititated             */
         nettimestat =              /* Get a new statrec                              */
            NewStatrec("NetTimeStats",POINT,MEANS,histflg,nbin,low,high);
      }
      else YS__warnmsg("Time in net statistics collection already set");
   }
   else if (type == BLKTIME) {      /* Collect time packet blocked stats              */
      if (blktimestat == NULL) {    /* Stat collection not yet inititated             */
         blktimestat =              /* Get a new statrec                              */
            NewStatrec("BlkTimeStats",POINT,MEANS,histflg,nbin,low,high);
      }
      else YS__warnmsg("Time blocked statistics collection already set");
   }
   else if (type == OPORTTIME)   {  /* Collect time waiting in oport stats            */
      if (oporttimestat == NULL) {  /* Stat collection not yet inititated             */
         oporttimestat =            /* Get a new statrec                              */
            NewStatrec("OPortTimeStats",POINT,MEANS,histflg,nbin,low,high);
      }
      else YS__warnmsg("Time in net statistics collection already set");
   }
   else if (type == MOVETIME) {     /* Collect time packet in motion stats            */
      if (movetimestat == NULL) {   /* Stat collection not yet inititated             */
         movetimestat =             /* Get a new statrec                              */
            NewStatrec("MoveTimeStats",POINT,MEANS,histflg,nbin,low,high);
      }
      else YS__warnmsg("Time packet moving statistics collection already set");
   }
   else if (type == LIFETIME) {     /* Collect packet lifetime stats                  */
      if (lifetimestat == NULL) {   /* Stat collection not yet inititated             */
         lifetimestat =             /* Get a new statrec                              */
            NewStatrec("MoveTimeStats",POINT,MEANS,histflg,nbin,low,high);
      }
      else YS__warnmsg("Time packet moving statistics collection already set");
   }
   else YS__errmsg("Invalid statistic type for networks, can't collect");
}

/**************************************************************************************/

void NetworkResetStats()        /* Resets all network statistics records              */
{
   if (nettimestat != NULL)     /* Collecting time in net statistics                  */
      StatrecReset(nettimestat);
   if (blktimestat != NULL)     /* Collecting time blocked statistics                 */
      StatrecReset(blktimestat);
   if (oporttimestat != NULL)   /* Collecting waiting in port statistics              */
      StatrecReset(oporttimestat);
   if (movetimestat != NULL)     /* Collecting time moving statistics                 */
      StatrecReset(movetimestat);
   if (lifetimestat != NULL)     /* Collecting packet lifetime statistics             */
      StatrecReset(lifetimestat);
}

/**************************************************************************************/

STATREC *NetworkStatPtr(type)    /* Returns a pointer to a network statistics record  */
int type;                        /* NETTIME, BLKTIME, OPORTTIME, MOVETIME             */
{
   if (type == NETTIME) return nettimestat;
   else if (type == BLKTIME) return blktimestat;
   else if (type == OPORTTIME) return oporttimestat;
   else if (type == MOVETIME) return movetimestat;
   else if (type == LIFETIME) return lifetimestat;
   else YS__errmsg("Invalid statistic type for networks");
}

/**************************************************************************************/

void NetworkStatRept()        /* Prints a report of network statistics                */
{
   TracePrintTag("statrpt","\nNETWORK STATISTICS REPORT:\n\n");
   if (nettimestat != NULL) {
      TracePrintTag("statrpt",
         "    Network throughput = %g\n",StatrecRate(nettimestat));
      TracePrintTag("statrpt",
         "    Average packet latency = %g\n",StatrecMean(nettimestat));
   }
   if (movetimestat != NULL) {
      TracePrintTag("statrpt","    Ideal latencey (no conflicts) = %g\n",
             StatrecMean(movetimestat));
   }
   if (movetimestat != NULL && nettimestat != NULL)
      TracePrintTag("statrpt","    Normalized latency = %g\n",
             StatrecMean(movetimestat)/StatrecMean(nettimestat));
   if (blktimestat != NULL) {
      TracePrintTag("statrpt","    Average time packets blocked in the network = %g\n",
             StatrecMean(blktimestat));
   }
   if (oporttimestat != NULL) {
      TracePrintTag("statrpt","    Average time packets wait to be received = %g\n",
             StatrecMean(oporttimestat));
   }
   if (lifetimestat != NULL) {
      TracePrintTag("statrpt","    Average packet lifetime = %g\n",
             StatrecMean(lifetimestat));
   }
   TracePrintTag("statrpt","\n");
}

/**************************************************************************************/

/* Look Ahead Router connects to the Output Buffer of the previous buffer */
void DemuxCreditBuffer(demux, buf)
BUFFER *buf;
DEMUX *demux;
{
	demux->prevmodule = (MODULE*)buf;
	return;
}

/* Input Buffer connects to the previous Demux (look ahead router) to trace previous buffer */
void BufferCreditDemux(buf,demux)
BUFFER *buf;
DEMUX *demux;
{
	buf->prevmodule = (MODULE*)demux;
	return;
}

/**************************************************************************************/

int GetLinkUtil(demux)
DEMUX *demux;
{
	return demux->util;
}

/**************************************************************************************/
int YS__Congestion(ibuf, obuf, pkt)
PACKET *pkt;
BUFFER *ibuf;
BUFFER *obuf;
{
	BUFFER *newbuf;
	int k, vc, cong,i ;

	if( ibuf->output_credit == 0 )
		return 1;

	if( ibuf->output_credit > 0 ) {
		if( SemaphoreWaiting(ibuf->bufcredit_sema) > 0 ) {
			SemaphoreSignal(ibuf->bufcredit_sema);
			SemaphoreSet(ibuf->bufcredit_sema);
		}
		return 0;
	}
}

/**************************************************************************************/
int CongestionCounter(obuf)
BUFFER *obuf;
{
	return obuf->vc_data.congestion;
}

/**************************************************************************************/
double PowerReturn(oport, type)
OPORT *oport;
int		type;
{

	switch(type) {

		case 0:
				return oport->power;
				break;

		case 1:
				return oport->input_buffer;
				break;

		case 2:
				return oport->crossbar;
				break;

		case 3:
				return oport->link;
				break;

		case 4:
				return oport->route;
				break;

		case 5:
				return oport->vc;
				break;

		case 6:
				return oport->swtch;
				break;

		default:
				YS__errmsg("Error\n");
	}

}

/**************************************************************************************/
void YS__IBVirtualChannelRelease(inbuf, pkt)
BUFFER *inbuf;
PACKET *pkt;
{
	BUFFER *outbuf;
	int k;

	/* Here write the code for releasing Virtual channel allocation at the input buffer */
	/* Clear the input buffer and then the output buffer for VC */
	/* Release any packet waiting for VC */

	if( inbuf->buftype == INPUT_BUFFER ) {
		if( pkt->data.pkttype == (pkt->data.packetsize - 1)/FLITSZ ) {
			outbuf = ((BUFFER*)inbuf->outputbuf);
			k = 0;
			do {
				if( outbuf->vc_data.input_seqno[k] == pkt->data.seqno ) {
					if( inbuf->output_seqno != pkt->data.seqno ) {
						printf("op buf %d id %d ip buf %d id %d Incorrect seqnos %d %d type %d\n",
							outbuf, outbuf->id, inbuf, inbuf->id, inbuf->output_seqno, pkt->data.seqno, pkt->data.pkttype);
						YS__errmsg("Incorrect seqnos\n");
					}

					inbuf->output_vc = -1;
					inbuf->output_port = -1;
					inbuf->output_seqno = -1;
					inbuf->output_credit = -1;
					inbuf->output_alloc = -1;

					outbuf->vc_data.input_vc[k] = -1;
					outbuf->vc_data.input_port[k] = -1;
					outbuf->vc_data.input_seqno[k] = -1;
					*((outbuf->vc_data.inputbuf)+k) = NULL;

					if( SemaphoreWaiting(outbuf->bufva_sema) > 0 )
						SemaphoreSignal(outbuf->bufva_sema);
					else
						SemaphoreSet(outbuf->bufva_sema);
					break;
				}
				else
					k++;
			}while( k < VC );

			//printf("NO Releasing VC seqno %d id %d ip buf %d id %d seqnos %d %d type %d\n",
			//	outbuf->vc_data.input_seqno[k], outbuf->id, inbuf, inbuf->id,
			//	inbuf->output_seqno, pkt->data.seqno, pkt->data.pkttype);

		}
	}
}

/**************************************************************************************/
void YS__IBCreditFlowControl(buf, pkt)
BUFFER *buf;
PACKET *pkt;
{
	BUFFER *newbuf, *input_prevbuf, *output_prevbuf;
	DEMUX *newdemux;
	int prev_vcindex, i,k, temp;

	if( buf->buftype == INPUT_BUFFER ) {
		//printf("1 About to release credit %d tyep %d\n", buf->id, buf->buftype );
		newdemux = (DEMUX*)buf->prevmodule;
		//printf("2 Checking for matching demux %d\n", newdemux->id);
   		if( newdemux->prevmodule != NULL ) {
   			output_prevbuf = (BUFFER*)newdemux->prevmodule;
   			prev_vcindex = ((buf->id)%(VC+1));
   			//printf("3 Identifying previous vcindex %d %d \n", prev_vcindex, buf->id );

   			//printf("4 Returning credit op buf %d Vcindex %d credit %d time %g\n",
			//   	output_prevbuf->id, prev_vcindex, output_prevbuf->vc_data.input_credit[prev_vcindex],
			//   	YS__Simtime);

   			if( output_prevbuf->vc_data.input_credit[prev_vcindex] < CREDITS ) {
   				output_prevbuf->vc_data.input_credit[prev_vcindex] =
   					output_prevbuf->vc_data.input_credit[prev_vcindex] + 1;
			}

   			if( output_prevbuf->vc_data.input_seqno[prev_vcindex] != -1 ) {
   				input_prevbuf = (BUFFER*)*((output_prevbuf->vc_data.inputbuf) + (prev_vcindex));
   				if( input_prevbuf->output_credit < CREDITS )
   					input_prevbuf->output_credit = input_prevbuf->output_credit + 1;

				if( SemaphoreWaiting(input_prevbuf->bufcredit_sema) > 0 ){
   					SemaphoreSignal(input_prevbuf->bufcredit_sema);
   					SemaphoreSet(input_prevbuf->bufcredit_sema);
				}
   				else
   					SemaphoreSet(input_prevbuf->bufcredit_sema);
   			}
   		}
   		//else {
			//printf("No match\n");
		//}
	}
}
