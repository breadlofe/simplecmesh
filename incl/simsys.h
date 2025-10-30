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

#ifndef SIMH
#define SIMH

#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include "dbsim.h"

/**************************************************************************************\
****************************************************************************************
**                                                                                    **
**                                     CONSTANTS                                      **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/


/**************************************************************************************\
*                    Internal constants (Not needed by the user)                       *
\**************************************************************************************/

#define debug           /* Causes trace statments to be included in the source code   */
#define MAXDBLEVEL 5    /* The number of trace levels                                 */

/*********************************  Types of Objects  *********************************/

#define UNTYPED                  0

/* YACSIM Objects */

#define QETYPE                1000      /* Queue elements                             */
#define   QUETYPE             1100      /* Queues                                     */
#define     SYNCQTYPE         1110      /* Synchronization Queueus                    */
#define       SEMTYPE         1111      /* Semaphores                                 */
#define       FLGTYPE         1112      /* Flages                                     */
#define       BARTYPE         1113      /* Barriers                                   */
#define       CONDTYPE        1114      /* Conditions                                 */
#define     RESTYPE           1120      /* Resources                                  */
#define   STATQETYPE          1200      /* Statistical Queue Elements                 */
#define     ACTTYPE           1210      /* Activities                                 */
#define       PROCTYPE        1211      /* Processes                                  */
#define       EVTYPE          1212      /* Events                                     */
#define     MSGTYPE           1220      /* Messages                                   */
#define   STVARTYPE           1300      /* State Variables                            */
#define     IVARTYPE          1310      /* Integer Valued State Variable              */
#define     FVARTYPE          1320      /* Real Valued State Variable                 */
#define   STRECTYPE           1400      /* Statistics Records                         */
#define     PNTSTATTYPE       1410      /* Point Statistics Records                   */
#define     INTSTATTYPE       1420      /* Interval STatistics Records                */

/* NETSIM Objects */

#define BUFFERTYPE            2001      /* Netsim Buffers                             */
#define MUXTYPE               2002      /* Netsim Multiplexers                        */
#define DEMUXTYPE             2003      /* Netsim Demultiplexers                      */
#define IPORTTYPE             2004      /* Netsim Input Ports                         */
#define OPORTTYPE             2005      /* Netsim Output Ports                        */
#define DUPLEXMODTYPE         2006      /* Netsim Duplex Module                       */
#define HEADTYPE              2007      /* Head Flits                                 */
#define TAILTYPE              2008      /* Tail Flits                                 */

/* PARCSIM Objects */

#define PROCRTYPE             3001      /* Processors                                 */
#define USRPRTYPE             3002      /* User Processes                             */
#define OSPRTYPE              3003      /* OS Processes                               */
#define OSEVTYPE              3004      /* OS Events                                  */

/*********************************  Activity States  **********************************/

#define LIMBO                    0      /* Just created                               */
#define READY                    1      /* In the ready list                          */
#define DELAYED                  2      /* In the event list                          */
#define WAIT_SEM                 3      /* In a semaphore's queue                     */
#define WAIT_FLG                 4      /* In a flag's queue                          */
#define WAIT_CON                 5      /* In a conditions                            */
#define WAIT_RES                 6      /* In a resorce's queue                       */
#define USING_RES                7      /* In the event getting resource service      */

/******************************  Event & Process States  ******************************/

#define RUNNING                  8      /* Executing the body function                */

/**********************************  Process States  **********************************/

#define BLOCKED                  9      /* Due to a blocking schedule                 */
#define WAIT_JOIN               10      /* Due to a ProcessJoin                       */
#define WAIT_MSG                11      /* Due to a blocking ReceiveMsg               */
#define WAIT_BAR                12      /* Due to a BarrierSync()                     */

/*********************************  Processor States  *********************************/

#define IDLE                     0      /* Processor Ready List empty                 */
#define BUSY                     1      /* Processro Ready List not empty             */

/********************************* Resource Statuses **********************************/

#define QUIESCENT                0      /* No unevaluated state variable changes      */
#define PENDING                  1      /* Unevaluated state variable changes         */

/***********************************  Flag States  ************************************/

#define CLEARED                 18      /* Activites wait                             */
#define SET                     19      /* Activites continue                         */

/*********************************** Delay Constants  *********************************/

#define NUMDELAYS                6
#define MSG_ROUTING_DELAY        0
#define MSG_BUFOUT_DELAY         1
#define MSG_PACKETIZE_DELAY      2
#define MSG_PASSTHRU_DELAY       3
#define MSG_BUFIN_DELAY          4
#define MSG_DELIVER_DELAY        5

#define NET_FLITMOVE_DELAY       0
#define NET_MUXARB_DELAY         1
#define NET_MUXMOVE_DELAY        2
#define NET_DEMUX_DELAY          3

/**************************** Netsim Statistics Constants  ****************************/

#define NETTIME                  1
#define BLKTIME                  2
#define OPORTTIME                3
#define MOVETIME                 4
#define LIFETIME                 5

/*********************************** Miscellaneous  ***********************************/

#define POOLBLKSZ               10
#define DEFAULTHIST             64
#define CALQSZ                1024      /* Should be a power of 2                     */

#define MAXFANIN                11
#define MAXFANOUT               11
#define MSGACK                  -2
#define PROCESSORDEST            2
#define RDYLIST                  0      /* RRPRWP resource used as a cpu's ready list */

#ifndef FALSE
#define FALSE                    0
#endif
#ifndef TRUE
#define TRUE                     1
#endif

/**************************************************************************************\
*                          Externally visible constants                                *
\**************************************************************************************/

/********************************** Blocking Actions **********************************/

#define INDEPENDENT              0      /* Unrelated child scheduled                  */
#define NOBLOCK                  0      /* Nonblocking MessageReceive                 */
#define BLOCK                    1      /* Blocking Schedule or Message Receive       */
#define FORK                     2      /* Forking Schedule                           */

/*******************************  Event Characteristics  ******************************/

#define DELETE                   1      /* Event deleted at termination               */
#define NODELETE                 0      /* Event not deleted at termination           */

/******************************  Message Parameters  **********************************/

#define ANYTYPE                 -1      /* Receiving process ignores message type     */
#define ANYSENDER                0      /* Recieving process ignores message sender   */

#define ANYSENDERPROCESSOR      -1      /* Receiving processor ignores mesage sender  */
#define PROCESSDEST              1      /* Message sent to a process                  */
#define PROCESSORDEST            2      /* Message sent to a processor                */
#define BLOCK_UNTIL_RECEIVED     1
#define BLOCK_UNTIL_SENT         2
#define BLOCK_UNTIL_PACKETIZED   3
#define BLOCK_UNTIL_BUFFERED     4
#define DEFAULT_PKT_SZ          64

/***********************  Statistics Record Characteristics  **************************/

#define NOMEANS                  0      /* Means not computed                         */
#define MEANS                    1      /* Means are computed                         */
#define HIST                     2      /* Histogram is collected                     */
#define NOHIST                   3      /* Histogram not collected                    */
#define POINT                    4      /* Point statistics record type               */
#define INTERVAL                 5      /* Interval statistics record type            */

/*****************************  Argument and Buffer Size  *****************************/

#define UNKNOWN                 -1      /* Size not specified                         */
#define DEFAULTSTK               0      /* Default stack size                         */

/****************************  Types of Queue Disciplines  ****************************/

#define FCFS                     1      /* First Come First Served                    */
#define FCFSPRWP                 2      /* FCFS Preemptive Resume with Priorities     */
#define LCFSPR                   3      /* Last Come First Served Preemptive Resume   */
#define PROCSHAR                 4      /* Processor Sharing                          */
#define RR                       5      /* Round Robin                                */
#define RAND                     6      /* Random                                     */
#define LCFSPRWP                 7      /* LCFS Preemptive Resume with Priorities     */
#define SJN                      8      /* Shortest Job Next                          */
#define RRPRWP                   9      /* RR Preemptive Resume with Priorities       */
#define LCFS                    10      /* Last Come First Served                     */

/**********************************  Queue Statistics  ********************************/

#define TIME                     1
#define UTIL                     2
#define LENGTH                   3
#define BINS                     4
#define BINWIDTH                 5
#define EMPTYBINS                6

/***********************************  Event List Types  *******************************/

#define CALQUE                   0
#define LINQUE                   1

/*********************************  Netsim Routing modes  *****************************/

#define NOWAIT                   0
#define WAIT                     1

/***************************************  Misc.  **************************************/

#define ME                       0
#define GLOBAL                   0
#define LOCAL                    1
#define ALL                      0
#define BOTH                     2


/**************************************************************************************\
****************************************************************************************
**                                                                                    **
**                                   MACROS                                           **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

#define PSDELAY


/**************************************************************************************\
****************************************************************************************
**                                                                                    **
**                                  TYPEDEFS                                          **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

typedef struct YS__Qelem      QELEM;
typedef struct YS__Pool       POOL;
typedef struct YS__Qe         QE;
typedef struct YS__Queue      QUEUE;
typedef struct YS__SyncQue    SYNCQUE;
typedef struct YS__Sema       SEMAPHORE;
typedef struct YS__Flag       FLAG;
typedef struct YS__Barr       BARRIER;
typedef struct YS__Cond       CONDITION;
typedef struct YS__Resc       RESOURCE;
typedef struct YS__StatQe     STATQE;
typedef struct YS__Act        ACTIVITY;
typedef struct YS__Event      EVENT;
typedef struct YS__Mess       MESSAGE;
typedef struct YS__Stvar      STVAR;
typedef struct YS__Stvar      IVAR;
typedef struct YS__Stvar      FVAR;
typedef struct YS__Stat       STATREC;
typedef int    (*cond)();     /* Pointer to a function that returns int               */
typedef void   (*func)();     /* Pointer to a function with no return value           */

typedef struct YS__Prcr       PROCESSOR;
typedef struct YS__Mod        MODULE;
typedef struct YS__Buf        BUFFER;
typedef struct YS__Mux        MUX;
typedef struct YS__Demux      DEMUX;
typedef struct YS__Duplexmod  DUPLEXMOD;
typedef struct YS__OPort      OPORT;
typedef struct YS__IPort      IPORT;
typedef struct YS__Pkt        PACKET;
typedef struct YS__PktData    PKTDATA;
typedef int    (*rtfunc)();   /* Pointer to a routing function                        */
typedef double (*rngen)();    /* Random number generator                              */


/**************************************************************************************\
****************************************************************************************
**                                                                                    **
**                       GLOBAL VARIABLES (defined in util.c)                         **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

extern QUEUE    *YS__PendRes;         /* Queue of Resources to be evaluated           */
extern EVENT    *YS__ActEvnt;         /* Pointer to the currently occurring event     */
extern double   YS__Simtime;          /* The current simulation time                  */
extern int      YS__idctr;            /* Used to generate unique ids for objects      */
extern char     YS__prbpkt[];         /* Buffer for probe packets                     */
extern int      YS__msgcounter;       /* System defined unique message ID             */
extern int      YS__interactive;      /* Flag; set if running under viewsim or dbsim  */

extern POOL     YS__MsgPool;          /* Pool of MESSAGE descriptors                  */
extern POOL     YS__EventPool;        /* Pool of EVENT descriptors                    */
extern POOL     YS__QueuePool;        /* Pool of QUEUE descriptor                     */
extern POOL     YS__SemPool;          /* Pool of SEMAPHORE descriptors                */
extern POOL     YS__FlagPool;         /* Pool of FLAG descriptors                     */
extern POOL     YS__BarPool;          /* Pool of BARRIER descriptors                  */
extern POOL     YS__ResPool;          /* Pool of RESOURCE descriptors                 */
extern POOL     YS__StvarPool;        /* Pool of IVAR and FVAR descriptors            */
extern POOL     YS__CondPool;         /* Pool of CONDITION descriptors                */
extern POOL     YS__QelemPool;        /* Pool of QELEM descriptors                    */
extern POOL     YS__StatPool;         /* Pool of STATREC descriptors                  */
extern POOL     YS__HistPool;         /* Pool of histograms of default size           */

extern QUEUE    *YS__ActPrcr;         /* List of active processors                    */
extern POOL     YS__PrcrPool;         /* Pool of PROCESSOR descriptors                */
extern POOL     YS__PktPool;          /* Pool of short PACKET descriptors             */
extern int      YS__Cycles;           /* Count of profiling cycles accumulated        */
extern double   YS__CycleTime;        /* Cycle time                                   */
extern int      YS__PacketSize;       /* Packet size, can be changed by user          */
extern double   YS__OSDelays[NUMDELAYS][3];   /* Table of Operating System delays     */

extern POOL     YS__ReqPool;          /* Pool of REQ descriptors             */

extern STATREC  *YS__BusyPrcrStat;    /* Statrec for processor utilization            */
extern int      YS__BusyPrcrs;        /* Number of busy processors                    */
extern int      YS__TotalPrcrs;       /* Number of total processors                   */

extern STVAR    *SIMTIME;             /* An FVAR equal to the current simulation time */
extern int      TraceIDs;             /* If != 0, show object ids in trace output     */
extern int      TraceLevel;           /* Controls the amount of trace information     */


/**************************************************************************************\
****************************************************************************************
**                                                                                    **
**                              OBJECT DECLARATIONS                                   **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

/**************************************************************************************\
*                                 QELEM Declaration                                    *
\**************************************************************************************/

struct YS__Qelem {				/* Used only with state variables and pools           */
   char   *pnxt;                /* Next pointer for Pools                             */
   QELEM  *next;                /* Pointer to the next Qelem in the list              */
   char   *optr;                /* Pointer to an object                               */
};


/**************************************************************************************\
*                               POOL Declaration                                       *
\**************************************************************************************/

 struct YS__Pool {        /* Used to minimize the use of malloc        */
   char name[32];         /* User defined name                         */
   char *p_head;          /* Pointer to the first element of the queue */
   char *p_tail;          /* Pointer to the last element of the queue  */
   char *pf_head;         /* Pointer to the first element of the queue */
   char *pf_tail;         /* Pointer to the last element of the queue  */
   int  objects;          /* Number of objects to malloc               */
   int  objsize;          /* Size of objects in bytes                  */
   int newed;
   int killed;
};


/******************************  POOL Operations **************************************/

void YS__PoolInit(POOL *pptr, char *name, int objs, int objsz);  /* Initialize a pool  */
void YS__PoolStats(POOL *);
char *YS__PoolGetObj(POOL *pptr);     /* Get an object from a pool */
void YS__PoolReturnObj(POOL *pptr, void *optr);       /* Return an object to its pool              */
void YS__PoolReset(POOL *pptr);        /* Deallocate all objects in a pool */

/**************************************************************************************\
*                                 QE Declaration                                       *
\**************************************************************************************/

struct YS__Qe {					/* Queue element; base of most other objects          */
   char *pnxt;                  /* Next pointer for Pools                             */
   QE   *next;                  /* Pointer to the element after this one              */
   int  type;                   /* PROCTYPE or EVTYPE                                 */
   int  id;                     /* System defined unique ID                           */
   char name[32];               /* User assignable name for dubugging                 */
};


/**************************************************************************************\
*                              QUEUE Declaration                                       *
\**************************************************************************************/

struct YS__Queue {  /* Basic queue of QEs; derived from QE                     */
                           /* QE fields */
   char    *pnxt;               /* Next pointer for Pools                             */
   QE      *next;               /* Pointer to the element after this one              */
   int     type;                /* Identifies the type of queue element               */
   int     id;                  /* System defined unique ID                           */
   char    name[32];            /* User assignable name for dubugging                 */
                           /* QUEUE fields */
   QE      *head;               /* Pointer to the first element of the queue          */
   QE      *tail;               /* Pointer to the last element of the queue           */
   int     size;                /* Number of elements in the queue                    */
};

/******************************** QUEUE Operations ************************************/

QUEUE  *YS__NewQueue();         /* Create and return a pointer to a new queue         */
char   *YS__QueueGetHead();     /* Removes & returns a pointer to the head of a queue */
void   YS__QueuePutHead();      /* Puts a queue element at the head of a queue        */
void   YS__QueuePutTail();      /* Puts a queue element at the tail of a queue        */
char   *YS__QueueNext();        /* Get pointer to element following an element        */
int    YS__QueueDelete();       /* Removes a specified element from the queue         */
void   YS__QueueReset();        /* Removes and frees all of the queue's elements      */
double YS__QueueHeadval();      /* Returns the time of the first queue element        */
void   YS__QueueInsert();       /* Inserts an element in order of its time, FIFO      */
void   YS__QueueEnter();        /* Enters an element in order of its time, LIFO       */
void   YS__QueuePrinsert();     /* Inserts an elem in order of its priority, FIFO     */
void   YS__QueuePrenter();      /* Enters an element in order of its priority, LIFO   */
void   YS__QueuePrint();        /* Prints the contents of a queue                     */
int    YS__QeId();              /* Returns the system defined ID or 0 if TrID is 0    */
int    YS__QueueCheckElement(); /* Checks to see if an element is in a queue          */
void   QueueCollectStats();     /* Initiates statistics collection for a queue        */
void   QueueResetStats();       /* Resets statistics collectin for a queue            */
STATREC* QueueStatPtr();        /* Returns a pointer to a queue's statistics          */


/**************************************************************************************\
*                                SYNCQUE Declaration                                   *
\**************************************************************************************/

struct YS__SyncQue {/* Synchronization Queue; base of sema, flg, cond, barr    */
                           /* QE fields */
   char    *pnxt;               /* Next pointer for Pools                             */
   QE      *next;               /* Pointer to the element after this one              */
   int     type;                /* Identifies the type of queue element               */
   int     id;                  /* System defined unique ID                           */
   char    name[32];            /* User assignable name for dubugging                 */
                           /* QUEUE fields */
   QE      *head;               /* Pointer to the first element of the queue          */
   QE      *tail;               /* Pointer to the last element of the queue           */
   int     size;                /* Number of elements in the queue                    */
                           /* SYNCQUE fields */
   STATREC *lengthstat;         /* Queue length                                       */
   STATREC *timestat;           /* Time in the queue                                  */
};


/**************************************************************************************\
*                             SEMAPHORE Declaration                                    *
\**************************************************************************************/

struct YS__Sema  {
                           /* QE fields */
   char     *pnxt;              /* Next pointer for Pools                             */
   QE       *next;              /* Pointer to the element after this one              */
   int      type;               /* SEMTYPE                                            */
   int      id;                 /* System defined unique ID                           */
   char     name[32];           /* User assignable name for dubugging                 */
                           /* QUEUE fields */
   ACTIVITY *head;              /* Pointer to first element of the queue              */
   ACTIVITY *tail;              /* Pointer to last element of the queue               */
   int      size;               /* Number of elements in the queue                    */
                           /* SYNCQUE fields */
   STATREC  *lengthstat;        /* Queue length                                       */
   STATREC  *timestat;          /* Time in the queue                                  */
                           /* SEMAPHORE fields */
   int      val;                /* Semaphore value                                    */
   int      initval;            /* Initial value used for resetting the semaphore     */
};

/***************************** SEMAPHORE Operations ***********************************/

SEMAPHORE *NewSemaphore();      /* Creates & returns a pointer to a new semaphore     */
int        SemaphoreInit();     /* If queue is empty its value is set to i            */
void       SemaphoreSignal();   /* Signals the semaphore                              */
void       SemaphoreSet();      /* Set sem. to 1 if empty, or release an waiting act. */
int        SemaphoreDecr();     /* Decrement the sem. value and return the new value  */
void       SemaphoreWait();     /* Wait on a semaphore                                */
int        SemaphoreValue();    /* Returns the value of the semaphore                 */
int        SemaphoreWaiting();  /* Returns the # of activities in the queue           */


/**************************************************************************************\
 *                                 FLAG Declaration                                    *
\**************************************************************************************/

struct YS__Flag  {
                           /* QE fields */
   char     *pnxt;              /* Next pointer for Pools                             */
   QE       *next;              /* Pointer to the element after this one              */
   int      type;               /* FLGTYPE                                            */
   int      id;                 /* System defined unique ID                           */
   char     name[32];           /* User assignable name for dubugging                 */
                           /* QUEUE fields */
   ACTIVITY *head;              /* Pointer to first element of the queue              */
   ACTIVITY *tail;              /* Pointer to last element of the queue               */
   int      size;               /* Number of elements in the queue                    */
                           /* SYNCQUE fields */
   STATREC  *lengthstat;        /* Queue length                                       */
   STATREC  *timestat;          /* Time in the queue                                  */
                           /* FLAG fields */
   int      state;              /* CLEARED or SET                                     */
};

/******************************** FLAG Operations *************************************/

FLAG *NewFlag();                /* Creates and returns a pointer to a new flag        */
int   FlagSet();                /* Sets a flag                                        */
int   FlagRelease();            /* Releases activities waiting at a flag              */
void  FlagWait();               /* Waits for a flag to be set or released             */
int   FlagWaiting();            /* Returns the # of activities in the queue           */


/**************************************************************************************\
*                               BARRIER Declaration                                    *
\**************************************************************************************/

struct YS__Barr  {
                           /* QE fields */
   char     *pnxt;              /* Next pointer for Pools                             */
   QE       *next;              /* Pointer to the element after this one              */
   int      type;               /* BARTYPE                                            */
   int      id;                 /* System defined unique ID                           */
   char     name[32];           /* User assignable name for dubugging                 */
                           /* QUEUE fields */
   ACTIVITY *head;              /* Pointer to first element of the queue              */
   ACTIVITY *tail;              /* Pointer to last element of the queue               */
   int      size;               /* Number of elements in the queue                    */
                           /* SYNCQUE fields */
   STATREC  *lengthstat;        /* Queue length                                       */
   STATREC  *timestat;          /* Time in the queue                                  */
                           /* BARRIER fields */
   int      cnt;                /* Waiting for "cnt" processes                        */
   int      initcnt;            /* Initial count                                      */
};

/******************************** BARRIER Operations **********************************/

BARRIER *NewBarrier();          /* Creates and returns a pointer to a new barrier     */
int      BarrierInit();         /* If a barrier's queue is empty, sets its value to i */
void     BarrierSync();         /* Waits at a barrier synchronization point           */
int      BarrierNeeded();       /* Returns # of additional syncs to free barrier      */
int      BarrierWaiting();      /* Returns the number of processes waiting at barrier */


/**************************************************************************************\
*                             CONDITION Declaration                                    *
\**************************************************************************************/

struct YS__Cond  {
                           /* QE fields */
   char     *pnxt;              /* Next pointer for Pools                             */
   QE       *next;              /* Pointer to the element after this one              */
   int      type;               /* CONDTYPE                                           */
   int      id;                 /* System defined unique ID                           */
   char     name[32];           /* User assignable name for dubugging                 */
                           /* QUEUE fields */
   ACTIVITY *head;              /* Pointer to first element of the queue              */
   ACTIVITY *tail;              /* Pointer to last element of the queue               */
   int      size;               /* Number of elements in the queue                    */
                           /* SYNCQUE fields */
   STATREC  *lengthstat;        /* Queue length                                       */
   STATREC  *timestat;          /* Time in the queue                                  */
                           /* CONDITION fields */
   int      state;              /* TRUE, FALSE                                        */
   cond     body;               /* Pointer to the condition's defining function       */
   char     *argptr;            /* Pointer to arguments or data for this condition    */
   int      argsize;            /* Size of argument structure                         */
};

/******************************** CONDITION Operations ********************************/

CONDITION *NewCondition();      /* Creates and returns a pointer to a new condition   */
void       ConditionWait();     /* Waits until a condition holds                      */
int        ConditionState();    /* Returns the state of a condition                   */
int        ConditionWaiting();  /* Returns the # of activities in the queue           */
void       ConditionSetArg();   /* Sets the argument pointer of a condition           */


/**************************************************************************************\
*                              RESOURCE Declaration                                    *
\**************************************************************************************/

struct YS__Resc  {
                           /* QE fields */
   char     *pnxt;              /* Next pointer for Pools                             */
   QE       *next;              /* Pointer to the element after this one              */
   int      type;               /* RESTYPE                                            */
   int      id;                 /* System defined unique ID                           */
   char     name[32];           /* User assignable name for dubugging                 */
                           /* QUEUE fields */
   ACTIVITY *head;              /* Pointer to first element of the queue              */
   ACTIVITY *tail;              /* Pointer to last element of the queue               */
   int      size;               /* Number of elements in the queue                    */
                           /* RESOURCE fields */
   STATREC  *lengthstat;        /* Queue length                                       */
   STATREC  *timestat;          /* Time in the queue                                  */
   STATREC  *utilstat;          /* Server utilization                                 */
   int      servers;            /* Maximum number of servers for this queue           */
   int      freeservers;        /* Number of free servers                             */
   int      qdisc;              /* Type of queue discipline used                      */
   ACTIVITY *serving;           /* Pointer to process getting service                 */
   int      status;             /* QUIESCENT, PENDING                                 */
   int      oldsize;            /* Used to adjust times in PROCSHAR resources         */
   double   timeslice;          /* Used for RR queue discipline                       */
};

/****************************** RESOURCE Operations ***********************************/

void     YS__RscActivate();     /* Moves processes from a resource to the EvntLst     */
int      YS__RscContinue();     /* Returns an activity to a resource after a slice    */
void     YS__RscInsert();       /* Inserts an activity in a resource                  */
void     YS__RscAdd();          /* Adds a process to a processor's ready list         */
RESOURCE *NewResource();        /* Creates and returns a pointer to a new resource    */
void     ResourceUse();         /* Requests service from a resource                   */
int      ResourceWaiting();     /* Returns the # of processes in the queue            */
int      ResourceServicing();   /* Returns the # of activities getting service        */
void     ResourceResetStats();  /* Resets all statistics records of the resource      */


/**************************************************************************************\
*                               STATQE Declaration                                     *
\**************************************************************************************/

struct YS__StatQe { /* Statistical queue element; base of activities & msgs    */
                           /* QE fields */
   char       *pnxt;            /* Next pointer for Pools                             */
   QE         *next;            /* Pointer to the element after this one              */
   int        type;             /* PROCTYPE or EVTYPE                                 */
   int        id;               /* System defined unique ID                           */
   char       name[32];         /* User assignable name for dubugging                 */
                           /* STATQE fields */
   double     enterque;         /* Time the activity enters a queue                   */
};


/**************************************************************************************\
*                             ACTIVITY Declaration                                     *
\**************************************************************************************/

struct YS__Act {    /* Activity; base of processes and events                  */
                           /* QE fields */
   char       *pnxt;            /* Next pointer for Pools                             */
   QE         *next;            /* Pointer to the element after this one              */
   int        type;             /* PROCTYPE or EVTYPE                                 */
   int        id;               /* System defined unique ID                           */
   char       name[32];         /* User assignable name for dubugging                 */
                           /* STATQE fields */
   double     enterque;         /* Time the activity enters a queue                   */
                           /* ACTIVITY fields */
   char       *argptr;          /* Pointer to arguments or data for this activity     */
   int        argsize;          /* Size of argument structure                         */
   double     time;             /* time used for ordering priority queues             */
   int        parentid;         /* Unique id of process that forked this ACTIVITY     */
   int        status;           /* Status of activity (e.g., ready, waiting,...)      */
   int        blkflg;           /* INDEPENDENT, BLOCK, or FORK                        */
   STATREC    *statptr;         /* Statistics record for status statistics            */
   double     priority;         /* Priority used for resource scheduling              */
   double     timeleft;         /* Used for RR resources and time slicing             */
   ACTIVITY   *rscnext;         /* Used for PROCSHAR resources only                   */
   RESOURCE   *rscptr;          /* Activity using resource *rscptr                    */
   PROCESSOR  *pp;              /* This process is attached to the processor *pp      */
};

/******************************** ACTIVITY Operations *********************************/

int      YS__ActId();           /* Returns the system define Id or 0 if TrID is 0     */
void     ActivitySetArg();      /* Sets the argument pointer of an activity           */
char     *ActivityGetArg();     /* Returns the argument pointer of an activity        */
int      ActivityArgSize();     /* Returns the size of an argument                    */
void     ActivitySchedTime();   /* Schedules an activity to start in the future       */
void     ActivitySchedSema();   /* Schedules an activity to wait for a semaphore      */
void     ActivitySchedFlag();   /* Schedules an activity to wait for a flag           */
void     ActivitySchedCond();   /* Schedules an activity to wait for a condition      */
void     ActivitySchedRes();    /* Schedules an activity to wait use a resource       */
void     ActivityCollectStats();/* Activates statistics collection for an activity    */
void     ActivityStatRept();    /* Prints a report of an activity's statistics        */
STATREC  *ActivityStatPtr();    /* Returns a pointer to an activity's stat record     */
ACTIVITY *ActivityGetMyPtr();   /* Returns a pointer to the active activity           */
ACTIVITY *ActivityGetParPtr();  /* Returns a pointer to the active activity's parent  */


/**************************************************************************************\
*                                EVENT Declaration                                     *
\**************************************************************************************/

struct YS__Event {
                           /* QE fields */
   char       *pnxt;            /* Next pointer for Pools                             */
   QE         *next;            /* Pointer to the element after this one              */
   int        type;             /* PROCTYPE or EVTYPE                                 */
   int        id;               /* System defined unique ID                           */
   char       name[32];         /* User assignable name for dubugging                 */
                           /* STATQE fields */
   double     enterque;         /* Time the activity enters a queue                   */
                           /* ACTIVITY fields */
   char       *argptr;          /* Pointer to arguments or data for this activity     */
   int        argsize;          /* Size of argument sturcture                         */
   double     time;             /* time used for ordering priority queues             */
   int        parentid;         /* Unique id of process that forked this ACTIVITY     */
   int        status;           /* Status of activity (e.g., ready, waiting,...)      */
   int        blkflg;           /* INDEPENDENT, BLOCK, or FORK                        */
   STATREC    *statptr;         /* Statistics record for status statistics            */
   double     priority;         /* Priority used for resource scheduling              */
   double     timeleft;         /* Used for RR resources and time slicing             */
   ACTIVITY   *rscnext;         /* Used for PROCSHAR resources only                   */
   RESOURCE   *rscptr;          /* Activity using resource *rscptr                    */
   PROCESSOR  *pp;              /* This process is attached to the processor *pp      */
                           /* EVENT fields */
   func     body;               /* Defining function for the event                    */
   int      state;              /* Used to save return point after reschedule         */
   int      deleteflag;         /* DELETE or NODELETE                                 */
   int      evtype;             /* User defined event type                            */
};

/********************************* EVENT Operations ***********************************/

EVENT *NewEvent();              /* Creates and returns a pointer to a new event       */
EVENT *NewOSEvent();            /* Creates and return a pointer to a new OS event     */
int    EventGetType();          /* Returns the events type                            */
void   EventSetType();          /* Sets the event's type                              */
int    EventGetDelFlag();       /* Returns DELETE (1) or NODELETE (0)                 */
void   EventSetDelFlag();       /* Makes an event deleting                            */
int    EventGetState();         /* Returns the state of an event                      */
void   EventSetState();         /* Sets state used to designate a return point        */
void   EventReschedTime();      /* Reschedules an event to occur in the future        */
void   EventReschedSema();      /* Reschedules an event to wait for a semaphore       */
void   EventReschedFlag();      /* Reschedules an event to wait for a flag            */
void   EventReschedCond();      /* Reschedules an event to wait for a condition       */
void   EventReschedRes();       /* Reschedules an event to use a resource             */


/**************************************************************************************\
*                              MESSAGE Declaration                                     *
\**************************************************************************************/

struct YS__Mess {
                           /* QE fields */
   char      *pnxt;             /* Next pointer for Pools                             */
   QE        *next;             /* Pointer to the element after this one              */
   int       type;              /* Indentifies the type of QE                         */
   int       id;                /* System defined unique ID                           */
   char      name[32];          /* User assignable name for dubugging                 */
                           /* STATQE fields */
   double    enterque;
                           /* MESSAGE fields */
   char      *bufptr;           /* Pointer to the message contents                    */
   int       msgtype;           /* User defined message type                          */
   int       msgsize;           /* Size of the message in bytes                       */
   int       pktsz;             /* LONG or SHORT packets                              */
   STVAR     *status;           /* Pointer to a user supplied status state variable   */
   double    sendtime;          /* Time the message was sent                          */
   ACTIVITY  *source;           /* Pointer to sending activity                        */
   int       srccpu;            /* ID of sending process' processor                   */
   double    priority;          /* Priority used for resolving routing conflicts      */
   int       blockflag;         /* BLOCK or NOBLOCK                                   */
   int       destflag;		/* msg sent to a PROCESSDEST or PROCESSORDEST         */
   PACKET    *packets;          /* Pointer to message's packets                       */
   int       pktorecv;          /* Count of packets yet to be recieved                */
   int       pktosend;          /* Count of packets yet to be sent                    */
};


/**************************************************************************************\
*                                 STVAR Declaration                                    *
\**************************************************************************************/

struct YS__Stvar {
                           /* QE fields */
   char      *pnxt;             /* Next pointer for Pools                             */
   QE        *next;             /* Pointer to the element after this one              */
   int       type;              /* IVARTYPE or FVARTYPE                               */
   int       id;                /* System defined unique ID                           */
   char      name[32];          /* User assignable name for dubugging                 */
                           /* STVAR fields */
   QELEM     *CondList;         /* Conditions that depend on this STVAR               */
   union {
      double fval;              /* Value for FVARs                                    */
      int    ival;              /* Value for IVARs                                    */
   } val;
};

/******************************** STVAR Operations ************************************/

int    YS__SvId();              /* Returns the system defined Id or 0 if TraceId is 0 */
STVAR *NewIvar();               /* Creates and returns a pointer to a new Ivar        */
STVAR *NewFvar();               /* Creates and returns a pointer to a new Fvar        */
void   Iset();                  /* Sets the value of an Ivar                          */
void   Fset();                  /* Sets the value of an Fvar                          */
int    Ival();                  /* Returns the value of an Ivar                       */
double Fval();                  /* Returns th value of an Fvar                        */


/**************************************************************************************\
*                            STATRECORD  Declaration                                   *
\**************************************************************************************/

struct YS__Stat {
                           /* QE fields */
   char    *pnxt;               /* Next pointer for Pools                             */
   QE      *next;               /* Pointer to the element after this one              */
   int     type;                /* PNTSTATTYPE or INTSTATTYPE                         */
   int     id;                  /* System defined unique ID                           */
   char    name[32];            /* User assignable name for dubugging                 */
                           /* STATRECORD Fields */
   double  maxval,minval;       /* Max and min update values encountered              */
   int     samples;             /* Number of updates                                  */
   int     meanflag;            /* MEANS or NOMEANS                                   */
   double  sum;                 /* Accumulated sum of update values                   */
   double  sumsq;               /* Accumulated sum of squares of update values        */
   double  sumwt;               /* Accumulated sum of weights                         */
   double  *hist;               /* Pointer to a histogram array                       */
   int     bins;                /* Number of bins of histograms                       */
   double  time0;               /* Starting time of the current sampling interval     */
   double  time1;               /* Time of the last sample                            */
   double  lastt;               /* Last interval point entered with Update()          */
   double  lastv;               /* Last interval value entered with Update()          */
   double  interval;            /* The sampling interval                              */
   int     intervalerr;         /* Nonzero => a negative interval encountered         */
};

/******************************** STATREC Operations **********************************/

int      YS__StatrecId();       /* Returns the system defined ID or 0                 */
STATREC  *NewStatrec();         /* Creates and returns a pointer to a new statrec     */
void     StatrecSetHistSz();    /* Sets the default histogram size                    */
void     StatrecReset();        /* Resets the statrec                                 */
void     StatrecUpdate();       /* Updates the statrec                                */
void     StatrecReport();       /* Generates and displays a statrec report            */
int      StatrecSamples();      /* Returns the number of samples                      */
double   StatrecMaxVal();       /* Returns the maximum sample value                   */
double   StatrecMinVal();       /* Returns the minimum sample value                   */
int      StatrecBins();         /* Returns the number of bins                         */
double   StatrecLowBin();       /* Returns the low bin upper bound                    */
double   StatrecHighBin();      /* Returns the high bin lower bound                   */
double   StatrecBinSize();      /* Returns the bin size                               */
double   StatrecHist();         /* Returns the value of the ith histogram element     */
double   StatrecMean();         /* Returns the mean                                   */
double   StatrecSdv();          /* Returns the standard deviation                     */
double   StatrecInterval();     /* Returns the sampling interval                      */
double   StatrecRate();         /* Returns the sampling rate                          */
void     StatrecEndInterval();  /* Terminates a sampling interval                     */


/**************************************************************************************\
*                             EVENT LIST Operations                                    *
\**************************************************************************************/

void     YS__EventListSetBins();  /* Sets the number of bins to a fixed size          */
void     YS__EventListSetWidth(); /* Sets the bin width to a fixed size               */
void     YS__EventListInit();     /* Initializes the event list                       */
void     YS__EventListInsert();   /* Inserts an element into the event list           */
ACTIVITY *YS__EventListGetHead(); /* Returns the head of the event list               */
double   YS__EventListHeadval();  /* Returns the time value of the event list head    */
int      YS__EventListDelete();   /* Removes an element from the event list           */
void     YS__EventListPrint();    /* Lists the contents of the event list             */
int      EventListSize();         /* Returns the size of the event list               */
void     EventListSelect();       /* Selects the type of event list to use            */
void     EventListCollectStats(); /* Activates auto stats collection for event list   */
void     EventListResetStats();   /* Resest a statistics record of a queue            */
STATREC  *EventListStatPtr();     /* Returns a pointer to a event list's statrec      */


/**************************************************************************************\
*                                 DRIVER Operations                                    *
\**************************************************************************************/

void YS__RdyListAppend();       /* Appends an activity onto the systme ready list     */
void DriverReset();             /* Resets the driver (Sets YS__Simtime to 0)          */
void DriverInterrupt();         /* Interrupts the driver and returns to the user      */
int  DriverRun();               /* Activates the simulation driver; returns a value   */
void BreakPoint();              /* Causes the simulation manager to halt immediately  */


/**************************************************************************************\
*                                UTILITY Operations                                    *
\**************************************************************************************/

void   YS__errmsg();           /* Prints error message & terminates simulation        */
void   YS__warnmsg();          /* Prints warning message (if TraceLevel > 0)          */
void   YS__SendPrbPkt (char type, const char *name, const char *data);      /* Sends a trace message to a probe                    */

double GetSimTime();           /* Returns the current simulation time                 */
double YacRand();              /* Random Number Generator                             */
void   YacSeed();              /* Set the seed for yacrand                            */
void   TracePrint(const char *fmt, ...);
void   TracePrintTag(const char *tag, const char *fmt, ...);

/**************************************************************************************\
*                               PROCESSOR Declaration                                  *
\**************************************************************************************/

struct YS__Prcr {
                               /* QE fields */
   char     *pnxt;                /* Next pointer for Pools                           */
   QE       *next;                /* Pointer to the element after this one            */
   int      type;                 /* Not used by processors                           */
   int      id;                   /* User Defined id                                  */
   char     name[32];             /* User assignable name for dubugging               */
                               /* PROCESSOR fields */
   int      status;               /* IDLE, BUSY                                       */
   RESOURCE *RdyList;             /* Processor's ready list - a RRPRWP resource       */
   EVENT    *localsender;         /* Event for sending local messages                 */
   QUEUE    *localmsgs;           /* Queue of local messages                          */
   IPORT    *iports[MAXFANOUT];   /* Network input ports                              */
   EVENT    *iportevt[MAXFANOUT]; /* Network input port events                        */
   QUEUE    *iportmsg[MAXFANOUT]; /* Network input port message queues                */
   int      noiports;             /* Number of network input ports on the processor   */
   rtfunc   router;               /* Message routing function for the processor       */
   OPORT    *oports[MAXFANOUT];   /* Network output ports                             */
   int      nooports;             /* Number of network output ports on the processor  */
   QUEUE    *MsgList;             /* Queue of messages delivered to this processor    */
   QUEUE    *WaitingProcesses;    /* Queue of processes waiting for messages          */
 };

/******************************* PROCESSOR Operations *********************************/

PROCESSOR *NewProcessor();        /* Creates a new PROCESSOR                          */
void      ProcessorAttach();      /* Attaches a process to a processor                */
void      SetCycleTime();         /* Set the PARCSIM cycle time                       */
void      SetOSDelays();          /* Set various OS delays                            */
void      ProcessorSendMsg();     /* Sends a message to a processor                   */
int       ProcessorReceiveMsg();  /* Copies receives data into buf & returns its size */
int       ProcessorCheckMsg();    /* Checks for a message of a given type             */
void      ProcessorGlobalUtil();  /* Activates computation of global processor util.  */
void      ProcessorLocalUtil();   /* Activates computation of local processor util.   */
double    ProcessorGetUtil();     /* Returns the processor utiliaztion to date        */
void      ProcessorUtilRept();    /* Prints out the processor util statistics         */
void      YS__PSDelay();



/**************************************************************************************\
*                                  MODULE Declaration                                  *
\**************************************************************************************/

struct YS__Mod
{
   int id;                   /* User Supplied ID for debugging                        */
   int type;                 /* Module type - BUFFERTYPE, MUXTYPE, DEMUXTYPE PORTTYPE */
};


/**************************************************************************************\
*                                     MUX Declaration                                  *
\**************************************************************************************/

struct YS__Mux
{
   int id;                      /* User Supplied ID for debugging                     */
   int type;                    /* Module type = MUXTYPE                              */

   MODULE *nextmodule;          /* Pointer to the next module connected to this one   */
   SEMAPHORE *arbsema;          /* Pointer to the semaphore for this MUX              */
   int index;                   /* Input index if next module is a MUX                */
   MUX *muxptr;                 /* Pointer to a preceeding MUX                        */
   int fan_in;                  /* No of inputs to the MUX                            */
   int muxtype;					/* Type parameters classifies the mux				  */

   SEMAPHORE *cktsema;				/* Semaphore that locks the circuit				  */
};

/*********************************** MUX Operations ***********************************/

MUX       *NewMux();              /* Creates and returns a pointer to a new mux       */


/**************************************************************************************\
*                                   DEMUX Declaration                                  *
\**************************************************************************************/

struct YS__Demux
{
   int id;                         /* User Supplied ID for debugging                  */
   int type;                       /* Module type = DEMUXTYPE                         */

   MODULE **nextmodule;            /* Pointers to the next modules                    */
   int *index;                     /* Input index if next module is a MUX             */
   int fan_out;                    /* Fan-out of the demux                            */
   rtfunc router;                  /* Pointer to a routing function                   */

   int util;
   int demuxtype;				   /* Type parameters classifies the demux			  */
   MODULE *prevmodule;			   /* Needed to connect demuxes to previous buffers   */

   int *route;					   /* Needed to keep track of output function when
   										adaptive routing functions are implemented	  */
};

/********************************** DEMUX Operations **********************************/

DEMUX     *NewDemux();            /* Creates and returns a pointer to a new demux     */


/**************************************************************************************\
*                                INPUT PORT Declaration                                *
\**************************************************************************************/

struct YS__IPort
{
   int id;                      /* User Supplied ID for debugging                     */
   int type;                    /* Module type - IPORTTYPE                            */

   MODULE *nextmodule;          /* Pointer to the next module connected to this one   */
   MODULE *destination;         /* Pointer to next module for tail to enter           */
   int index;                   /* Input index if next module is a MUX                */
   SEMAPHORE *netrdy;           /* Semaphore that controls packet movement into net   */
   SEMAPHORE *portrdy;          /* Semaphore for users to wait at                     */
   int qfree;                   /* Number of packets that can be queued at port       */
};

/********************************** IPORT Operations **********************************/

IPORT     *NewIPort();            /* Creates and returns a pointer to a new iport     */
SEMAPHORE *IPortSemaphore();      /* Returns a pointer to the ready sema of an iport  */
int       IPortSpace();           /* Returns the # of free packet slots in an iport   */


/**************************************************************************************\
*                               OUTPUT PORT Declaration                                *
\**************************************************************************************/

struct YS__OPort
{
   int id;                      /* User Supplied ID for debugging                     */
   int type;                    /* Module type - OPORTTYPE                            */

   SEMAPHORE *freespace;        /* Free packet space avaialble in port                */
   SEMAPHORE *pktavail;         /* Packet available to processor                      */
   PACKET *qhead;               /* Head pointer for the queue                         */
   PACKET *qtail;               /* Tail pointer for the queue                         */
   MUX *muxptr;                 /* Pointer to a preceeding MUX                        */
   int count;                   /* Number of packets in the port's queue              */
   PROCESSOR *pr;               /* Processor to which the oport is connected          */
   EVENT *event;                /* PacketReceiverEvent running on procr if attached   */

   double power;
   int input_buffer;
   int crossbar;
   int link;
   int route;
   int vc;
   int swtch;
};

/********************************** OPORT Operations **********************************/

OPORT     *NewOPort();          /* Creates and returns a pointer to a new oport       */
SEMAPHORE *OPortSemaphore();    /* Returns a pointer to the req sema of an oport      */
int       OPortPackets();       /* Returns the number of pkts avail at an oport       */


/**************************************************************************************\
*                                  BUFFER Declaration                                  *
\**************************************************************************************/

struct YS_VCStruct
{
	int			*input_port;
	int			*input_vc;
	int			*input_credit;
	int			*input_seqno;
	MODULE		**inputbuf;
	int			congestion;
	int			keep_track;
	double		*creditime;

};

struct YS__Buf
{
   int id;                      /* User Supplied ID for debugging                     	*/
   int type;                    /* Module type = BUFFERTYPE                           	*/

   PACKET *head;                /* Pointer to element at the front of the buffer      	*/
   PACKET *tail;                /* Pointer to element at the end of the buffer        	*/
   int    size;                 /* Size of the buffer in flits                        	*/
   int    free;                 /* Number of free flit positions                      	*/
   int    tailtype;             /* Type of last event in buffer: HEADTYPE or TAILTYPE 	*/
   MODULE *nextmodule;          /* Pointer to the next module connected to this one   	*/
   MODULE *destination;         /* Pointer to next module for tail to enter           	*/
   EVENT  *WaitingHead;         /* Pointer to pkt head waiting to enter the buffer    	*/
   MUX    *muxptr;              /* Pointer to semaphore acquired by HeadEvent         	*/
   int    index;                /* Input index if next module is a MUX			*/
   int	  buftype;		/* specifies to be input/output buffer			*/

   /* Following are needed for Input Buffer Implementation */
   MODULE *outputbuf;		/* This is used to connect to buffers for credit	*/
   int	  output_port;
   int    output_vc;
   int	  output_credit;
   int	  output_seqno;
   int	  output_alloc;
   SEMAPHORE *bufcredit_sema;

   /* Following are needed for Output Buffer Implementation */
   SEMAPHORE *bufva_sema;
   struct YS_VCStruct	vc_data;

   /* Following connections are needed for credit implementation */
   MODULE *prevmodule;

   /* Congestion Control Sema */
   SEMAPHORE *bufcc_sema;

   /* VC Allocation Sema */
   SEMAPHORE *va_alloc;
};

/********************************* BUFFER Operations **********************************/

BUFFER    *NewBuffer();           /* Creates and returns a pointer to a new buffer    */

/********************************* BUFFER Operations **********************************/

/**************************************************************************************\
*                                  PKTDATA Declaration                                 *
\**************************************************************************************/

struct YS__PktData
{
   int     seqno;               /* User supplied id for sequencing packts of a msg    */
   MESSAGE *mesgptr;            /* Pointer the message for this packet                */
   int     pktsize;             /* Number of flits in the packet                      */
   int     srccpu;              /* ID of CPU sending message                          */
   int     destcpu;             /* ID of CPU receiving message                        */
   double  recvtime;            /* Time to move the packet out of an output port      */

   double  createtime;          /* Time the packet was created                        */
   double  nettime;             /* Time the packet spent in the network               */
   double  blktime;             /* Time the packet was blocked in the network         */
   double  oporttime;           /* Time th packet spent waiting in an output port     */
   int	   vcindex;				/* Virtual Channel ID carried by the packet header	  */
   int	   route;				/* Routing information obtained						  */
   int	   packetsize;
   int	   pkttype;
   int	   ckt;
   int	   intercpu;			/* Intermediate destination CPU						  */
   int	   routeA;				/* Indicates the completion of first phase of routing */

   double  powers;
   int		xbar;
   int		ibufs;
   int		lnk;
   int		vastage;
   int		rcstage;
   int		sastage;

   int	   switching;
   int	   tag;
};


/**************************************************************************************\
*                                  PACKET Declaration                                  *
\**************************************************************************************/

struct YS__Pkt
{
   char    *pnxt;          /* Next pointer for Pools                                  */
   PACKET  *next;          /* Used in message operations                              */

   struct YS__PktData data;/* Packet's user accessible data (See prev declaration)    */

   MUX     *muxptr;        /* Pointer to the last MUX passed through by the head      */
   int     index;          /* Index used for routing out of a demux                   */
   EVENT   *headev;        /* Pointer to this packet's head event                     */
   MODULE  *module;        /* Pointer to the module that the head is at               */
   EVENT   *SleepingTail;  /* Pointer to the tail event of this packet, if sleeping   */
   int     waitingfortail; /* Used in SAF when head must wait for tail                */
   IPORT   *lastiport;     /* Iport through which the packet entered the network      */

   /* The head and tail flits of a packet can be in two different buffers at the same */
   /* time.  Headbuf and Tailbuf point to the buffers that contain the head and tail. */
   /* Since the head and tail flits can be in the event list at the same time they    */
   /* are in buffers, we can not use the event's next pointer for buffers.  Therefore */
   /* headnext and tailnext are fields in the pakcet's descriptor for this purpose.   */
   /* Tailoffset indicates the number of flit positions that the tail is from the     */
   /* front of a buffer if it is the first element in the buffer queue.  That is, the */
   /* tail flit may be at the front of the queue, but there may still be  some flits  */
   /* internal to the packet ahead of the tail flit in the buffer.                    */

   MODULE  *headbuf;       /* Pointer to the module that holds the head               */
   MODULE  *tailbuf;       /* Pointer to the module that holds the tail               */
   PACKET  *headnext;      /* Next pointer for the head event                         */
   PACKET  *tailnext;      /* Next pointer for the tail event                         */
   int     tailoffset;     /* Offset of the tail from the front of its buffer         */

   MODULE  *carrybuf;	   /* Pointer to the input buffer module					  */
};

/******************************* PACKET Operations ************************************/

PACKET    *NewPacket();           /* Creates and returns a pointer to a new packet    */
double    PacketSend();           /* Sends a packet to a network iport                */
PACKET    *PacketReceive();       /* Receives a packet from a network oport           */
PKTDATA   *PacketGetData();       /* Returns a pointer to the data in a packet        */
void      PacketFree();           /* Returns a packet to the pool of free packets     */


/**************************************************************************************\
*                                NETWORK Operations                                    *
\**************************************************************************************/

void    NetworkConnect();       /* Connects two network modules                       */
void    NetworkSetCycleTime();  /* Sets the cycle time; all other times are multiples */
void    NetworkSetFlitDelay();  /* Sets the flit delay                                */
void    NetworkSetMuxDelay();   /* Sets the time to move a flit through a mux         */
void    NetworkSetArbDelay();   /* Sets the time for a flit to arbitrate at a mux     */
void    NetworkSetDemuxDelay(); /* Sets the time to move a flit through a demux       */
void    NetworkSetPktDelay();   /* Sets the packet delay                              */
void    NetworkSetThresh();     /* Sets the buffer threshold                          */
void    NetworkSetWFT();        /* Sets the WFT mode                                  */
void    NetworkCollectStats();  /* Activates stat colleciton for the network          */
void    NetworkResetStats();    /* Resets all network statistics records              */
STATREC *NetworkStatPtr();      /* Returns a pointer to a network statistics record   */
void    NetworkStatRept();      /* Prints a report of network statistics              */

#endif
