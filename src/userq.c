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

#include "../incl/simsys.h"
#include "../incl/dbsim.h"
#include "../incl/tr.userq.h"

/*****************************************************************************/
/* SEMAPHORE Operations: Implement the standard semaphore queue. Since       */
/* this code runs on a uniprocessor, the operations are not implemented in   */
/* an indivisible fashion.                                                   */
/*****************************************************************************/

SEMAPHORE *NewSemaphore(sname,i)  /* Creates & returns a pointer to a new semaphore   */
char *sname;                      /* User assigned name                               */
int i;                            /* Initial semaphore value                          */
{
   SEMAPHORE *semptr;

   PSDELAY;

   semptr = (SEMAPHORE*)YS__PoolGetObj(&YS__SemPool);
   semptr->id = YS__idctr++;
   strncpy(semptr->name,sname,31);
   semptr->name[31] = '\0';
   semptr->next = NULL;
   semptr->head = NULL;
   semptr->tail = NULL;
   semptr->type = SYNCQTYPE;
   semptr->size = 0;
   semptr->lengthstat = NULL;
   semptr->timestat = NULL;
   semptr->val = i;
   semptr->initval = i;
   TRACE_SEMAPHORE_new;        /* Creating semaphore with value                       */
   return semptr;
}

/**************************************************************************************/

void SemaphoreSignal(SEMAPHORE *sptr) /* Signals the semaphore, and continues. If activities are */
                           /* waiting at the semaphore, the one at the head of the    */
                           /* queue is released and the semaphore value is unchanged  */
                           /* If none are waiting, the semaphore value is incremented */
                           /* The queue is FIFO.                                      */
{
   ACTIVITY *aptr;

   PSDELAY;

   if (sptr->size == 0)  {                      /* Queue is empty                     */
      sptr->val++;                              /* increment value and continue       */
      TRACE_SEMAPHORE_v1p;                      /* ... signalling semaphore ...       */
   }
   else  {                                      /* Queue not empty                    */
      TRACE_SEMAPHORE_v2p;                      /* ... signalling semaphore ...       */
      aptr = (ACTIVITY*)YS__QueueGetHead((SYNCQUE *) sptr); /* Get head of queue                  */
      YS__RdyListAppend(aptr);                  /* and add it to the ready list       */
      TRACE_SEMAPHORE_v3;                       /* Actvity released ...               */
   }
}

/**************************************************************************************/

void SemaphoreSet(sptr)    /* Sets the semaphore's value to 1, and coninues.  If      */
                           /* activities are waiting at the semaphore, the one at the */
                           /* head of the queue is released and the semaphore value   */
                           /* is set to 0.  The queue is FIFO.                        */
SEMAPHORE *sptr;           /* Pointer to the semaphore                                */
{
   ACTIVITY *aptr;

   PSDELAY;

   if (sptr->val == 1) YS__warnmsg("Setting a set semaphore");
   if (sptr->size == 0)  {                       /* Queue is empty                    */
      sptr->val = 1;
      TRACE_SEMAPHORE_s1p;                       /* ... setting semaphore ...         */
   }
   else  {                                       /* Queue not empty                   */
      TRACE_SEMAPHORE_s2p;                       /* ... setting semaphore             */
      aptr = (ACTIVITY*)YS__QueueGetHead((SYNCQUE *) sptr);  /* Get head of queue                 */
      YS__RdyListAppend(aptr);                   /* and add it to the ready list      */
      sptr->val = 0;
      TRACE_SEMAPHORE_v3;                        /* Activity released ...             */
   }
}

/**************************************************************************************/

int SemaphoreDecr(sptr)  /* Decrements the semaphore and returns its new value        */
SEMAPHORE *sptr;         /* Pointer to the semaphore                                  */
{

   PSDELAY;

   if (sptr->val > 0) sptr->val--;   /* Semaphore values can not go negative          */
   return sptr->val;
}

/**************************************************************************************/

void SemaphoreWait(sptr) /* Waits on a semaphore.  If the value is > 0 decrement it   */
                         /* and continues, else suspends                              */
SEMAPHORE *sptr;         /* Pointer to the semaphore                                  */
{
  YS__errmsg("SemaphoreWait not supported in this version!");
}

/**************************************************************************************/

int SemaphoreValue(sptr)  /* Returns the value of the semaphore                       */
SEMAPHORE *sptr;          /* Pointer to the semaphore                                 */
{
   PSDELAY;

   return sptr->val;
}

/**************************************************************************************/

int SemaphoreWaiting(sptr)  /* Returns the # of activities in the queue               */
SEMAPHORE *sptr;            /* Pointer to the semaphore                               */
{
   PSDELAY;

   return sptr->size;
}


/**************************************************************************************/




/**************************************************************************************\
********************************* BARRIER Operations ***********************************
****************************************************************************************
**                                                                                    **
**          Barriers implement the standard barrier synchronizaiton operation         **
**          where several processes wait until they all reach the barrier.            **
**                                                                                    **
***************************************************************************************
\**************************************************************************************/

BARRIER *NewBarrier(bname,i)  /* Creates & returns a pointer to a new barrier         */
char *bname;                  /* User assigned name                                   */
int i;                        /* Initial barrier value                                */
{
   BARRIER *barptr;

   PSDELAY;

   barptr = (BARRIER*)YS__PoolGetObj(&YS__BarPool);
   barptr->id = YS__idctr++;
   strncpy(barptr->name,bname,31);
   barptr->name[31] = '\0';
   barptr->next = NULL;
   barptr->head = NULL;
   barptr->tail = NULL;
   barptr->size = 0;
   barptr->type = SYNCQTYPE;
   barptr->lengthstat = NULL;
   barptr->timestat = NULL;
   barptr->cnt = i;
   barptr->initcnt = i;
   TRACE_BARRIER_new;                 /* Creating new barrier ...                     */
   return barptr;
}

/**************************************************************************************/

int BarrierInit(bptr,i)    /* If a barrier's queue is empty, sets its value to i      */
                           /* Returns the size of the queue if i > 0 and -1 otherwise */
BARRIER *bptr;             /* Pointer to the barrier                                  */
int i;                     /* New value for the barrier                               */
{

   PSDELAY;

   TRACE_BARRIER_init;                  /* Intitializing barrier ...                  */
   if (bptr->size == 0 && i > 0) {      /* Only works if barrier queue is empty       */
      bptr->cnt = i;
      bptr->initcnt = i;
    }
   if (i > 0) return bptr->size;
   else return -1;                      /* Operation successful iff it returns 0      */
};

/**************************************************************************************/

void BarrierSync(bptr)        /* Waits at a barrier synchronization point             */
BARRIER *bptr;                /* Pointer to the barrier                               */
{

}

/**************************************************************************************/

int  BarrierNeeded(bptr)   /* Returns # of additional syncs needed to free barrier    */
BARRIER *bptr;             /* Pointer to the barrier                                  */
{
   PSDELAY;

   return bptr->cnt;
}

/**************************************************************************************/

int  BarrierWaiting(bptr)  /* Returns the number of processes waiting at the barrier  */
BARRIER *bptr;             /* Pointer to the barrier                                  */
{
   PSDELAY;

   return (bptr->initcnt - bptr->cnt);
}

/**************************************************************************************\
*********************************** FLAG Operations ************************************
****************************************************************************************
**                                                                                    **
**        Flags are similar to "events in other simulation systems.  Processes        **
**        can wait for flags to be set (events posted).                               **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

FLAG *NewFlag(fname)          /* Creates & returns a pointer to a new flag            */
char *fname;                  /* User assigned name                                   */
{
   FLAG *flgptr;

   PSDELAY;

   flgptr = (FLAG*)YS__PoolGetObj(&YS__FlagPool);
   flgptr->id = YS__idctr++;
   strncpy(flgptr->name,fname,31);
   flgptr->name[31] = '\0';
   flgptr->next = NULL;
   flgptr->head = NULL;
   flgptr->tail = NULL;
   flgptr->size = 0;
   flgptr->type = SYNCQTYPE;
   flgptr->lengthstat = NULL;
   flgptr->timestat = NULL;
   flgptr->state = CLEARED;
   TRACE_FLAG_new;              /* Creating new flag                                  */
   return flgptr;
}

/**************************************************************************************/

int FlagSet(fptr)            /* Sets a flag                                           */
FLAG *fptr;                  /* Pointer to the flag                                   */
{
   int num;
   ACTIVITY *actptr;

   PSDELAY;

   num = fptr->size;
   TRACE_FLAG_set1p;                              /* ... setting flag ...             */
   if (fptr->state == SET) YS__warnmsg("Setting a set flag");
   if (fptr->size > 0) {                          /* If activities are released       */
      fptr->state = CLEARED;                      /*   the flag is cleared            */
      while ((actptr = (ACTIVITY*)YS__QueueGetHead(fptr))  != NULL) {
         TRACE_FLAG_flgrelease
         YS__RdyListAppend(actptr);
      }
   }
   else fptr->state = SET;                         /* Otherwise the flag is set       */
   return num;                                     /* Return the queue size           */
}

/**************************************************************************************/

int FlagRelease(fptr)       /* Releases activities waiting at a flag                  */
FLAG *fptr;                 /* Pointer to the flag                                    */
{
   int num;
   ACTIVITY *actptr;

   PSDELAY;

   num = fptr->size;
   TRACE_FLAG_release1p;                   /* ... Releasing all actvities ...         */
   if (fptr->size > 0) {
      while ((actptr = (ACTIVITY*)YS__QueueGetHead(fptr))  != NULL) {
         TRACE_FLAG_flgrelease
         YS__RdyListAppend(actptr);
      }
   }
   fptr->state = CLEARED;                  /* The flag is cleared always              */
   return num;                             /* Return the queue size                   */
}

/**************************************************************************************/

void FlagWait(fptr)            /* Waits on a flag to be set or released               */
FLAG *fptr;                    /* Pointer to the flag                                 */
{
   ACTIVITY *actptr;

}

/**************************************************************************************/

int FlagWaiting(fptr)       /* Returns the # of activities in the queue               */
FLAG *fptr;                 /* Pointer to the flag                                    */
{

   PSDELAY;

   return fptr->size;
}

/**************************************************************************************\
************************************ STVAR Operations **********************************
****************************************************************************************
**                                                                                    **
**         State variables are used with conditions.  The operations on state         **
**         variables have side effects on conditions.  Whenever a condition           **
**         depends on the value of a state variable and that variable changes,        **
**         the condition is reevaluated.                                              **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

STVAR *NewIvar(sname,i)    /* Creates and returns a pointer to a new Ivar             */
char *sname;               /* User specified name                                     */
int i;                     /* Initial value of the Ivar                               */
{
   STVAR *svptr;

   PSDELAY;

   svptr = (STVAR*)YS__PoolGetObj(&YS__StvarPool);
   svptr->id = YS__idctr++;
   strncpy(svptr->name,sname,31);
   svptr->name[31] = '\0';
   svptr->type =IVARTYPE;
   svptr->CondList = NULL;         /* Not yet linked to any conditions                */
   svptr->val.ival = i;
   TRACE_IVAR_new;                 /* Creating integer valued state variable          */
   return svptr;
}

/**************************************************************************************/

STVAR *NewFvar(sname,x)    /* Creates and returns a pointer to a new Fvar             */
char *sname;               /* User specified name                                     */
double x;                  /* Initial value of the Fvar                               */
{
   STVAR *svptr;

   PSDELAY;

   svptr = (STVAR*)YS__PoolGetObj(&YS__StvarPool);
   svptr->id = YS__idctr++;
   strncpy(svptr->name,sname,31);
   svptr->name[31] = '\0';
   svptr->type =FVARTYPE;
   svptr->CondList = NULL;         /* Not yet linked to any condition                 */
   svptr->val.fval = x;
   TRACE_FVAR_new;                 /* Creating real valued state variable             */
   return svptr;
}

/**************************************************************************************/

int YS__SvId(svptr)        /* Returns the system defined Id or 0 if TraceId is 0      */
STVAR *svptr;              /* Pointer to the state variable                           */
{
   if (TraceIDs) return svptr->id;
   else return 0;
}

/**************************************************************************************/

void Iset(svptr,i)         /* Sets the value of an Ivar                               */
STVAR *svptr;              /* Pointer to the Ivar                                     */
int   i;                   /* New value of the Ivar                                   */
{
   QELEM *qeptr = svptr->CondList;
   CONDITION *cptr;
   ACTIVITY *actproc;

   PSDELAY;

   if (svptr->type != IVARTYPE) YS__errmsg("Iset only works with IVAR's");
   svptr->val.ival = i;

   TRACE_STVAR_set;              /* State variable modified, new value ...            */
   if (qeptr == NULL)
      YS__warnmsg("Setting an IVAR that is not linked to a Condition");

   while (qeptr != NULL)  {      /* For each conditon that depends on *svptr          */
      cptr = (CONDITION*)(qeptr->optr);
      cptr->state = cptr->body(cptr->argptr,cptr->argsize);
      TRACE_STVAR_evalcond;      /* Evaluating condition ...                          */
      if (cptr->state == 1 && cptr->size >0)  /* Condition true & activities waiting  */
         while ((actproc = (ACTIVITY*)YS__QueueGetHead(qeptr->optr)) != NULL) {
            TRACE_STVAR_condrelease;         /* Activity released from condition      */
            YS__RdyListAppend(actproc);      /* and added to the ready list           */
         }
      qeptr = qeptr->next;
   }
}

/**************************************************************************************/

void Fset(svptr,x)         /* Sets the value of an Fvar                               */
STVAR *svptr;              /* Pointer to the Fvar                                     */
double  x;                 /* New value of the Fvar                                   */
{
   QELEM *qeptr = svptr->CondList;
   ACTIVITY *actproc;
   CONDITION *cptr;

   PSDELAY;

   if (svptr->type != FVARTYPE) YS__errmsg("Fset only works with FVAR's");
   svptr->val.fval = x;

   if (svptr != SIMTIME) {
      TRACE_STVAR_set;              /* State variable modified, new value ...         */
      if (qeptr == NULL)
         YS__warnmsg("Setting an IVAR that is not linked to a Condition");
   }
   while (qeptr != NULL)  {      /* For each conditon that depends on *svptr          */
      cptr = (CONDITION*)(qeptr->optr);
      cptr->state = cptr->body(cptr->argptr,cptr->argsize);
      TRACE_STVAR_evalcond;      /* Evaluating condition ...                          */
      if (cptr->state == 1 && cptr->size >0)  /* Condition true & activities waiting  */
         while ((actproc = (ACTIVITY*)YS__QueueGetHead(qeptr->optr)) != NULL) {
            TRACE_STVAR_condrelease;         /* Activity released from condition      */
            YS__RdyListAppend(actproc);      /* and added to the ready list           */
         }
      qeptr = qeptr->next;
   }
}

/**************************************************************************************/

int Ival(svptr)            /* Returns the value of an Ivar                            */
STVAR *svptr;              /* Pointer to the Ivar                                     */
{

   PSDELAY;

   return svptr->val.ival;
}

/**************************************************************************************/

double Fval(svptr)         /* Returns the value of an Fvar                            */
STVAR *svptr;              /* Pointer to the Fvar                                     */
{

   PSDELAY;

   return svptr->val.fval;
}

/**************************************************************************************\
********************************** CONDITION Operations*********************************
****************************************************************************************
**                                                                                    **
**        Conditions are logical expressions involving state variables. Processes     **
**        can sychronize on a condition by waiting until it becomes true.             **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/


/**************************************************************************************\
********************************** RESOURCE Operations *********************************
****************************************************************************************
**                                                                                    **
**       Resources contain queues and servers.  Activities can request service        **
**       from resources.  They wait in the queue until a server is available.         **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

static char* qdiscname[11] = { "RDYLST","FCFS","FCFSPRWP","LCFSPR","PROCSHAR","RR",
                               "RAND","LCFSPRWP","SJN","RRPRWP","LCFS"};

/**************************************************************************************/

RESOURCE *NewResource(qname,disc,nserv,slice)     /* Creates a new resource           */
char *qname;                                      /* User defined name                */
int disc;                                         /* The queueing discipline          */
int nserv;                                        /* Number of servers                */
double slice;                                     /* Time slice for RR discipline     */
{
   RESOURCE *rptr;

   rptr = (RESOURCE*)YS__PoolGetObj(&YS__ResPool); /* Get the resource's structure    */

   /* Initialize the fields common to all queues */

   rptr->id = YS__idctr++;                         /* Give it a unique ID             */
   strncpy(rptr->name,qname,31);                   /* User assigned name for tracing  */
   rptr->name[31] = '\0';                          /* Names are limited to 31 chars   */
   rptr->next = NULL;                              /* Queue is empty initially        */
   rptr->head = NULL;
   rptr->tail = NULL;
   rptr->size = 0;
   rptr->type = RESTYPE;                           /* Used by general queue functions */

   /* Initialize the fields unique to resources */

   rptr->qdisc = disc;        /* The queueing discipline used for entering activities */
   rptr->servers = nserv;     /* The number of servers available                      */
   rptr->serving = NULL;      /* Activity getting service; used only in PR resources  */
   rptr->status = QUIESCENT;  /* PENDING when waiting changes to take effect          */
   rptr->timeslice =slice;    /* Used for interleaving service; used only in RR       */
   rptr->oldsize = 0;         /* Used to adjust times in PROCSHAR resources           */
   rptr->lengthstat = NULL;   /* Pointer to queue length statrec                      */
   rptr->timestat = NULL;     /* Pointer to time in queue statrec                     */
   rptr->utilstat = NULL;     /* Pointer to server utilization statrec                */
   TRACE_RESOURCE_new;        /* Creating resource ...                                */

   /* Set up for different queueing disciplines */

   if (disc == FCFS && slice > 0.0)
           YS__warnmsg("Time slice only used with round robin queue discipline");

   if (disc == LCFS && slice > 0.0)
           YS__warnmsg("Time slice only used with round robin queue discipline");

   if (disc == FCFSPRWP) {
      if (nserv > 1) {
         rptr->servers = 1;
         YS__warnmsg("Preemptive Resume resources will only use one server.");
      }
      if (slice > 0.0)
         YS__warnmsg("Time slice only used with round robin queue disciplines");
   }

   if (disc == LCFSPRWP) {
      if (nserv > 1) {
         rptr->servers = 1;
         YS__warnmsg("Preemptive Resume resources will only use one server.");
      }
      if (slice > 0.0)
         YS__warnmsg("Time slice only used with round robin queue disciplines");
   }

   if (disc == PROCSHAR)  {
      if (nserv > 1) {
         rptr->servers = 1;
         YS__warnmsg("Processor sharing resources will only use one server.");
      }
      if (slice > 0.0)
         YS__warnmsg("Time slice only used with round robin queue disciplines");
   }

   if (disc == LCFSPR)  {
      if (nserv > 1) {
         rptr->servers = 1;
         YS__warnmsg("Preemptive Resume resources will only use one server.");
      }
      if (slice > 0.0)
         YS__warnmsg("Time slice only used with round robin queue disciplines");
   }

   if (disc == SJN) {
      if (nserv > 1) {
         rptr->servers = 1;
         YS__warnmsg("Preemptive Resume resources will only use one server.");
      }
      if (slice > 0.0)
         YS__warnmsg("Time slice only used with round robin queue disciplines");
   }

   if (disc == RAND && slice > 0.0)
      YS__warnmsg("Time slice only used with round robin queue disciplines");

   if (disc == RRPRWP && nserv > 1) {
      rptr->servers = 1;
      YS__warnmsg("Preemptive Resume resources will only use one server.");
   }

   rptr->freeservers = rptr->servers;           /* Servers free initially             */

   return rptr;
}

/**************************************************************************************/

void ResourceUse(rptr,timeinc)      /* Requests service from a resource                 */
RESOURCE *rptr;                     /* Pointer to the resource                          */
double   timeinc;                   /* Amount of service requested                      */
{
   TRACE_RESOURCE_use1;             /* Process requests service from resource           */

}

/**************************************************************************************/

void YS__RscInsert(rptr,aptr)   /* Inserts an activity in a resource                  */
RESOURCE *rptr;                 /* Pointer to the resource                            */
ACTIVITY *aptr;                 /* Pointer to the activity to add                     */
{
   aptr->status = WAIT_RES;             /* Activity now waiting for service           */
   if (aptr->statptr)                   /* Statistics collected for this activity     */
      StatrecUpdate(aptr->statptr,(double)WAIT_RES,YS__Simtime);

   if (rptr->qdisc == FCFS) {           /* First Come First Served ********************/
      YS__QueuePutTail(rptr,aptr);      /* Add activity at tail of the queue          */
      aptr->enterque = YS__Simtime;     /* Record time the activity entered the queue */
   }

   else if (rptr->qdisc == LCFS) {      /* Last Come First Served *********************/
      YS__QueuePutHead(rptr,aptr);      /* Add the activity at the head of the queue  */
      aptr->enterque = YS__Simtime;     /* Record time the activity entered the queue */
   }

   else if (rptr->qdisc == LCFSPR) {    /* LCFS Preemptive Resume *********************/
      YS__QueuePutHead(rptr,aptr);      /* Add the activity at the head of the queue  */
      aptr->enterque = YS__Simtime;     /* Record time the activity entered the queue */
   }

   else if (rptr->qdisc == FCFSPRWP) {  /* FCFS Preemptive Resume With Priorities *****/
//      YS__QueuePrinsert(rptr,aptr);     /* Enter by priority, FCFS if equal           */
      aptr->enterque = YS__Simtime;     /* Record time the activity entered the queue */
   }

   else if (rptr->qdisc == LCFSPRWP) {  /* LCFS Preemptive Resume With Priorities *****/
//      YS__QueuePrenter(rptr,aptr);      /* Enter by priority, LCFS if equal           */
      aptr->enterque = YS__Simtime;     /* Record time the activity entered the queue */
   }

   else if (rptr->qdisc == RR) {        /* Round Robin ********************************/
      aptr->timeleft = aptr->time;      /* Service time remaining                     */
      YS__QueuePutTail(rptr,aptr);      /* Add the activity at the tail of the queue  */
      aptr->enterque = YS__Simtime;     /* Record time the activity entered the queue */
   }

   else if (rptr->qdisc == RAND) {      /* Random *************************************/
      aptr->priority = YacRand();       /* Implemented by using a random priority     */
//      YS__QueuePrinsert(rptr,aptr);     /* Enter by priority, FCFS if equal           */
      aptr->enterque = YS__Simtime;     /* Record time the activity entered the queue */
   }

   else if (rptr->qdisc == PROCSHAR) {  /* Processor Sharing **************************/
      if (rptr->head == NULL)           /* Can't use general queue insertion routines */
         rptr->head = aptr;             /*    because activities in this resource are */
      else rptr->tail->rscnext = aptr;  /*    in the queue and the ready list at the  */
      rptr->tail = aptr;                /*    same time and need two next fields.     */
      rptr->tail->rscnext = NULL;       /*    "next" is used as always for the evlist */
      rptr->size++;                     /*    rscnext is used for the resource queue. */
      TRACE_QUEUE_show(rptr);           /* Prints the queue contents if enabled       */
      aptr->enterque = YS__Simtime;     /* Record time the activity entered the queue */
   }

   else if (rptr->qdisc == SJN) {       /* Shortest Job Next **************************/
      YS__QueueInsert(rptr,aptr);       /* Enter by priority, FCFS if equal           */
      aptr->enterque = YS__Simtime;     /* Record time the activity entered the queue */
   }

   else if (rptr->qdisc == RRPRWP) {    /* RR Preemptive Resume With Priority *********/
      aptr->timeleft = aptr->time;      /* Service time remaining                     */
 //     YS__QueuePrinsert(rptr,aptr);     /* Enter by priority, FCFS if equal           */
      aptr->enterque = YS__Simtime;     /* Record time the activity entered the queue */
   }

   else if (rptr->qdisc == RDYLIST) {   /* RRPRWP queue used as a cpu's ready list ****/
      aptr->timeleft = aptr->time;
//      YS__QueuePrinsert(rptr,aptr);     /* Enter by priority, FCFS if equal           */
      aptr->enterque = YS__Simtime;     /* Record time the activity entered the queue */
   }

   else YS__errmsg("Unimplemented queuing discipline");

   if ( rptr->status != PENDING)  {      /* All activities requesting service are put */
      rptr->status = PENDING;            /*   in the queue at this point.  Then the   */
      YS__QueuePutTail(YS__PendRes,rptr);/*   the queue is evaluated just before time */
   }                                     /*   advance to see which gets service next. */

   if (rptr->lengthstat) {               /* Queue length statistics are collected     */
      if (rptr->qdisc == PROCSHAR)       /* All activities are always in the queue    */
         StatrecUpdate(rptr->lengthstat,(double)(rptr->size),YS__Simtime);
      else {                             /* Some are in the queue and some are being  */
         StatrecUpdate(rptr->lengthstat, /*    serviced; all are counted.             */
            (double)(rptr->size + rptr->servers - rptr->freeservers),
            YS__Simtime);
      }
   }
}

/**************************************************************************************/

int ResourceWaiting(rptr)   /* Returns the # of activities waiting for service        */
RESOURCE *rptr;             /* Pointer to the resource                                */
{
   return rptr->size;
}

/**************************************************************************************/

int ResourceServicing(rptr) /* Returns the # of activities getting service            */
RESOURCE *rptr;             /* Pointer to the resource                                */
{
   return rptr->servers - rptr->freeservers;
}

/**************************************************************************************/

void YS__RscActivate(rptr)   /* Moves activities from the resource to the YS__EvntLst */
RESOURCE *rptr;              /* Pointer to the resource                               */

/* When an activity is scheduled to use a resource, it is put in the resource's queue */
/* and the resource is marked as PENDING.  Just before simulation time is to be       */
/* advanced, all PENDING resources are evaluated.  Each one is checked to find        */
/* any activities that should be preempted or scheduled.  This routine performs       */
/* these operations and is called by the driver before increasing simulation time,    */
/* if there are any PENDING resources.  If any activities from a resource queue are   */
/* scheduled in the event list at the current time, this will prevent simulation time */
/* from advancing.  As a consequence of all this, for preemptive queue, if            */
/* ResourceUse() is invoked more than once at the same time, it will be the activity  */
/* with the highest priority that calls ResourceUse()  most recently in real time     */
/* that gets service.                                                                 */

{
   ACTIVITY *actptr;

   if (rptr->qdisc == FCFS || rptr->qdisc == LCFS)  {

      while (rptr->freeservers > 0 &&                /* There is a free server        */
             rptr->size > 0)  {                      /* and an activity to schedule   */
         actptr = (ACTIVITY*)YS__QueueGetHead(rptr); /* Get the activity from the q   */
         TRACE_RESOURCE_activate1;                   /* Activity resumes service ...  */
         actptr->time = YS__Simtime + actptr->time;  /* Service end time              */
         actptr->status = USING_RES;                 /* For statistics collection     */
         if (actptr->statptr)                        /* Activity stats collected      */
            StatrecUpdate(actptr->statptr,(double)USING_RES,YS__Simtime);
         YS__EventListInsert(actptr);                /* Schedule the activity         */
         rptr->freeservers--;                        /* There is one fewer server     */
         if (rptr->utilstat)                         /* Utilization stats collected   */
            StatrecUpdate(rptr->utilstat,            /* One more server busy          */
                          (double)(rptr->servers-rptr->freeservers),
                          YS__Simtime);
      }
   }

   else if (rptr->qdisc == LCFSPR && rptr->size > 0)  {

      actptr = (ACTIVITY*)YS__QueueGetHead(rptr);    /* Get the head of the queue     */
      if (rptr->serving != NULL) {                   /* Is the server busy?           */
         TRACE_RESOURCE_activate2;                   /* Prempting activity            */
         YS__EventListDelete(rptr->serving);         /* Get it out of the event list  */
         rptr->freeservers++;                        /* One free server now           */
         if (rptr->utilstat)                         /* Util stats collected          */
            StatrecUpdate(rptr->utilstat,            /* Utilization decremented       */
                    (double)(rptr->servers-rptr->freeservers),
                    YS__Simtime);
         rptr->serving->time =                       /* Preempted activity has not    */
            rptr->serving->time - YS__Simtime;       /*   finished all its service    */
         rptr->serving->status = WAIT_RES;           /* For statistics collection     */
         if (rptr->serving->statptr)                 /* Activity statistics collected */
            StatrecUpdate(rptr->serving->statptr,(double)WAIT_RES,YS__Simtime);
         YS__QueuePutHead(rptr,rptr->serving);       /* Preempted activity returned   */
      }                                              /*     to the queue              */
      TRACE_RESOURCE_activate1;                      /* Activity resumes service      */
      actptr->time = YS__Simtime + actptr->time;     /* Service end time              */
      actptr->status = USING_RES;                    /* For statistics collection     */
      if (actptr->statptr)                           /* Activity statistics collected */
         StatrecUpdate(actptr->statptr,(double)USING_RES,YS__Simtime);
      YS__EventListInsert(actptr);                   /* Schedule the new activity     */
      rptr->freeservers--;                           /* Server now busy               */
      if (rptr->utilstat)                            /* Utilization stats collected   */
         StatrecUpdate(rptr->utilstat,
                       (double)(rptr->servers-rptr->freeservers),
                       YS__Simtime);
      rptr->serving = actptr;                        /* Identifies serviced activity  */
   }

   else if (rptr->qdisc == FCFSPRWP && rptr->size > 0) {

      actptr = (ACTIVITY*)YS__QueueGetHead(rptr);   /* Get the head of the queue      */
      if (rptr->serving != NULL                     /* The server is busy with lower  */
            && actptr->priority > rptr->serving->priority) { /* priority activit      */
         TRACE_RESOURCE_activate2;                  /* Preempting activity ...        */
         YS__EventListDelete(rptr->serving);        /* Get it out of the event list   */
         rptr->freeservers++;                       /* Server now idle                */
         if (rptr->utilstat)                        /* Utilization stats collected    */
            StatrecUpdate(rptr->utilstat,           /* Utilization decremented        */
                          (double)(rptr->servers-rptr->freeservers),
                          YS__Simtime);
         rptr->serving->time =                      /* Preempted activity has not     */
            rptr->serving->time - YS__Simtime;      /*   finished all its service     */
         rptr->serving->status = WAIT_RES;          /* For statistics collection      */
         if (rptr->serving->statptr)                /* Activity statistics collected  */
            StatrecUpdate(rptr->serving->statptr,(double)WAIT_RES,YS__Simtime);
//         YS__QueuePrenter(rptr,rptr->serving);      /* Preempted activity returned to */
         rptr->serving = NULL;                      /*    the queue                   */
      }
      if (rptr->serving == NULL) {                  /* NULL only if preemption occurs */
         TRACE_RESOURCE_activate1;                  /* Activity resumes service ...   */
         actptr->time = YS__Simtime + actptr->time; /* Service end time               */
         actptr->status = USING_RES;                /* For statistics collection      */
         if (actptr->statptr)                       /* Activity statistics collected  */
            StatrecUpdate(actptr->statptr,(double)USING_RES,YS__Simtime);
         YS__EventListInsert(actptr);               /* Schedule the new activity      */
         rptr->freeservers--;                       /* Server now busy                */
         if (rptr->utilstat)                        /* Utilization stats collected    */
            StatrecUpdate(rptr->utilstat,
                          (double)(rptr->servers-rptr->freeservers),
                          YS__Simtime);
         rptr->serving = actptr;                    /* Identifies service activity    */
      }
      else  {                                       /* Activity not preempted         */
         YS__QueuePutHead(rptr,actptr);             /* Put new activity back in queue */
      }
   }

   else if (rptr->qdisc == LCFSPRWP && rptr->size > 0)  {

      actptr = (ACTIVITY*)YS__QueueGetHead(rptr);   /* Get the head of the queue      */
      if (rptr->serving != NULL                     /* The server is busy with lower  */
        && actptr->priority >= rptr->serving->priority)  { /* priority activity       */
         TRACE_RESOURCE_activate2;                  /* Preempting activity ...        */
         YS__EventListDelete(rptr->serving);        /* Get it out of the event list   */
         rptr->freeservers++;                       /* Server now idle                */
         if (rptr->utilstat)                        /* Utilization stats collected    */
            StatrecUpdate(rptr->utilstat,           /* Utilizatin decrementd          */
                          (double)(rptr->servers-rptr->freeservers),
                          YS__Simtime);
         rptr->serving->time =                      /* Preempted activity has not     */
            rptr->serving->time - YS__Simtime;      /*   finished all its service     */
         rptr->serving->status = WAIT_RES;          /* For statistics collection      */
         if (rptr->serving->statptr)                /* Activity statistics collected  */
            StatrecUpdate(rptr->serving->statptr,(double)WAIT_RES,YS__Simtime);
//         YS__QueuePrenter(rptr,rptr->serving);      /* Preempted activity returned to */
         rptr->serving = NULL;                      /*   the queue                    */
      }
      if (rptr->serving == NULL) {                  /* NULL only if preemption occurs */
         TRACE_RESOURCE_activate1;                  /* Activity resumes service ...   */
         actptr->time = YS__Simtime + actptr->time; /* Service end time               */
         actptr->status = USING_RES;                /* For statistics collection      */
         if (actptr->statptr)                       /* Activity statistics collected  */
            StatrecUpdate(actptr->statptr,(double)USING_RES,YS__Simtime);
         YS__EventListInsert(actptr);               /* Schedule the new activity      */
         rptr->freeservers--;                       /* Server now busy                */
         if (rptr->utilstat)                        /* Utilization stats collected    */
            StatrecUpdate(rptr->utilstat,
                          (double)(rptr->servers-rptr->freeservers),
                          YS__Simtime);
         rptr->serving = actptr;                    /* Identifies service activity    */
      }
      else  {                                       /* Activity not preempted         */
         YS__QueuePutHead(rptr,actptr);             /* Put new activity back in queue */
      }
   }

   else if (rptr->qdisc == RR && rptr->size > 0)  {

      while (rptr->freeservers > 0 &&                      /* There are servers free  */
             rptr->size > 0)  {                            /*   and work to do        */
         actptr = (ACTIVITY*)YS__QueueGetHead(rptr);       /* Get activity from queue */
         TRACE_RESOURCE_activate1;                         /* Activity resumes ...    */
         if (rptr->timeslice < actptr->timeleft) {         /* Needs more than 1 slice */
            actptr->time = YS__Simtime + rptr->timeslice;  /* End of next slice       */
            actptr->timeleft = actptr->timeleft - rptr->timeslice; /* New time left   */
         }
         else {                                            /* Needs <= one slice      */
            actptr->time = YS__Simtime + actptr->timeleft; /* Time service ends       */
            actptr->timeleft = 0.0;                        /* That finishes it        */
         }
         actptr->status = USING_RES;                       /* For stats collection    */
         if (actptr->statptr)                              /* Collect activity stats  */
            StatrecUpdate(actptr->statptr,(double)USING_RES,YS__Simtime);
         YS__EventListInsert(actptr);                      /* Schedule the new act.   */
         rptr->freeservers--;                              /*   on a free server      */
         if (rptr->utilstat)                               /* Collect util. stats     */
            StatrecUpdate(rptr->utilstat,
                          (double)(rptr->servers-rptr->freeservers),
                          YS__Simtime);
      }
   }

   else if (rptr->qdisc == RAND && rptr->size > 0)  {

      while (rptr->freeservers > 0 &&                      /* There are servers free  */
             rptr->size > 0)  {                            /*   and work to do        */
         actptr = (ACTIVITY*)YS__QueueGetHead(rptr);       /* Get activity from queue */
         TRACE_RESOURCE_activate1;                         /* Activity resumes ...    */
         actptr->time = YS__Simtime + actptr->time;        /* Time service ends       */
         actptr->status = USING_RES;                       /* For stats collection    */
         if (actptr->statptr)                              /* Collect activity stats  */
            StatrecUpdate(actptr->statptr,(double)USING_RES,YS__Simtime);
         YS__EventListInsert(actptr);                      /* Schedule the new act.   */
         rptr->freeservers--;                              /*   on a free server      */
         if (rptr->utilstat)                               /* Collect util. stats     */
            StatrecUpdate(rptr->utilstat,
                          (double)(rptr->servers-rptr->freeservers),
                          YS__Simtime);
      }
   }

   else if (rptr->qdisc == PROCSHAR && rptr->size > 0)  {

      /* Activities requesting service from a PROCSHAR resouces are put in both the   */
      /* resource's queue and the evenlist.  Therefore they need two next pointers.   */
      /* the standard pointer "next" is used for the event list while the special     */
      /* pointer "rscnext" is used for the resource's queue.  Each time a new         */
      /* activity is added, the remaining service times for all the activites in the  */
      /* resource are adjusted so that the total time for all is alloted evenly.      */

      ACTIVITY *prptr;

      prptr = rptr->head;                     /* Get the head activity of the queue   */
      TRACE_RESOURCE_activate4;               /* Adjusting service times ...          */
      while (prptr != NULL) {                 /* Take every activity out of the event */
         if (YS__EventListDelete(prptr)) {    /*   list and adjust its time           */
            prptr->time =                     /* This way for activites on the evlst  */
               ((prptr->time - YS__Simtime)/rptr->oldsize) * rptr->size + YS__Simtime;
         }
         else                                 /* This way for new not yet on evlst    */
            prptr->time = (prptr->time) * rptr->size + YS__Simtime;
         TRACE_RESOURCE_activate3;            /* Activity resumes service ...         */
         prptr->status = USING_RES;           /* For statistics collection            */
         if (prptr->statptr)                  /* Activity stats collected             */
            StatrecUpdate(prptr->statptr,(double)USING_RES,YS__Simtime);
         YS__EventListInsert(prptr);          /* Return or enter act in event list    */
         prptr = prptr->rscnext;              /* and at the tail of the resource que  */
      }
      rptr->oldsize = rptr->size;             /* Remember the new queue size          */
      if (rptr->utilstat)                     /* Utilization stats collected          */
         StatrecUpdate(rptr->utilstat,(double)(rptr->size),YS__Simtime);
   }

   else if (rptr->qdisc  == SJN)  {

      while (rptr->freeservers > 0 &&                 /* There are free servers       */
             rptr->size > 0)  {                       /*   and work to do             */
         actptr = (ACTIVITY*)YS__QueueGetHead(rptr);  /* Get activity from the queue  */
         TRACE_RESOURCE_activate5;                    /* Activity resumes ...         */
         actptr->time = YS__Simtime + actptr->time;   /* Time service ends            */
         actptr->status = USING_RES;                  /* For stat collection          */
         if (actptr->statptr)                         /* Activity stats collected     */
            StatrecUpdate(actptr->statptr,(double)USING_RES,YS__Simtime);
         YS__EventListInsert(actptr);                 /* Schedule new activity on a   */
         rptr->freeservers--;                         /*   on a free server           */
         if (rptr->utilstat)                          /* Utilization stats collected  */
            StatrecUpdate(rptr->utilstat,
                          (double)(rptr->servers-rptr->freeservers),
                          YS__Simtime);
      }
   }

   else if ( (rptr->qdisc == RRPRWP || rptr->qdisc == RDYLIST) && rptr->size > 0)
   {
      actptr = (ACTIVITY*)YS__QueueGetHead(rptr);   /* Get the head of the queue      */
      if (rptr->serving != NULL                     /* The server is busy             */
            && actptr->type != OSPRTYPE             /* Head not OS Process            */
            && actptr->type != OSEVTYPE             /* Head not OS Event              */
            && actptr->priority > rptr->serving->priority)  /* Peempting activity     */
      {                                             /* has higher priority            */
         TRACE_RESOURCE_activate2;                  /* Preempting activity ....       */
         YS__EventListDelete(rptr->serving);        /* Get the serviced activity      */
         rptr->freeservers++;                       /*   freeing up the server        */
         if (rptr->utilstat)                        /* Utilization stats collected    */
            StatrecUpdate(rptr->utilstat,
                          (double)(rptr->servers-rptr->freeservers),
                          YS__Simtime);
         if (rptr->qdisc == RDYLIST) {              /* If resource used as cpu rdylst */
            YS__BusyPrcrs--;                        /*   the cpu becomes idle         */
            if (YS__BusyPrcrStat)                   /* CPU stats collected            */
               StatrecUpdate (YS__BusyPrcrStat, (double) YS__BusyPrcrs, YS__Simtime);
         }
         rptr->serving->time =                      /* Recompute the service time     */
            rptr->serving->timeleft +               /*   remaining for the preempted  */
            rptr->serving->time - YS__Simtime;      /*   activity                     */
         rptr->serving->status = WAIT_RES;          /* For statistics collection      */
         if (rptr->serving->statptr)                /* Activity stats collected       */
            StatrecUpdate(rptr->serving->statptr,(double)WAIT_RES,YS__Simtime);
//         YS__QueuePrenter(rptr,rptr->serving);      /* Put preempted activity in que  */
         rptr->serving = NULL;                      /* Deactivate server              */
      }
      if (rptr->serving == NULL) { /* Fails only if server busy with higher priority  */
         TRACE_RESOURCE_activate1;                  /* Activity resumes service       */
         if (rptr->timeslice < actptr->timeleft) {  /* More than time slice left      */
            actptr->time =                          /* Schedule another slice         */
               YS__Simtime + rptr->timeslice;
            actptr->timeleft = actptr->timeleft - rptr->timeslice;
         }
         else {                                      /* Less than time slice left     */
            actptr->time =                           /* Scheduled time = time left    */
               YS__Simtime + actptr->timeleft;
            actptr->timeleft = 0.0;
         }
         actptr->status = USING_RES;                  /* For statistics collection    */
         if (actptr->statptr)                         /* Activity stats collected     */
            StatrecUpdate(actptr->statptr,(double)USING_RES,YS__Simtime);
         YS__EventListInsert(actptr);                 /* Schedule the new process     */
         rptr->freeservers--;                         /*   on the server              */
         if (rptr->utilstat)                          /* Utilization stats collected  */
            StatrecUpdate(rptr->utilstat,
                          (double)(rptr->servers-rptr->freeservers),
                          YS__Simtime);
         if (rptr->qdisc == RDYLIST) {                /* If resource used as a cpu    */
            YS__BusyPrcrs++;                          /* If is now busy again         */
            if (YS__BusyPrcrStat)                     /* CPU stats collected          */
               StatrecUpdate (YS__BusyPrcrStat, (double) YS__BusyPrcrs, YS__Simtime);
         }
         rptr->serving = actptr;                       /* Points to service actiity   */
      }
      else  { /* Activiy requesting service fail due to preempt due to lower priority */
         YS__QueuePutHead(rptr,actptr);                /* Put it in the queue         */
      }
   }

   else YS__errmsg("Unimplemented queuing discipline");
}


/**************************************************************************************/

int YS__RscContinue(aptr)  /* Returns an activity to a resource after a time slice    */
ACTIVITY *aptr;            /* Pointer to the activity                                 */
{
   /* The routine first checks to see if the activity is using a resource and whether */
   /* or not it has received all its requested service.  */

   RESOURCE *rptr = aptr->rscptr;

   if (aptr->rscptr == NULL) return 1;       /* The activity is not using a resource  */
                                             /* This should never happen, see driver  */
   if (rptr->qdisc == RR && aptr->timeleft > 0.0) { /* Activity is in a RR resource   */
      TRACE_RESOURCE_use4p;          /* Activity completes slice from res/procr ...   */
      aptr->time = aptr->timeleft;           /* Put it back in the resource's queue   */
      YS__QueuePutTail(rptr,aptr);           /* with a request for the remaining time */
      rptr->freeservers++;                   /* One more free server                  */

      aptr->status = WAIT_RES;               /* For statistics collection             */
      if (aptr->statptr)                     /* Activity stats collected              */
         StatrecUpdate(aptr->statptr,(double)WAIT_RES,YS__Simtime);
      if (rptr->utilstat)                    /* Server utilization stats collected    */
         StatrecUpdate(rptr->utilstat,
            (double)(rptr->servers - rptr->freeservers), YS__Simtime);

      if ( rptr->status != PENDING)  {       /* Activities requesting service are put */
         rptr->status = PENDING;             /* in the queue at this point. Then the  */
         YS__QueuePutTail(YS__PendRes,rptr); /* queue is evaluated just before time   */
      }                                      /* advances to see who gets service next */
      return 0;
   }

   if ( (aptr->timeleft > 0.0) &&    /* Activity is in a RRPRWP or RDYLIST resource   */
        (rptr->qdisc == RRPRWP || rptr->qdisc == RDYLIST)) { /* & needs more service  */
      TRACE_RESOURCE_use4p;          /* Activity completes slice from res/procr ...   */
      aptr->time = aptr->timeleft;   /* Put it back in the resource's queue with a    */
//      YS__QueuePrinsert(rptr,aptr);  /* request of the remaining service time         */
      rptr->freeservers++;           /* The server is now free                        */
      rptr->serving = NULL;

      aptr->status = WAIT_RES;       /* For statistics collection                     */
      if (aptr->statptr)             /* Activity statistics collected                 */
         StatrecUpdate(aptr->statptr,(double)WAIT_RES,YS__Simtime);
      if (rptr->utilstat)            /* Server utilization statistics colledted       */
         StatrecUpdate(rptr->utilstat,
            (double)(rptr->servers - rptr->freeservers), YS__Simtime);
      if (rptr->qdisc == RDYLIST) {         /* Resource is a cpu's ready list         */
         YS__BusyPrcrs--;                   /* One more idle processor                */
         if (YS__BusyPrcrStat)              /* Procssor utilization stats collected   */
            StatrecUpdate (YS__BusyPrcrStat, (double) YS__BusyPrcrs, YS__Simtime);
      }

      if ( rptr->status != PENDING)  {       /* Activities requesting service are put */
         rptr->status = PENDING;             /* in the queue at this point. Then the  */
         YS__QueuePutTail(YS__PendRes,rptr); /* queue is evaluated just before time   */
      }                                      /* advances to see who gets service next */
      return 0;
   }

   if (rptr->qdisc == PROCSHAR) { /* Activity is in a processor sharing resource      */
      /* Since PROCSHAR resources are not sliced, the activity is finished with the   */
      /* queue, but it is still in the resource's queue and must be taken out         */

      ACTIVITY *actptr, *prevptr;

      actptr = rptr->head;        /* Take the activity out of the resource's queue    */
      while (actptr != (ACTIVITY*)aptr && actptr != NULL) { /* Find it first          */
         prevptr = actptr;
         actptr = actptr->rscnext;
      }
      if (actptr == (ACTIVITY*)aptr) {             /* We found it                     */
         TRACE_RESOURCE_takeout;                   /* Taking element from queue ...   */
         if (actptr == (rptr->head))               /* It was at the head              */
            rptr->head = actptr->rscnext;          /* Next element is new head        */
         else prevptr->rscnext = actptr->rscnext;  /* Take it out & adjust next ptrs  */
         if (actptr == (rptr->tail))               /* It was at the tail              */
            rptr->tail = prevptr;                  /* Adjust the tail pointer         */
         actptr->rscnext = NULL;                   /* Clear its next pointer          */
         rptr->size--;                             /* and decrement the queue size    */
         TRACE_QUEUE_show(rptr);
       }
   }

   TRACE_RESOURCE_use3p;             /* Activity completes service from res/procr ... */

   if (rptr->qdisc != PROCSHAR) {    /* The activity is finished with the resource    */
      rptr->freeservers++;           /* PROCSHAR resources don't have servers         */
      if (rptr->qdisc == RDYLIST) {
         YS__BusyPrcrs--;         /* One more idle processor                          */
         if (YS__BusyPrcrStat)    /* Processor utilization statistics collected       */
            StatrecUpdate (YS__BusyPrcrStat, (double) YS__BusyPrcrs, YS__Simtime);
      }
   }

   if ( rptr->size > 0 && rptr->status != PENDING)  {
      /* The activity was returned to the resource's queue by this operations because */
      /* it had finished a slice and needed additional service from the resource      */
      rptr->status = PENDING;
      YS__QueuePutTail(YS__PendRes,rptr);
   }

  if ( rptr->size == 0 && rptr->status == PENDING)  {
      /* There is nothing in the queue so it should not be PENDING                    */
      YS__QueueDelete(YS__PendRes,rptr);
      rptr->status = QUIESCENT;
   }

   if (rptr->utilstat)                  /* Utilization stats collected                */
      if (rptr->qdisc == PROCSHAR)      /* All activities in the queue for PROCSHAR   */
         StatrecUpdate(rptr->utilstat,(double)(rptr->size),YS__Simtime);
      else                              /* Some at servers for other queues           */
         StatrecUpdate(rptr->utilstat,
                       (double)(rptr->servers-rptr->freeservers),
                       YS__Simtime);

   if (rptr->lengthstat)                /* Queue length stats colelcted               */
      if (rptr->qdisc == PROCSHAR)
         StatrecUpdate(rptr->lengthstat,(double)(rptr->size),YS__Simtime);
      else
         StatrecUpdate(rptr->lengthstat,
                 (double)(rptr->size+rptr->servers-rptr->freeservers),
                 YS__Simtime);

   if (rptr->timestat)                  /* Time in queue stats collected              */
      StatrecUpdate(rptr->timestat,YS__Simtime - aptr->enterque,1.0);

   rptr->serving = NULL;
   aptr->rscptr = NULL;
   return 1;
}

/**************************************************************************************/

void YS__RscAdd(aptr,timeinc)    /* Adds a process to a processor's ready list        */
ACTIVITY *aptr;                  /* Pointer to the process to add                     */
double timeinc;                  /* Amount of service requested                       */
{
   /* The usual way for an activity to use a resource is for it to ask for service by */
   /* calling ResourceUse() or by having it scheduled.  Either of these requires a    */
   /* pointer to the resource.  When a process attached to a processor delays, it     */
   /* must get time from a resource (the processors ready list) instead of being      */
   /* scheduled directly on the event list.  This operation provides a means of doing */
   /* this.  It is called from with in ProcessDelay() it the process is attached to a */
   /* processor.  It is also used to put an activity on the processor's ready list    */
   /* when it wakes up after a suspension other than a delay (e.g., waitin on a sema. */

   RESOURCE *rptr;

   aptr->status = WAIT_RES;               /* For statistics collection                */
   if (aptr->statptr)                     /* Activity statistics collected            */
      StatrecUpdate(aptr->statptr,(double)WAIT_RES,YS__Simtime);
   if (aptr->pp == NULL)                  /* The activity is not attached to a procr  */
      YS__errmsg("Only attached processors can be added to a processor's ready list");
   rptr = aptr->pp->RdyList;              /* Get a pointer to the processor's rdylit  */
   TRACE_RESOURCE_use2;                   /* Activity added to processor's ready list */
   aptr->time = timeinc;                  /* Service time requested                   */
   aptr->timeleft = timeinc;
//   YS__QueuePrinsert(rptr,aptr);          /* Put it in the queue ordered by time FIFO */
   aptr->enterque = YS__Simtime;          /* For time in queue statistics             */
   if ( rptr->status != PENDING)  {       /* Activities requesting service are put    */
      rptr->status = PENDING;             /* in the queue at this point. Then the     */
      YS__QueuePutTail(YS__PendRes,rptr); /* queue is evaluated just before time      */
   }                                      /* advances to see who gets service next    */
   if (rptr->lengthstat)                  /* Queue length stats collected             */
      StatrecUpdate(rptr->lengthstat,
         (double)(rptr->size + rptr->servers - rptr->freeservers),
         YS__Simtime);
   aptr->rscptr = rptr;                   /* Tells activty which resource it is in    */
}

