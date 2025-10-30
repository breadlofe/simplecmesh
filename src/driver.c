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

// Fixed minor bugs and commented out some unessisary calls to objects no longer supported for the linux version
// Brian Neel AMD 2013

#include "../incl/simsys.h"
#include "../incl/dbsim.h"
#include "../incl/tr.driver.h"

#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>

static double SimUpTo;               /* Duration of a simulation run         */
static int    StopFlag;              /* If != 0, Driver interrupts and returns        */

static int     YS__TrMode;           /* Parameters used for interactive control       */
static double  YS__TrTime;           /* with DBSIM                           */
static int     YS__TrIter;

void UserMain(int, char **);

/*****************************************************************************/
/* main: This is were it all starts.  Main initializes                       */
/* the simulator driver.  It then calls the user-supplied routine UserMain.  */
/*****************************************************************************/

int
main (int argc, char *argv[])
{
  fprintf(stderr,"Simulation starting!\n");

  YS__idctr = 0;                        /* Unique ID generator              */
  YS__ActEvnt = NULL;                   /* Points to currently active event */
  YS__Simtime = 0.0;                    /* Simulation time starts at 0      */
  StopFlag    = 0;                      /* If != 0, Driver interrupts and returns    */
  YS__Cycles = 0;                       /* Initialize PARCSIM delays        */
  YS__CycleTime = 1.0;

  /* Initialize all the pools used by the simulator */
  //YS__PoolInit(&YS__MsgPool,"MessagePool",100,sizeof(MESSAGE));
  YS__PoolInit(&YS__EventPool,"EventPool",1000,sizeof(EVENT));
  //YS__PoolInit(&YS__QueuePool,"QueuePool",4,sizeof(QUEUE));
  YS__PoolInit(&YS__SemPool,"SemaphorePool",1000,sizeof(SEMAPHORE));
  //YS__PoolInit(&YS__QelemPool,"QelemPool",50,sizeof(QELEM));
  YS__PoolInit(&YS__StatPool,"StatRecPool",100,sizeof(STATREC));
  //YS__PoolInit(&YS__HistPool,"HistogramPool",50,DEFAULTHIST);
  //YS__PoolInit(&YS__PrcrPool,"ProcessorPool",25,sizeof(PROCESSOR));
  YS__PoolInit(&YS__PktPool,"PacketPool",1000,sizeof(PACKET));

  YS__EventListInit();                           /* Initialize the event list */

  //YS__ActPrcr = YS__NewQueue("ActPrcr");         /* For PARCSIM statistics  */
  //YS__TotalPrcrs = 0;
  //YS__BusyPrcrStat = NULL;

  UserMain(argc,argv);                      /* Transfer to user code  */

  exit(0);
}

/*****************************************************************************/
/* DRIVER Operations: These routines manipulate the event list and           */
/* call the user activities at the appropriate simulation times.             */
/*****************************************************************************/

void YS__RdyListAppend (ACTIVITY *aptr)   /* Appends an activity onto the system ready list  */
{
  aptr->time = YS__Simtime;   /* Ready list is all activities on the event list*/
  YS__EventListInsert(aptr);  /*    schedule at the current simulation time  */
  aptr->status = READY;       /* For statistics collection               */
  if (aptr->statptr)          /* Statistics collected for activities     */
    StatrecUpdate(aptr->statptr,(double)READY,YS__Simtime);
}

/*****************************************************************************/

void DriverReset()                 /* Resets the driver (Sets YS__Simtime to 0)*/
{
  STATREC *srptr;
  MESSAGE *mptr;

  if (YS__ActEvnt != NULL)
    YS__errmsg("Can not call DriverReset() from within a process or an event");

  /* Return all objects to the pools & free all malloced structures */

  for (mptr = (MESSAGE*)YS__MsgPool.p_head; mptr != NULL; mptr = (MESSAGE*)(mptr->pnxt))
    if (mptr->bufptr != NULL) free(mptr->bufptr);
  YS__PoolReset(&YS__MsgPool);
  YS__PoolReset(&YS__EventPool);
  YS__PoolReset(&YS__QueuePool);
  YS__PoolReset(&YS__SemPool);
  YS__PoolReset(&YS__QelemPool);
  YS__PoolReset(&YS__PrcrPool);
  YS__PoolReset(&YS__PktPool);
  for (srptr = (STATREC*)YS__StatPool.p_head;
       srptr != NULL;
       srptr = (STATREC*)(srptr->pnxt) )
    if (srptr->hist != NULL)
    {
      if ((srptr->bins+4)*sizeof(double) <= YS__HistPool.objsize)
      {
	YS__PoolReturnObj(&YS__HistPool,srptr->hist-1);
      }
      else
      {
	free(srptr->hist);
      }
    }
  YS__PoolReset(&YS__HistPool);
  YS__PoolReset(&YS__StatPool);

  YS__ActEvnt = NULL;
  YS__Simtime = 0.0;
  YS__TrTime = 0.0;
  YS__Cycles = 0;
  YS__CycleTime = 1.0;
  StopFlag = 0;
  YS__EventListInit();
  TRACE_DRIVER_reset;
}

/*****************************************************************************/

/* Interrupts the driver and returns to the user   */
void DriverInterrupt (int i /* User supplied return value */)
{
  PSDELAY;

  if (i == 0) YS__errmsg("Simulator interrupted with 0 stopflag");
  StopFlag = i;                    /* i returned by DriverRun()             */
  TRACE_DRIVER_interrupt1
}

/*****************************************************************************/

/* Activates the simulation driver; returns a value */
/* set by DriverInterrupt or 0 for termination      */
int DriverRun (double time)
{
  ACTIVITY *actptr;
  double oldsimtime;

  TRACE_DRIVER_run                     /* User activating simulation drivers */

  if (time > 0.0)                  /* Run for time units of time       */
    SimUpTo = YS__Simtime + time;
  else SimUpTo = -1.0;             /* Run until the event list is empty*/
  StopFlag = 0;                        /* Set to i by DriverRun()          */

  while (1)  {                         /* Start of main simulation loop    */
    /* The ready list consists of those activities at the head of
       the event list that are scheuduled for the current
       simulation time */

    while (YS__EventListHeadval() == YS__Simtime) {    /* The ready list is not empty  */
      actptr = (ACTIVITY*)YS__EventListGetHead();    /* Get the next activity */

/*if(YS__Simtime == 41.5)
{
	printf("EVENT fetched from queue: Cycle:%g\n", YS__Simtime);
	YS__EventListPrint();
}*/

      if (YS__interactive && YS__TrIter >= 0) {  /* Running under DBSIM     */
	if (YS__TrMode == EVENT_CHANGES) {     /* Mode is "event changes" */
	  YS__TrIter--;                      /* Another event has occurred   */
	  if (YS__TrIter == 0) {             /* All requested events occurred*/
#if 0 /*  Interactive mode not supported */
	    YS__UpdateTime(YS__Simtime);   /* Send time to DBSIM      */
	    YS__SendCommand(ACK);          /* Tell DBSIM last command is done*/
	    YS__GetCommand(0);             /* Get the next command from DBSIM*/
#endif
	  }
	}
      }

      /* activity must be activated by switching to it if it is a
         process or calling it if it is an event.  */


      if (actptr->type == EVTYPE        /* The activity is an event      */
	  || actptr->type == OSEVTYPE)
	{
	  YS__ActEvnt = (EVENT*)actptr;      /* An event is now active      */
	  TRACE_DRIVER_body2                 /* Initiating event ...        */
	  YS__ActEvnt->status = RUNNING; /* For statistics collection   */
	  if (YS__ActEvnt->statptr)          /* Activity stats collected    */
	    StatrecUpdate(YS__ActEvnt->statptr,(double)RUNNING,YS__Simtime);

	  (YS__ActEvnt->body)();             /* THE EVENT OCCURS            */

/*if(YS__Simtime == 41.5)
{
	printf("Return from event body: Cycle:%g\n", YS__Simtime);
	YS__EventListPrint();
}*/

	  YS__ActEvnt->status = LIMBO;       /* For statistics collection   */
	  if (YS__ActEvnt->statptr)          /* Activity stats collected    */
	    StatrecUpdate(YS__ActEvnt->statptr,(double)LIMBO,YS__Simtime);

	  if (YS__ActEvnt->deleteflag == DELETE)  {
	    TRACE_DRIVER_evterminate       /* Event terminating ...       */
	    TRACE_DRIVER_evdelete      /* Deleting event ...          */

//		if(YS__Simtime == 41.5 && actptr->id == 3214)
//		{
//			printf("\nBREAKPOINT: Before delete\n\n");
//		}

	    if (YS__ActEvnt->blkflg == BLOCK)  {          /* Parent was blocked  */
	      YS__errmsg("BlkFlg == BLOCK not allowed in this version");   /* Wake parent up      */
	    }

	    if (YS__ActEvnt->blkflg == FORK)  {    /* Parent forked this evnt      */
	      YS__errmsg("BlkFlg == FORK not allowed in this version");   /* Wake parent up      */
	    }

	    TRACE_DRIVER_body6                             /* Deleting event */
	    if (YS__ActEvnt->statptr != NULL) {        /* Free the statrec */
	      if (YS__ActEvnt->statptr->hist != NULL)/* Free the histogram */
		free(YS__ActEvnt->statptr->hist);
	      YS__PoolReturnObj(&YS__StatPool,YS__ActEvnt->statptr);
	    }

		int prev_id = actptr->id;

	    YS__PoolReturnObj(&YS__EventPool,YS__ActEvnt);     /* Free the event */
	  }

	  YS__ActEvnt = NULL;

	  if (StopFlag)  {                  /* Driver returns            */
	    TRACE_DRIVER_interrupt        /* Driver Interrupt occurs     */
	    return StopFlag;
	  }
	}
    }

    TRACE_DRIVER_empty                            /* --------- Ready List Empty */
		
/*	if(YS__Simtime == 41 || YS__Simtime == 41.5)
	{
		printf("im here cycle:%g\n", YS__Simtime);
		YS__EventListPrint();
	}*/

    if  (YS__EventListHeadval() >= 0)  {  /* Returns 0 if event list is empty*/
      oldsimtime = YS__Simtime;              /* Remember current simulation time   */
      YS__Simtime = YS__EventListHeadval();  /* Advance simulation time   */
      if (SimUpTo > 0.0 &&                   /* DriverRun called for SimUpTo time  */
	  YS__Simtime > SimUpTo)  {          /*    and it is there        */
	YS__Simtime = SimUpTo;             /* Restart at SimUpto point  */
	TRACE_DRIVER_body3                 /* TIME STEP COMPLETED       */
	return StopFlag;               /* Return from DriverRun     */
      }

      if (oldsimtime < YS__Simtime) {          /* Simulation really did advance      */
	/* If pending resources were scheduled at the current
	   simulation time, they would be on the event list now and
	   simultation time would not actually advance */

	if (YS__interactive && YS__TrIter >= 0) {  /* Running under DBSIM */
	  if (YS__TrMode == FOR_TIME || YS__TrMode == UNTIL_TIME) {
	    while (YS__Simtime > YS__TrTime) {   /* Stop if simtime >= TrTime */
#if 0 /* Interactive mode not supported */
	      YS__UpdateTime(YS__TrTime);/* Send time to DBSIM        */
	      YS__SendCommand(ACK);      /* Tell DBSIM last command is done   */
	      YS__GetCommand(0);         /* Get the next command from DBSIM   */
#endif
	      if (YS__TrMode == TIME_CHANGES ||   /* Mode changed     */
		  YS__TrMode == EVENT_CHANGES ||
		  YS__TrIter < 0)
		break;
	    }
	    TRACE_DRIVER_simtime     /* Driver Increasing Simulation Time  */
	  }
	  else if (YS__TrMode == TIME_CHANGES) {
	    YS__TrIter--;                        /* Completed another time change    */
	    TRACE_DRIVER_simtime                 /* Driver Increasing Simulation Time*/
	    if (YS__TrIter == 0) {           /* Run for requested time changes   */
#if 0 /* Interactive mode not supported */
	      YS__UpdateTime(YS__Simtime); /* Send time to DBSIM      */
	      YS__SendCommand(ACK);        /* Tell DBSIM last command is done  */
	      YS__GetCommand(0);           /* Get the next command from DBSIM  */
#endif
	    }
	  }
	  else if (YS__TrMode == EVENT_CHANGES) {
	    TRACE_DRIVER_simtime         /* Driver Increasing Simulation Time */
	  }
	}
	else
	  TRACE_DRIVER_simtime          /* Driver Increasing Simulation Time  */
      }
    }
    else  {                          /* Terminate DriverRun due to empty event list  */
      TRACE_DRIVER_body4           /* READY & EVENT LIST EMPTY            */
      return StopFlag;
    }
  }
}
