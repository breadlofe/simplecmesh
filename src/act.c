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
#include "../incl/tr.act.h"

/**************************************************************************************/

static char*  procstates[16] = { "LIMBO","READY","DELAYED","WAIT_SEMAPHORE",
                                 "WAIT_FLAG","WAIT_CONDITION",
                                 "WAIT_RESOURCE","USING_RESOURCE",
                                 "RUNNING","BLOCKED","WAIT_JOIN",
                                 "WAIT_MESSAGE","WAIT_BARRIER" };

/**************************************************************************************\
******************************* ACTIVITY Operations ************************************
****************************************************************************************
**                                                                                    **
**      There are two types of activities, processes and events.  The activity        **
**      operations are those that are work the same on both types.  The               **
**      descriptors for both events and processes are obtained from the               **
**      descriptors for activities by simply adding fields.                           **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

int YS__ActId(aptr)            /* Returns the system defined ID or 0 if TraceIDs is 0 */
ACTIVITY *aptr;                /* Pointer to the activity, or NULL                    */
{
   if (TraceIDs) {             /* TraceIDs must be set to get IDs printed in traces   */
      if (aptr) return aptr->id;                     /* aptr points to something      */
      else if (YS__ActEvnt) return YS__ActEvnt->id;  /* Called from an active event   */
      else YS__errmsg("Null Activity Referenced");
   }
   else return 0;
}

/**************************************************************************************/

void ActivitySetArg(aptr,arptr,arsize) /* Sets the argument pointer of an activity    */
ACTIVITY *aptr;                        /* Pointer to the activity                     */
char  *arptr;                          /* Pointer to the new argument                 */
int arsize;                            /* Argument size                               */
{
   PSDELAY;

   if (aptr) {                                  /* aptr points to an activity         */
      TRACE_ACTIVITY_setarg;                    /* Setting argument for activity ...  */
      aptr->argptr = arptr;                     /* Set its argument                   */
      aptr->argsize = arsize;
   }
   else if (YS__ActEvnt) {                      /* Called from an active event        */
      YS__ActEvnt->argptr = arptr;              /* It is setting its own argument     */
      YS__ActEvnt->argsize = arsize;
   }
   else YS__errmsg("Null Activity Referenced");
}

/**************************************************************************************/

char *ActivityGetArg(aptr)           /* Returns the argument pointer of an activity   */
ACTIVITY *aptr;                      /* Pointer to the activity                       */
{
   PSDELAY;

   if (aptr) {                                /* aptr points to an activity           */
      TRACE_ACTIVITY_getarg;                  /* Getting argument from activity ...   */
      return aptr->argptr;
   }
   else if (YS__ActEvnt) return YS__ActEvnt->argptr;    /* Active event or process    */
   else YS__errmsg("Null Activity Referenced");
}

/**************************************************************************************/

int ActivityArgSize(aptr)            /* Returns the size of an argument               */
ACTIVITY *aptr;                      /* Pointer to the activity                       */
{
   PSDELAY;

   if (aptr) return aptr->argsize;                     /* aptr points to an activity  */
   else if (YS__ActEvnt) return YS__ActEvnt->argsize;  /* Active event or process     */
   else YS__errmsg("Null Activity Referenced");        /* own argument                */
}

/**************************************************************************************/

void ActivitySchedTime(actptr,timeinc,bflg) /* Schedules an activity in the future    */
ACTIVITY *actptr;                           /* Pointer to the activity                */
double timeinc;                             /* Time incremnt                          */
int bflg;                                   /* Block flag - INDEPENDENT,BLOCK,FORK    */
{
   ACTIVITY *aptr;

   PSDELAY;

   if (actptr) aptr = actptr;                 /* actptr points to an activity, use it */
   else if (YS__ActEvnt) {                    /* or an event is rescheduling itself   */
      if (YS__ActEvnt->deleteflag == DELETE)
         YS__errmsg("Can not reschedule a deleting event");
      aptr = (ACTIVITY*)YS__ActEvnt;
   }
   else YS__errmsg("Null Activity Referenced"); /* or there is a mistake              */

   TRACE_ACTIVITY_schedule1;  /* Scheduling activity to occur in "timeinc" time units */
   aptr->blkflg = bflg;

   if (aptr->type==PROCTYPE && aptr->status!=LIMBO)
      YS__errmsg("Processes can only be scheduled once");
   else if (aptr->type==EVTYPE && aptr->status!=RUNNING && aptr->status!=LIMBO)
      YS__errmsg("Can not reschedule a pending event");
    else if ((aptr->type==OSPRTYPE || aptr->type==USRPRTYPE) && aptr->status!=LIMBO)
      YS__errmsg("Processes can only be scheduled once");
   else if (aptr->type==OSEVTYPE && aptr->status!=RUNNING && aptr->status!=LIMBO)
      YS__errmsg("Can not reschedule a pending event");

  if (aptr->blkflg!=INDEPENDENT)
      YS__errmsg("Only Independent block-flag supported for ActivitySchedTime in this version");

   if (timeinc == 0.0)  {                                     /* Schedule immediately */
      aptr->time = YS__Simtime;
      YS__RdyListAppend(aptr);
   }
   else if (timeinc > 0.0)  {                               /* Schedule in the future */
      aptr->time = YS__Simtime+timeinc;
      aptr->status = DELAYED;
      if (aptr->statptr)
         StatrecUpdate(aptr->statptr,(double)DELAYED,YS__Simtime);
      YS__EventListInsert(aptr);
   }
   else YS__errmsg("Can not schedule an activity to occur in the past");

}

/**************************************************************************************/

void ActivitySchedSema(actptr,sptr,bflg)    /* Schedules an activity to wait until a  */
                                            /* semaphore becomes positive.            */
ACTIVITY *actptr;                           /* Pointer to the activity                */
SEMAPHORE *sptr;                            /* Pointer to the semaphore               */
int bflg;                                   /* Block flag - INDEPENDENT,BLOCK,FORK    */
{
   ACTIVITY *aptr;

   PSDELAY;

   if (actptr) aptr = actptr;                 /* actptr points to an activity, use it */
   else if (YS__ActEvnt) {                    /* or an event is rescheduling itself   */
      if (YS__ActEvnt->deleteflag == DELETE)
         YS__errmsg("Can not reschedule a deleting event");
      aptr = (ACTIVITY*)YS__ActEvnt;
   }
   else YS__errmsg("Null Activity Referenced"); /* or there is a mistake              */

   TRACE_ACTIVITY_schedule2;         /* Scheduling activity to wait for semaphore ... */
   aptr->blkflg = bflg;

   if (aptr->type==PROCTYPE && aptr->status!=LIMBO)
      YS__errmsg("Processes can only be scheduled once");
   else if (aptr->type==EVTYPE && aptr->status!=RUNNING && aptr->status!=LIMBO)
      YS__errmsg("Can not reschedule a pending event");
   else if ((aptr->type==OSPRTYPE || aptr->type==USRPRTYPE) && aptr->status!=LIMBO)
      YS__errmsg("Processes can only be scheduled once");
   else if (aptr->type==OSEVTYPE && aptr->status!=RUNNING && aptr->status!=LIMBO)
      YS__errmsg("Can not reschedule a pending event");

   if (aptr->blkflg!=INDEPENDENT)
      YS__errmsg("Only processes can perform forking or blocking schedules");

   if (sptr->val > 0)  {
      TRACE_ACTIVITY_schedule3;       /* Semaphore decremented and activity activated */
      sptr->val--;
      YS__RdyListAppend(aptr);
   }
   else  {
      TRACE_ACTIVITY_schedule4;           /* Semaphore <= 0, activity waits           */
      aptr->status = WAIT_SEM;
      if (aptr->statptr)
         StatrecUpdate(aptr->statptr,(double)WAIT_SEM,YS__Simtime);
      YS__QueuePutTail(sptr,aptr);
   }

}

/**************************************************************************************/

void ActivitySchedFlag(actptr,fptr,bflg) /* Schedules an activity to wait for a flag  */
                                         /* to be set.                                */
ACTIVITY *actptr;                        /* Pointer to the activity                   */
FLAG *fptr;                              /* Pointer to the flag                       */
int bflg;                                /* Block flag - INDEPENDENT,BLOCK,FORK       */
{
   ACTIVITY *aptr;

   PSDELAY;

   if (actptr) aptr = actptr;                 /* actptr points to an activity, use it */
   else if (YS__ActEvnt) {                    /* or an event is rescheduling itself   */
      if (YS__ActEvnt->deleteflag == DELETE)
         YS__errmsg("Can not reschedule a deleting event");
      aptr = (ACTIVITY*)YS__ActEvnt;
   }
   else YS__errmsg("Null Activity Referenced"); /* there is a mistake                 */

   TRACE_ACTIVITY_schedule8;             /* Scheduling activity to wait for flag ...  */
   aptr->blkflg = bflg;

   if (aptr->type==PROCTYPE && aptr->status!=LIMBO)
      YS__errmsg("Processes can only be scheduled once");
   else if (aptr->type==EVTYPE && aptr->status!=RUNNING && aptr->status!=LIMBO)
      YS__errmsg("Can not reschedule a pending event");
   else if ((aptr->type==OSPRTYPE || aptr->type==USRPRTYPE) && aptr->status!=LIMBO)
      YS__errmsg("Processes can only be scheduled once");
   else if (aptr->type==OSEVTYPE && aptr->status!=RUNNING && aptr->status!=LIMBO)
      YS__errmsg("Can not reschedule a pending event");

   if (aptr->blkflg!=INDEPENDENT)
      YS__errmsg("Only processes can perform forking or blocking schedules");

   if (fptr->state == CLEARED)  {
      TRACE_ACTIVITY_schedule13;  /* Flag not set, activity scheduled to wait on flag */
      aptr->status = WAIT_FLG;                /* The processes status changes         */
      if (aptr->statptr)                      /* Collect process statistics           */
         StatrecUpdate(aptr->statptr,(double)WAIT_FLG,YS__Simtime);
      YS__QueuePutTail(fptr,aptr);            /* Enter the flag's queue and           */
   }
   else  {
      TRACE_ACTIVITY_schedule14;   /* Flag set, activity scheduled to occur now       */
      fptr->state = CLEARED;       /* Flag is reset                                   */
      YS__RdyListAppend(aptr);
   }

}

/**************************************************************************************/

void ActivitySchedCond(actptr,cptr,bflg) /* Schedules an activity to wait until a     */
                                         /* condition is true.                        */
ACTIVITY *actptr;                        /* Pointer to the activity                   */
CONDITION *cptr;                         /* Pointer to the condition                  */
int bflg;                                /* Block flag - INDEPENDENT,BLOCK,FORK       */
{
   ACTIVITY *aptr;

   PSDELAY;

   if (actptr) aptr = actptr;                  /* actptr pints to an activity, use it */
   else if (YS__ActEvnt) {                     /* or an event is rescheduling itself  */
      if (YS__ActEvnt->deleteflag == DELETE)
         YS__errmsg("Can not reschedule a deleting event");
      aptr = (ACTIVITY*)YS__ActEvnt;
   }
   else YS__errmsg("Null Activity Referenced");  /* or there is a mistake             */

   TRACE_ACTIVITY_schedule5;        /* Scheduling activity to wait for condition ...  */
   aptr->blkflg = bflg;

   if (aptr->type==PROCTYPE && aptr->status!=LIMBO)
      YS__errmsg("Processes can only be scheduled once");
   else if (aptr->type==EVTYPE && aptr->status!=RUNNING && aptr->status!=LIMBO)
      YS__errmsg("Can not reschedule a pending event");
   else if ((aptr->type==OSPRTYPE || aptr->type==USRPRTYPE) && aptr->status!=LIMBO)
      YS__errmsg("Processes can only be scheduled once");
   else if (aptr->type==OSEVTYPE && aptr->status!=RUNNING && aptr->status!=LIMBO)
      YS__errmsg("Can not reschedule a pending event");

   if (aptr->blkflg!=INDEPENDENT )
      YS__errmsg("Only processes can perform forking or blocking schedules");

   if (cptr->state == FALSE) {
      TRACE_ACTIVITY_schedule11;          /* Condition false ...                      */
      aptr->status = WAIT_CON;            /* Process status changes                   */
      if (aptr->statptr)                  /* Collect process statistics               */
         StatrecUpdate(aptr->statptr,(double)WAIT_CON,YS__Simtime);
      YS__QueuePutTail(cptr,aptr);        /* Enter condition's queue                  */
   }
   else {
      TRACE_ACTIVITY_schedule15;          /* Condition true ...                       */
      YS__RdyListAppend(aptr);
   }
}

/**************************************************************************************/

void ActivitySchedRes(actptr,rptr,timeinc,bflg)
                                         /* Schedules an activity to use service from */
                                         /* a resource.                               */
ACTIVITY *actptr;                        /* Pointer to the activity                   */
RESOURCE *rptr;                          /* Pointer to the resource                   */
double timeinc;                          /* Amount of requested service               */
int bflg;                                /* Block flag - INDEPENDENT,BLOCK,FORK       */
{
   ACTIVITY *aptr;

   if (actptr) aptr = actptr;                 /* actptr points to an activity, use it */
   else if (YS__ActEvnt) {                    /* or an event is rescheduling itself   */
      if (YS__ActEvnt->deleteflag == DELETE)
         YS__errmsg("Can not reschedule a deleting event");
      aptr = (ACTIVITY*)YS__ActEvnt;
   }
   else YS__errmsg("Null Activity Referenced");  /* or there is a mistake             */

   TRACE_ACTIVITY_schedule6;              /* Scheduling activity to use resource ...  */
   aptr->blkflg = bflg;

   if (aptr->type==PROCTYPE && aptr->status!=LIMBO)
      YS__errmsg("Processes can only be scheduled once");
   else if (aptr->type==EVTYPE && aptr->status!=RUNNING && aptr->status!=LIMBO)
      YS__errmsg("Can not reschedule a pending event");
   else if ((aptr->type==OSPRTYPE || aptr->type==USRPRTYPE) && aptr->status!=LIMBO)
      YS__errmsg("Processes can only be scheduled once");
   else if (aptr->type==OSEVTYPE && aptr->status!=RUNNING && aptr->status!=LIMBO)
      YS__errmsg("Can not reschedule a pending event");

   if (aptr->blkflg!=INDEPENDENT)
      YS__errmsg("Only processes can perform forking or blocking schedules");

   if (timeinc >=0.0) aptr->time = timeinc;
   else YS__errmsg("Can not request negative service time from a resource");
   TRACE_ACTIVITY_schedule12;            /* - resource notified or already pending    */
   YS__RscInsert(rptr,aptr);             /* Add the activity to the resource's queue  */
   aptr->rscptr = rptr;
   if (aptr->statptr)
      StatrecUpdate(aptr->statptr,(double)WAIT_RES,YS__Simtime);
}

/**************************************************************************************/

void ActivityCollectStats(aptr) /* Activates automatic statistics collection          */
ACTIVITY *aptr;                 /* Pointer to the activity                            */
{
   ACTIVITY *actptr;

   PSDELAY;

   if (aptr) actptr = aptr;
   else if (YS__ActEvnt) actptr = (ACTIVITY*)YS__ActEvnt;
   else YS__errmsg("NULL Activity referenced");

   if (actptr->statptr == NULL) {
      actptr->statptr = NewStatrec(actptr->name,INTERVAL,NOMEANS,HIST,12,1.0,13.0);
      StatrecUpdate(actptr->statptr,(double)(actptr->status),YS__Simtime);
   }                                                      /* Start the first interval */
   else YS__warnmsg("Process statistics collection already set");
}

/**************************************************************************************/

void ActivityStatRept(actptr)  /* Prints a report of an activity's statistics         */
ACTIVITY *actptr;              /* Pointer to the activity                             */
{
   ACTIVITY *aptr;
   STATREC *srptr;
   double total = 0.0;
   int i;

   PSDELAY;

   if (actptr) aptr = actptr;                 /* actptr points to an activity, use it */
   else if (YS__ActEvnt) aptr = (ACTIVITY*)YS__ActEvnt; /* or use the Active Event    */
   else YS__errmsg("Null Activity Referenced"); /* there is a mistake                 */

   srptr = aptr->statptr;
   if (srptr) {                              /* Statistics collection activated       */
      TracePrintTag("statrpt","\nStatistics for activity %s ",aptr->name);
      total = srptr->time1 - srptr->time0;   /* Total = sampling interval             */
      if (total > 0.0) {
         TracePrintTag("statrpt","from time %g to time %g:\n",srptr->time0,srptr->time1);
         for (i=0; i<srptr->bins; i++) {
            if (srptr->hist[i] > 0.0) {
               TracePrintTag("statrpt","   %6.3f time units, %5.2f%s of the sampling interval, ",
                       srptr->hist[i],(srptr->hist[i]/total)*100,"%");
               TracePrintTag("statrpt","were spent in state %-10s\n",procstates[i]);
            }
         }
      }
      else TracePrintTag("statrpt","\n   Zero interval; no statistics available");
      TracePrintTag("statrpt","\n");
   }
   else YS__warnmsg("Statistics not collected; cannot print report");
}

/**************************************************************************************/

STATREC *ActivityStatPtr(aptr) /* Returns a pointer to an activity's  statrec         */
ACTIVITY *aptr;                /* Pointer to the activity                             */
{
   PSDELAY;

   return aptr->statptr;
}

/**************************************************************************************/

ACTIVITY *ActivityGetParPtr()  /* Returns a pointer to the current activity's parent  */
{                              /* Returns NULL if it has no parent                    */

}

/**************************************************************************************/

ACTIVITY *ActivityGetMyPtr()   /* Returns a pointer to the current activity           */
{
   PSDELAY;

   if (YS__ActEvnt) return (ACTIVITY*)(YS__ActEvnt);
   else YS__errmsg("ActivityGetParPtr() must be called from within an activity");
}


/**************************************************************************************\
********************************* EVENT Operations *************************************
****************************************************************************************
**                                                                                    **
**        Events do not have a stack.  As a result, they are like subroutines         **
**        must terminate and start again at the beginning instead of suspending.      **
**        The rescheduling operations are an attempt to give events some of the       **
**        suspending properties of processes, but they are limited in what they do.   **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

EVENT *NewEvent(ename,bodyname,dflag,etype) /* Creates & returns pointer to an event  */
char *ename;                                /* User defined name                      */
func bodyname;                              /* Defining function of the event         */
int dflag;                                  /* DELETE or NODELETE                     */
int etype;                                  /* User defined type                      */
{
   EVENT *eptr;

   PSDELAY;

   eptr = (EVENT*)YS__PoolGetObj(&YS__EventPool);      /* Get the activity descriptor */
   eptr->id = YS__idctr++;                             /* System assigned unique ID   */
   strncpy(eptr->name,ename,31);                       /* Copy the name               */
   eptr->name[31] = '\0';                              /*   Limited to 31 characters  */
   eptr->type = EVTYPE;                                /* Initialize all fields       */
   eptr->next = NULL;
   eptr->argptr = NULL;
   eptr->argsize = 0;
   eptr->time = 0.0;
   eptr->parentid = -1;
   eptr->status = LIMBO;
   eptr->blkflg = INDEPENDENT;
   eptr->statptr = NULL;

   eptr->priority = 0.0;
   eptr->timeleft = 0.0;
   eptr->enterque = 0.0;
   eptr->rscptr = NULL;
   eptr->pp = NULL;

   eptr->body = bodyname;
   eptr->deleteflag = dflag;
   eptr->evtype = etype;
   eptr->state = 0;
   TRACE_EVENT_event;                                   /* Creating event ...         */
   return eptr;
}

/**************************************************************************************/

void EventReschedTime(timeinc,st)  /* Reschedules an event to occur in the future     */
double timeinc;                    /* Time increment                                  */
int st;                            /* Return state                                    */
{
   PSDELAY;

   if (YS__ActEvnt == NULL)
      YS__errmsg("EventReschedule() can only be invoked from within an event body");
   if (YS__ActEvnt->deleteflag == DELETE)
      YS__errmsg("Can not reschedule a deleting event");

   if (timeinc < 0.0) YS__errmsg("Events can not be scheduled in the past");

   TRACE_EVENT_reschedule1; /* Rescheduling activity to occur in "timeinc" time units */
   YS__ActEvnt->state = st;

   if (YS__ActEvnt->pp == NULL)
      if (timeinc == 0.0)  {                                  /* Schedule immediately */
         YS__ActEvnt->time = YS__Simtime;
         YS__RdyListAppend(YS__ActEvnt);
      }
      else {                                                /* Schedule in the future */
         YS__ActEvnt->time = YS__Simtime+timeinc;
         YS__ActEvnt->status = DELAYED;
         if (YS__ActEvnt->statptr)
            StatrecUpdate(YS__ActEvnt->statptr,(double)DELAYED,YS__Simtime);
         YS__EventListInsert(YS__ActEvnt);
      }
   else
      YS__RscAdd(YS__ActEvnt,timeinc);  /* Add to the processor's ready list          */
}

/**************************************************************************************/

void EventReschedSema(sptr,st)   /* Reschedules an event to wait for a semaphore      */
SEMAPHORE *sptr;                 /* Pointer to the semaphore                          */
int st;                          /* Return state                                      */
{
   PSDELAY;

   if (YS__ActEvnt == NULL)
      YS__errmsg("EventReschedule() can only be invoked from within an event body");
   if (YS__ActEvnt->deleteflag == DELETE)
      YS__errmsg("Can not reschedule a deleting event");

   TRACE_EVENT_reschedule2;          /* Rescheduling event to wait for semaphore      */
   YS__ActEvnt->state = st;
   if (sptr->val > 0)  {
      TRACE_EVENT_reschedule3;       /* Semaphore decremented and activity activated  */
      sptr->val--;
      YS__RdyListAppend(YS__ActEvnt);
   }
   else  {
      TRACE_EVENT_reschedule4;       /* Semaphore <= 0, activity waits                */
      YS__ActEvnt->status = WAIT_SEM;
      if (YS__ActEvnt->statptr)
         StatrecUpdate(YS__ActEvnt->statptr,(double)WAIT_SEM,YS__Simtime);
      YS__QueuePutTail(sptr,YS__ActEvnt);
   }
}

/**************************************************************************************/

void EventReschedFlag(fptr,st)    /* Reschedules an event to wait for a flag          */
FLAG *fptr;                       /* Pointer to the flag                              */
int st;                           /* Return state                                     */
{
   EVENT *aptr;

   PSDELAY;

   if (YS__ActEvnt == NULL)
      YS__errmsg("EventReschedule() can only be invoked from within an event body");
   if (YS__ActEvnt->deleteflag == DELETE)
      YS__errmsg("Can not reschedule a deleting event");

   aptr = YS__ActEvnt;
   TRACE_EVENT_reschedule5;               /* Rescheduling event to wait for flag      */
   aptr->state = st;

   if (fptr->state == CLEARED)  {
      TRACE_ACTIVITY_schedule13;  /* Flag not set, activity scheduled to wait on flag */
      aptr->status = WAIT_FLG;                /* The processes status changes         */
      if (aptr->statptr)                      /* Collect process statistics           */
         StatrecUpdate(aptr->statptr,(double)WAIT_FLG,YS__Simtime);
      YS__QueuePutTail(fptr,aptr);            /* Enter the flag's queue and           */
   }
   else  {
      TRACE_ACTIVITY_schedule14;   /* Flag set, activity scheduled to occur now       */
      fptr->state = CLEARED;       /* Flag is reset                                   */
      YS__RdyListAppend(aptr);
   }
}

/**************************************************************************************/

void EventReschedCond(cptr,st)     /* Reschedules an event to wait for a condition    */
CONDITION *cptr;                   /* Pointer to the condition                        */
int st;                            /* Return state                                    */
{
   EVENT *aptr;

   PSDELAY;

   if (YS__ActEvnt == NULL)
      YS__errmsg("EventReschedule() can only be invoked from within an event body");
   if (YS__ActEvnt->deleteflag == DELETE)
      YS__errmsg("Can not reschedule a deleting event");

   aptr = YS__ActEvnt;
   TRACE_EVENT_reschedule7;               /* Scheduling event to wait for condition   */
   aptr->state = st;

   if (cptr->state == FALSE) {
      TRACE_ACTIVITY_schedule11;          /* Condition false ...                      */
      aptr->status = WAIT_CON;            /* Process status changes                   */
      if (aptr->statptr)                  /* Collect process statistics               */
         StatrecUpdate(aptr->statptr,(double)WAIT_CON,YS__Simtime);
      YS__QueuePutTail(cptr,aptr);        /* Enter condition's queue                  */
   }
   else {
      TRACE_ACTIVITY_schedule15;          /* Condition true ...                       */
      YS__RdyListAppend(aptr);
   }
}

/**************************************************************************************/

void EventReschedRes(rptr,timeinc,st)   /* Reschedules an event to use a resource     */
RESOURCE *rptr;                         /* Pointer to the resource                    */
double timeinc;                         /* Requested service time                     */
int st;                                 /* Return state                               */
{
   if (YS__ActEvnt == NULL)
      YS__errmsg("EventReschedule() can only be invoked from within an event body");
   if (YS__ActEvnt->deleteflag == DELETE)
      YS__errmsg("Can not reschedule a deleting event");
   if (timeinc < 0.0)
      YS__errmsg("Can not request negative service time from a resource");

   TRACE_EVENT_reschedule9;           /* Rescheduling event to use resource ..        */
   YS__ActEvnt->state = st;
   YS__ActEvnt->time = timeinc;
   TRACE_EVENT_reschedule10;          /* - resource notified or already pending  ..   */
   YS__RscInsert(rptr,YS__ActEvnt);   /* Add the activity to the resource's queue     */
   YS__ActEvnt->rscptr = rptr;
   if (YS__ActEvnt->statptr)
      StatrecUpdate(YS__ActEvnt->statptr,(double)WAIT_RES,YS__Simtime);
}

/**************************************************************************************/

int EventGetType(eptr)        /* Returns the event's type                             */
EVENT *eptr;                  /* Pointer to the event, or NULL                        */
{
   PSDELAY;

   if (eptr) return eptr->evtype;                     /* eptr points to an event      */
   else if (YS__ActEvnt) return YS__ActEvnt->evtype;  /* Called from the event        */
   else YS__errmsg("Null Activity Referenced");
}

/**************************************************************************************/

void EventSetType(eptr,etype)      /* Sets the event's type                           */
EVENT *eptr;                       /* Pointer to the event                            */
int etype;                         /* Event's new type                                */
{
   PSDELAY;

   TRACE_EVENT_settype;
   if (eptr) eptr->evtype = etype;                    /* eptr points to an event      */
   else if (YS__ActEvnt) YS__ActEvnt->evtype = etype; /* Called from the event        */
   else YS__errmsg("Null Activity Referenced");
}

/**************************************************************************************/

int EventGetDelFlag(eptr)          /* Returns DELETE (1) or NODELETE (0)              */
EVENT *eptr;                       /* Pointer to an event                             */
{
   PSDELAY;

   if (eptr) return eptr->deleteflag;              /* eptr points to an event  */
   else if (YS__ActEvnt) return YS__ActEvnt->deleteflag;  /* Called from event */
   else YS__errmsg("Null Activity Referenced");
}

/**************************************************************************************/

void EventSetDelFlag()             /* Makes an event deleting                         */
{
   if (YS__ActEvnt == NULL)
      YS__errmsg("EventSetDelFlag() not called from within an event");
   if (YS__ActEvnt->status != RUNNING)
      YS__errmsg("Changing Delete Flag of a scheduled event");
   TRACE_EVENT_setdelflg;
   YS__ActEvnt->deleteflag = DELETE;
}

/**************************************************************************************/

int EventGetState()              /* Returns the state set by EventSetState()          */
{
   PSDELAY;

   if (YS__ActEvnt) return YS__ActEvnt->state;
   else YS__errmsg("EventGetState() must be called from within an active event");
}

/**************************************************************************************/

void EventSetState(eptr,st)      /* Sets state used to designate a return point       */
EVENT *eptr;                     /* Pointer to an event                               */
int st;                          /* New state value                                   */
{
   PSDELAY;

   if (eptr == NULL )            /* Called from within the event                      */
      if (YS__ActEvnt) YS__ActEvnt->state = st;
      else YS__errmsg("EventSetState() has NULL pointer, but not called from an event");
   else eptr->state = st;
}

/**************************************************************************************/

EVENT *NewOSEvent(ename,bodyname,dflag,etype) /* Creates a new OS event               */
char *ename;                                  /* User defined name                    */
func bodyname;                                /* Defining function of the event       */
int dflag;                                    /* DELETE or NODELETE                   */
int etype;                                    /* User defined type                    */
{
   EVENT *eptr;

   eptr = NewEvent(ename,bodyname,dflag,etype);
   eptr->type = OSEVTYPE;
   return eptr;
}

/**************************************************************************************/

