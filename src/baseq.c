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
#include "../incl/tr.baseq.h"
#include <stdarg.h>
#include "malloc.h"

// Operation of the POOL structures should be the same as when POOL_AS_MALLOC is defined
// Brian Neel AMD 2013

//#define POOL_AS_MALLOC	1

/*****************************************************************************/
/* POOL Operations: used to manage the allocation of memory used for         */
/* object descriptors.  These reduce the number of calls to malloc by        */
/* maintaining a list of descriptors that can be allocated for new           */
/* objects and then returned to the pool for reuse when the object is        */
/* deleted or the simulation reset.  Pools use malloc to obtain large        */
/* blocks of memory consisting of several objects and then parcels           */
/* them out in response to the "new" object operation.  Pools never          */
/* return memory to the system, so the size of the pool only increases.      */
/* In order to use a pool, the first two elements of each structure must be  */
/* char pointers "pnxt" and "pfnxt", which maintain the pool lists           */
/*****************************************************************************/

/*****************************************************************************/
/* YS__PoolInit: start out a new pool of objects, specifying the number to   */
/* allocate with each increment and the size of each object                  */
/*****************************************************************************/

void YS__PoolInit(pptr,name,objs,objsz)  /* Initialize a pool                */
POOL *pptr;                              /* Pointer to the pool              */
char *name;                              /* User defined name for the pool   */
int  objs;                               /* Number of objects to malloc      */
int  objsz;                              /* Size of each object in bytes     */
{
   pptr->p_head = NULL;                  /* Points to allocated objects      */
   pptr->p_tail = NULL;                  /* Points to the tail of the pool   */
   pptr->pf_head = NULL;                 /* Points to unallocated objects    */
   pptr->pf_tail = NULL;                 /* Points to unallocated objects    */
   pptr->objects = objs;                 /* Number of objects to allocate with each increment */
   pptr->objsize = objsz;                /* Size of each object              */
   pptr->newed = pptr->killed = 0;       /* Clear out these allocation stats */
   strncpy(pptr->name,name,31);          /* copy the name in                 */
   pptr->name[31] = '\0';
}

/*****************************************************************************/
/* YS__PoolStats: print out allocation stats, for debugging                  */
/*****************************************************************************/
void YS__PoolStats(POOL *pptr)
{
#ifdef DEBUG_POOL
  fprintf(simout,"Pool %s stats: newed %d killed %d\n",pptr->name,pptr->newed,pptr->killed);
#endif
}

/*****************************************************************************/
/* YS__PoolGetObj: Return a pointer to an object from the pool. If there are */
/* no free objects at the time, allocate a new chunk of objects and set      */
/* the pool fields for them. Zero them out also. Initialize some fields for  */
/* REQ data structures additionally.                                         */
/*****************************************************************************/
char *YS__PoolGetObj(pptr)       /* Get an object from the pool              */
POOL *pptr;                      /* Pointer to the pool                      */
{
   char *ptr;

   if(pptr == 0x100427bc8)
	   printf(""); //now just a breakpoint for only the event queue

   pptr->newed++;
#ifdef POOL_AS_MALLOC
   ptr = (char *)malloc(pptr->objsize);
   if (ptr == NULL)
     YS__errmsg("Malloc fails in PoolGetObj");
   memset(ptr,0,pptr->objsize);
   /* no need to assign pnext and pfnext */
#else /* Regular pool operation */

   TRACE_POOL_getobj1;           /* Getting object from pool                 */
   if (pptr->pf_head == NULL) 
   {  /* No unallocated objects in the pool       */
     int i;
     TRACE_POOL_getobj2;        /* Pool gets new block from system          */
     ptr = (char*)malloc((pptr->objects)*(pptr->objsize)); /* Get a block of objects */

//	 printf("In Pool get object/allocation: Name: %s, pool_objects: %d, object_size: %d, Poolstart: %x\n", pptr->name, pptr->objects, pptr->objsize, ptr); 

     if (ptr == NULL)
       YS__errmsg("Malloc fails in PoolGetObj");

     memset(ptr,0,(pptr->objects)*(pptr->objsize));

     for(i = 0; i<(pptr->objects)-1; i++) /* Link together the new objects*/
	 { 
       *((char**)(ptr+i*(pptr->objsize))) = ptr+(i+1)*(pptr->objsize); /* Setting up pfnxt */
       //*((char**)(ptr+i*(pptr->objsize) + sizeof(char *))) = ptr+(i+1)*(pptr->objsize); /* Setting up pnxt */
     }

     /*if (pptr->p_tail == NULL) // The pool is empty, this is first call to GetObj  
	 { 
       pptr->p_head = ptr;
     }
     else // Add the new objects at the tail of the pool 
	 {                 
       *((char**)(pptr->p_tail)) = ptr;
     }
     pptr->p_tail = ptr + (pptr->objects - 1)*(pptr->objsize); // Adjust tail pointer  
     *((char**)(pptr->p_tail)) = NULL;*/         // Last object has no next object  

	//Now set the pf_head and pf_tail 
     pptr->pf_head = ptr;
     pptr->pf_tail = ptr + (pptr->objects - 1)*(pptr->objsize); // Adjust tail pointer  
     *((char**)(pptr->pf_tail)) = NULL;         /* Last object has no next object */
   }

   ptr = pptr->pf_head;                        // Get the next free object             

   if(pptr->pf_head == pptr->pf_tail)	// If last element is being removed null both head and tail, else move head to next free object in the pool
   {
	   pptr->pf_head = NULL;
	   pptr->pf_tail = NULL;
   }
   else
   {
	   pptr->pf_head = *((char**)(pptr->pf_head));  /* Shift the middle to the right */
   }
#endif

   return ptr;
}

/*****************************************************************************/
/* YS__PoolReturnObj: Put an object back into its pool                       */
/*****************************************************************************/

void YS__PoolReturnObj(POOL *pptr,void *optr)       /* Return an object to its pool             */
{
   pptr->killed++;

#ifdef POOL_AS_MALLOC
   free(optr);
#else
   TRACE_POOL_retobj;                    /* Returning object to pool                  */

   if (pptr->pf_tail) {
     *((char **) (pptr->pf_tail)) = optr;
     //*((char **) ((char *)optr)) = NULL;
     pptr->pf_tail = optr;
   }
   else {
     pptr->pf_head = optr;
     pptr->pf_tail = optr;
     //*((char **) ((char *)optr)) = NULL;
   }

//   if(pptr->killed%1000 == 0)
//	   printf("newed: %d, killed: %d\n", pptr->newed, pptr->killed);

   memset(optr,0,pptr->objsize);
#endif

}

/*****************************************************************************/
/* YS__PoolReset: Deallocate and clear all objects in the pool.              */
/*****************************************************************************/

void YS__PoolReset(pptr)     /* Deallocates & clears all objects in the pool */
POOL *pptr;                  /* Pointer to the pool                          */
{
   char *ptr;
   int i;

#ifdef DEBUG_POOL
   fprintf(simout,"Pool %s at reset: newed %d killed %d\n",pptr->name,pptr->newed,pptr->killed);
#endif
   pptr->newed=pptr->killed=0;

   for (ptr = pptr->p_head; ptr != NULL; ptr = *((char **)(ptr + sizeof(char *)))) {
     if (*((char **)ptr) == NULL) /* Not part of free pool items queue */
       {
	 for (i= sizeof(char *)*2; i< pptr->objsize; i++) *(ptr+i) = '\0';
       }
     *((char **)(ptr+sizeof(char *))) = ptr + pptr->objsize; /* Putting all objects in one free pool */
   }
   pptr->pf_head = pptr->p_head;
   pptr->pf_tail = pptr->p_tail;
 }



/**************************************************************************************\
********************************* QUEUE Operations *************************************
****************************************************************************************
**                                                                                    **
**        Queues implement the basic linked lists used by several other               **
**        objects (e.g., semaphores, flags, resources).  They are simple              **
**        single-link lists with a pointer to the head and tail and links             **
**        that point in the direction from head to tail.  The generic element         **
**        for these queues is a QE, a structure that forms the base of several        **
**        different types of objects that can to put in queues (e.g., processes,      **
**        events, queues themselves).  There are several basic operations to          **
**        add elements to and delete them from queues.  Four special insertion        **
**        operations insert processs or events ordered by one of two keys             **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

void QueueCollectStats(qptr,type,meanflg,histflg,nbin,low,high)
                     /* Activates automatic statistics collection for the queue       */
SYNCQUE *qptr;       /* Pointer to the queue                                          */
int type;            /* LENGTH, TIME, UTIL                                            */
int meanflg;         /* MEANS or NOMEANS                                              */
int histflg;         /* HIST, NOHIST                                                  */
int nbin;            /* Number of bins                                                */
double low;          /* Max value for low bin                                         */
double high;         /* Min value for high bin                                        */
{
   char namestr[32];                          /* Used to build statrec name           */
   RESOURCE *rptr = (RESOURCE*)qptr;          /* Only cast qptr once                  */

   strncpy(namestr,qptr->name,24);            /* First part of statrec name           */
   namestr[31] = '\0';                        /* Make sure the string is terminated   */

   if (qptr->type != RESTYPE) {               /* Object must be a SYNCQUE             */
      if (type == LENGTH) {                   /* Collect queue length stats           */
         if (qptr->lengthstat == NULL)  {     /* Stat collection not yet initiated    */
            strcat(namestr,".length");        /* Tack ".length" onto the queue name   */
            qptr->lengthstat =                /* Get a new statrec & and update it    */
               NewStatrec(namestr,INTERVAL,meanflg,histflg,nbin,low,high);
            StatrecUpdate(qptr->lengthstat, (double)(qptr->size), YS__Simtime);
         }
         else YS__warnmsg("Queue length statistics collection already set");
      }
      else if (type == TIME) {                /* Collect time in queue stats          */
         if (qptr->timestat == NULL) {        /* Stat collectin not yet initiated     */
            strcat(namestr,".time");          /* Tack ".time" onto the queue name     */
            qptr->timestat =                  /* Get a new statrec                    */
               NewStatrec(namestr,POINT,meanflg,histflg,nbin,low,high);
         }
         else YS__warnmsg("Queue time statistics collection already set");
      }
      else YS__errmsg("Invalid statistic type for queues, can't collect");
   }
   else  {                                    /* Object is a RESOURCE                 */
      if (type == LENGTH) {                   /* Collect queue length stats           */
         if (qptr->lengthstat == NULL)  {     /* Stat collection not yet initiated    */
            strcat(namestr,".length");        /* Tack ".length" onto the queue name   */
            qptr->lengthstat =                /* Get a new statrec & update it        */
               NewStatrec(namestr,INTERVAL,meanflg,histflg,nbin,low,high);
            if (rptr->qdisc != PROCSHAR)      /* Length = # in queue + # served       */
               StatrecUpdate(qptr->lengthstat,
                      (double)(rptr->size + rptr->servers - rptr->freeservers),
                      YS__Simtime);
            else                              /* # in queue includes # served         */
               StatrecUpdate(qptr->lengthstat, (double)(qptr->size), YS__Simtime);
         }
         else YS__warnmsg("Queue length statistics collection already set");
      }
      else if (type == TIME) {                /* Collect time in queue stats          */
         if (qptr->timestat == NULL) {        /* Stat collection not yet initiated    */
            strcat(namestr,".time");          /* Tack ".time" onto the queue naem     */
            qptr->timestat =                  /* Get a new statrec                    */
               NewStatrec(namestr,POINT,meanflg,histflg,nbin,low,high);
         }
         else YS__warnmsg("Queue time statistics collection already set");
      }
      else if (type == UTIL) {                /* Collect utilization stats            */
         if (rptr->utilstat == NULL) {        /* Stat collection not yet initiated    */
            strcat(namestr,".util");          /* Tack ".util" onto the queue name     */
            rptr->utilstat =                  /* Get a new statrec and update it      */
               NewStatrec(namestr,INTERVAL,meanflg,histflg,nbin,low,high);
            if (rptr->qdisc == PROCSHAR)      /* All servers are always busy          */
               StatrecUpdate(rptr->utilstat, (double)(rptr->size), YS__Simtime);
            else
               StatrecUpdate(rptr->utilstat,  /* Only some servers may be busy        */
                             (double)(rptr->servers - rptr->freeservers), YS__Simtime);
         }
         else YS__warnmsg("Queue utilization statistics collection already set");
      }
      else YS__warnmsg("Invalid statistic type for queues, can't collect");
   }
}

/**************************************************************************************/

void QueueResetStats(qptr)       /* Resest a statistics record of a queue             */
SYNCQUE *qptr;                   /* Pointer to the queue                              */
{
   RESOURCE *rptr = (RESOURCE*)qptr;      /* Only cast qptr once                      */

   if (qptr->lengthstat != NULL)  {       /* Collecting queue length stats            */
      StatrecReset(qptr->lengthstat);     /* Reset the statrec                        */
      if (qptr->type == RESTYPE && ((RESOURCE*)qptr)->qdisc != PROCSHAR)
         StatrecUpdate(qptr->lengthstat,  /* Length = # in queue + # served           */
                (double)(qptr->size + rptr->servers - rptr->freeservers),
                YS__Simtime);
      else                                /* # in queue = # served                    */
         StatrecUpdate(qptr->lengthstat, (double)(qptr->size), YS__Simtime);
   }
   if (qptr->timestat != NULL)  {         /* Collecting time in queue stats           */
      StatrecReset(qptr->timestat);       /* Reset the statrec                        */
   }
   if (rptr->type == RESTYPE && rptr->utilstat != NULL)  {
      StatrecReset(rptr->utilstat);       /* Only resources can collect utilizations  */
      if (rptr->qdisc == PROCSHAR)        /* All servers are always busy              */
         StatrecUpdate(rptr->utilstat, (double)(rptr->size), YS__Simtime);
      else                                /* Only some servers may be busy            */
         StatrecUpdate(rptr->utilstat,
                       (double)(rptr->servers - rptr->freeservers), YS__Simtime);
   }
}

/**************************************************************************************/

STATREC *QueueStatPtr(qptr,type)     /* Returns a pointer to a queue's statrec        */
SYNCQUE *qptr;                       /* Pointer to the queue                          */
int type;                            /* LENGTH, TIME, UTIL                            */
{
   RESOURCE *rptr = (RESOURCE*)qptr;        /* Only cast qptr once                    */

   if (type == LENGTH)  {                   /* Get pointer to the length statrec      */
      if (qptr->lengthstat)                 /* Length stats collected                 */
         if (qptr->type == RESTYPE && rptr->qdisc != PROCSHAR)
            StatrecUpdate(qptr->lengthstat, /* Terminate last interval for statrec    */
                   (double)(qptr->size + rptr->servers - rptr->freeservers),
                   YS__Simtime);
         else
            StatrecUpdate(qptr->lengthstat, (double)(qptr->size), YS__Simtime);
      return qptr->lengthstat;              /* Returns NULL if not collected          */
   }
   if (type == TIME)  {                     /* Get pointer to the time statrec        */
      return qptr->timestat;                /* Returns NULL if not collected          */
   }
   if (type == UTIL && rptr->utilstat)  {   /* Get pointer to the utilization statrec */
      if (rptr->qdisc == PROCSHAR)          /* Terminate last interval for statrec    */
         StatrecUpdate(rptr->utilstat, (double)(rptr->size), YS__Simtime);
      else
         StatrecUpdate(rptr->utilstat,
                       (double)(rptr->servers - rptr->freeservers), YS__Simtime);
      return rptr->utilstat;                 /* Returns NULL if not collected          */
   }
   else YS__warnmsg("Invalid statistic type for queues");
   return NULL;
}

/**************************************************************************************/

QUEUE *YS__NewQueue(qname)    /* Create and return a pointer to a new queue           */
char *qname;                  /* User defined name                                    */
{
   QUEUE *qptr;

   qptr = (QUEUE*)YS__PoolGetObj(&YS__QueuePool);

   qptr->id = YS__idctr++;         /* QE fields */
   qptr->type = QUETYPE;

   strncpy(qptr->name,qname,31);   /* QUEUE fields */
   qptr->name[31] = '\n';
   qptr->head = NULL;
   qptr->tail = NULL;
   qptr->size = 0;

   return qptr;
}

/**************************************************************************************/

char  *YS__QueueGetHead(qptr)  /* Removes & returns a pointer to the head of a queue  */
SYNCQUE *qptr;                 /* Pointer to the queue                                */
{
   QE *qeptr;

   if (qptr == NULL) YS__errmsg("Null queue pointer passed to QueueGetHead");
   if (qptr->head != NULL)  {         /* Queue not empty                              */
      qeptr = qptr->head;                  /* Get the head element                    */
      qptr->head = qeptr->next;            /* Head points to the new head element     */
      qeptr->next = NULL;                  /* Clear next pointer of removed element   */
      if (qptr->head == NULL)
         qptr->tail = NULL;                /* Queue now empty                         */
      qptr->size--;                   /* One fewer element in the queue               */
      if (qptr->type == SYNCQTYPE) {  /* Resource statistics updated in userq.c       */
         if (qptr->lengthstat)        /* Queue length statistics collectd             */
            StatrecUpdate(qptr->lengthstat,(double)qptr->size,YS__Simtime);
         if (qptr->timestat)          /* Queue time statistics collected              */
            StatrecUpdate(qptr->timestat,
               YS__Simtime - ((STATQE*)qeptr)->enterque,1.0);
      }
      TRACE_QUEUE_gethead1;           /* Getting from head of queue                   */
   }
   else  {
      TRACE_QUEUE_gethead2;           /* Queue empty                                  */
      qeptr = NULL;                   /* Return NULL                                  */
   }
   TRACE_QUEUE_show(qptr);            /* Prints the contents of the queue             */
   return (char*)qeptr;
}

/**************************************************************************************/

void YS__QueuePutHead(qptr,qeptr)  /* Puts a queue element at the head of a queue     */
QUEUE  *qptr;                      /* Pointer to the queue                            */
QE *qeptr;                         /* Pointer the element to be added                 */
{
   SYNCQUE *sqptr = (SYNCQUE*)qptr;  /* Only cast qptr once                           */
   STATQE  *sqeptr = (STATQE*)qeptr; /* Only cast qeptr once                          */

   TRACE_QUEUE_puthead;              /* Putting at head of queue                      */
   if (qptr == NULL) YS__errmsg("Null queue pointer passed to QueuePutHead");
   if (qeptr == NULL) YS__errmsg("Null queue element pointer passed to QueuePutHead");
   if (qptr->head == NULL) {         /* The queue is empty */
      qptr->tail = qeptr;                 /* Head and tail elements are the same      */
      qeptr->next = NULL;                 /*    (just to be sure)                     */
   }
   else {                            /* Adding to the head of a non-empty queue       */
      qeptr->next = qptr->head;           /* Set forward link of new head element     */
   }                                      /*    (just to be sure)                     */
   qptr->head = qeptr;               /* New element in now the head                   */
   qptr->size++;                     /* One more element in the queue                 */

   if (qptr->type == SYNCQTYPE) {    /* Resource statistics updated in userq.c        */
      sqeptr->enterque = YS__Simtime;/* For time in queue statistics                  */
      if (sqptr->lengthstat)         /* Queue length statistics collected             */
         StatrecUpdate(sqptr->lengthstat,(double)qptr->size,YS__Simtime);
   }
   TRACE_QUEUE_show(qptr);           /* Prints the contents of the queue              */
}

/**************************************************************************************/

void YS__QueuePutTail(qptr,qeptr) /* Puts a queue element at the tail of a queue      */
QUEUE *qptr;                      /* Pointer to the queue                             */
QE    *qeptr;                     /* Pointer the element to be added                  */
{
   SYNCQUE *sqptr = (SYNCQUE*)qptr;  /* Only cast qptr once                           */
   STATQE  *sqeptr = (STATQE*)qeptr; /* Only cast qeptr once                          */

   TRACE_QUEUE_puttail;              /* Putting on the tail of queue                  */
   if (qptr == NULL) YS__errmsg("Null queue pointer passed to QueuePutTail");
   if (qeptr == NULL) YS__errmsg("Null queue element pointer passed to QueuePutTail");
   if (qptr->head == NULL) {         /* The queue is empty                            */
      qptr->head = qeptr;              /* Tail and head elements are the same         */
      qeptr->next = NULL;              /*    (just to be sure)                        */
   }
   else {                            /* Adding to the tail of a non-empty queue       */
      qptr->tail->next = qeptr;        /* Set forward link of old tail element        */
      qeptr->next = NULL;              /* Clear next pointer of new tail element      */
   }                                   /*    (just to be sure)                        */
   qptr->tail = qeptr;               /* New element is now the tail */
   qptr->size++;                     /* One more element in the queue                 */

   if (qptr->type == SYNCQTYPE) {    /* Resource statistics done in userq.c           */
      sqeptr->enterque = YS__Simtime;/* For time in queue statistics                  */
      if (sqptr->lengthstat)         /* Queue length statistics collected             */
         StatrecUpdate(sqptr->lengthstat,(double)qptr->size,YS__Simtime);
   }
   TRACE_QUEUE_show(qptr);           /* Prints the contents of the queue              */
}

/**************************************************************************************/

char *YS__QueueNext(qptr,qeptr)       /* Get pointer to element following an element  */
QUEUE *qptr;                          /* Pointer to the queue                         */
QE *qeptr;                            /* Pointer to an element in the queue           */
{
   QE *eptr;

   if (qptr == NULL) YS__errmsg("Null queue pointer passed to QueueNext");
   if (qptr->size <= 0) eptr = NULL;  /* Return NULL if the queue is empty            */
   else if (qeptr == NULL)
      eptr = (QE*)(qptr->head);       /* Return pointer to head if qeptr is NULL      */
   else eptr = (QE*)(qeptr->next);    /* Return next element otherwise                */
   TRACE_QUEUE_next;                  /* The element after ..., The head of ...       */
   return (char*)eptr;
}

/**************************************************************************************/

int YS__QueueCheckElement(qptr,qeptr) /* Checks to see if an element is in a queue    */
QUEUE *qptr;                          /* Pointer to the queue                         */
QE    *qeptr;                         /* Pointer to element                           */
{
   QE *ptr;

   if (qptr == NULL)
      YS__errmsg("Null queue pointer passed to QueueCheckElement");
   if (qeptr == NULL)
      YS__errmsg("Null queue element pointer passed to QueueCheckElement");

   for (ptr = qptr->head; ptr != NULL; ptr = ptr->next)
      if (ptr == qeptr) {
         TRACE_QUEUE_checkelem1;      /* Element in queue                             */
         return 1;
      }
   TRACE_QUEUE_checkelem2;            /* Element not it queue                         */
   return 0;
}

/**************************************************************************************/

int YS__QueueDelete(qptr,qeptr)       /* Removes a specified element from the queue   */
QUEUE *qptr;                          /* Pointer to the queue                         */
QE *qeptr;                            /* Pointer to element to be removed             */
/* Returns 1 if the element is in the queue and 0 if not                              */
{
   QE *eptr, *preveptr;
   SYNCQUE *sqptr = (SYNCQUE*)qptr;   /* Only cast qptr once                          */
   STATQE  *sqeptr = (STATQE*)qeptr;  /* Only cast qeptr once                         */

   if (qptr == NULL) YS__errmsg("Null queue pointer passed to QueueDelete");
   if (qeptr == NULL) YS__errmsg("Null queue element pointer passed to QueueDelete");
   TRACE_QUEUE_takeout;               /* Taking element from queue q                  */

   preveptr = qptr->head;             /* Start at the head                            */
   for (eptr = qptr->head; eptr != NULL; eptr = eptr->next)  { /* and search for it   */
      if (eptr == qeptr)  {                                    /* Found it            */
         if (eptr == (qptr->head))    /* Element at the head of the queue             */
            qptr->head = eptr->next;
         else                         /* Element in the middle of the queue           */
            preveptr->next = eptr->next;
         if (eptr == (qptr->tail)) {  /* Element at the tail of the queue             */
             if (qptr->size == 1)     /* this was the only element in the queue       */
               preveptr = NULL;
             qptr->tail = preveptr;
         }
         eptr->next = NULL;
         qptr->size--;                 /* Queue has one fewer element
         TRACE_QUEUE_show(qptr);       /* Prints the contents of the queue            */

         if (qptr->type == SYNCQTYPE) {/* Resource statistics updated in userq.c      */
            if (sqptr->lengthstat)     /* Queue length statistics collected           */
               StatrecUpdate(sqptr->lengthstat,(double)qptr->size,YS__Simtime);
            if (sqptr->timestat)       /* Queue time statistics collected             */
               StatrecUpdate(sqptr->timestat, YS__Simtime - sqeptr->enterque,1.0);
         }
         return 1;                     /* Element deleted                             */
      }
      preveptr = eptr;
   }
   TRACE_QUEUE_show(qptr);             /* Prints the contents of the queue            */
   return 0;                           /* Element not in queue                        */
}

/**************************************************************************************/

void YS__QueueReset(qptr)   /* Removes and frees all of the queue's elements          */
QUEUE *qptr;                /* Pointer to the queue                                   */
{
   ACTIVITY *aptr;

   TRACE_QUEUE_reset;       /* Clearing queue                                         */
   qptr->head = NULL;
   qptr->tail = NULL;
   qptr->size = 0;
   QueueResetStats(qptr);
   TRACE_QUEUE_show(qptr);  /* Prints the contents of the queue                       */
}

/**************************************************************************************/

double YS__QueueHeadval(qptr)    /* Returns the time of the first queue element       */
QUEUE *qptr;                     /* Pointer to the queue                              */
{
   double retval;

   if (qptr->head != NULL)
      retval = ((ACTIVITY*)(qptr->head))->time;
   else retval = -1.0;
   TRACE_QUEUE_headvalue;        /* Value of head of queue is                         */
   return retval;
}

/**************************************************************************************/

void YS__QueueInsert(qptr,aptr)  /* Inserts an element in order of its time, FIFO     */
QUEUE *qptr;                     /* Pointer to the queue                              */
ACTIVITY *aptr;                  /* Pointer to the element to insert                  */

/* This operation was originally used to implement the event list.  That is now done  */
/* by special routines in the file evlst.c.  The operation  is only used now for      */
/* inserting activities into Shortest Job Next resources                              */

{
   ACTIVITY *eptr;
   SYNCQUE *sqptr = (SYNCQUE*)qptr; /* Only cast qptr once                            */

   TRACE_QUEUE_insert;              /* Inserting element with time into queue         */
   if (qptr->head == NULL)  {       /* Queue is empty                                 */
      qptr->head = (QE*)aptr;
      qptr->tail = (QE*)aptr;
      aptr->next = NULL;
      qptr->size++;
   }
   else {
      if (aptr->time < ((ACTIVITY*)(qptr->head))->time) { /* put elem at queue head   */

         aptr->next = qptr->head;
         qptr->head = (QE*)aptr;
         qptr->size++;
      }
      else {
         if ( aptr->time >= ((ACTIVITY*)(qptr->tail))->time) { /* Put elem at tail    */

            qptr->tail->next = (QE*)aptr;
            aptr->next = NULL;
            qptr->tail = (QE*)aptr;
            qptr->size++;
         }
         else  {                                   /* put element in middle of queue  */
            eptr = (ACTIVITY*)(qptr->head);
            while (aptr->time >= ((ACTIVITY*)(eptr->next))->time)
               eptr = (ACTIVITY*)(eptr->next);     /* Find where it goes              */
            aptr->next = eptr->next;               /* and then put it in              */
            eptr->next = (QE*)aptr;
            qptr->size++;
   }  }  }
   if (qptr->type == SYNCQTYPE) {     /* Resource statistics updated in userq.c       */
      aptr->enterque = YS__Simtime;   /* For time statistics                          */
      if (sqptr->lengthstat)          /* Queue length statistics collected            */
         StatrecUpdate(sqptr->lengthstat,(double)qptr->size,YS__Simtime);
   }
   TRACE_QUEUE_show(qptr);            /* Prints the contents of the queue             */
}

/**************************************************************************************/

void YS__QueueEnter(qptr,aptr) /* Enters an element in order of its time, LIFO        */
QUEUE *qptr;                   /* Pointer to the queue                                */
ACTIVITY *aptr;                /* Pointer to the element to enter                     */

/* This operation is not used anymore.  It was originally used to implement the event */
/* list.  That is now done by special routines in the file evlst.c.                   */

{
   ACTIVITY *eptr;
   SYNCQUE *sqptr = (SYNCQUE*)qptr; /* Only cast qptr once                            */

   TRACE_QUEUE_enter;               /* Entering element with time into queue          */
   if (qptr->head == NULL)  {       /* Queue is empty                                 */
      qptr->head = (QE*)aptr;
      qptr->tail = (QE*)aptr;
      aptr->next = NULL;
      qptr->size++;
   }
   else  {
      if ( aptr->time <= ((ACTIVITY*)(qptr->head))->time ) { /* put elem at head      */
         aptr->next = qptr->head;
         qptr->head = (QE*)aptr;
         qptr->size++;
      }
      else  {
         if (aptr->time > ((ACTIVITY*)(qptr->tail))->time) { /* put elem at tail      */
            qptr->tail->next = (QE*)aptr;
            aptr->next = NULL;
            qptr->tail = (QE*)aptr;
            qptr->size++;
         }
         else  {                                /* put element in middle of queue     */
            eptr = (ACTIVITY*)qptr->head;
            while (aptr->time > ((ACTIVITY*)(eptr->next))->time)
               eptr = (ACTIVITY*)eptr->next;    /* Find where it goes                 */
            aptr->next = eptr->next;            /* and put it in                      */
            eptr->next = (QE*)aptr;
            qptr->size++;
   }  }  }
   if (qptr->type == SYNCQTYPE) {     /* Resource statistics updated in userq.c       */
      aptr->enterque = YS__Simtime;   /* For time statistics                          */
      if (sqptr->lengthstat)          /* Queue length statistics collected            */
         StatrecUpdate(sqptr->lengthstat,(double)qptr->size,YS__Simtime);
   }
   TRACE_QUEUE_show(qptr);         /* Prints the contents of the queue                */
}

/**************************************************************************************/

void YS__QueuePrint(qptr)       /* Prints the contents of a queue                     */
QUEUE *qptr;                    /* Pointer to the queue                               */
{
   QE *qeptr = NULL;
   int i;

   if (qptr == NULL) YS__errmsg("Null queue pointer passed to QueuePrint\n");
   else {
      i = 0;
      sprintf(YS__prbpkt,"            Queue %s contents:\n",qptr->name);
      YS__SendPrbPkt(TEXTPROBE,qptr->name,YS__prbpkt);
      qeptr = qptr->head;
      while(qeptr != NULL) {
         sprintf(YS__prbpkt,"              Qelem %d is %s\n",i,qeptr->name);
         YS__SendPrbPkt(TEXTPROBE,qptr->name,YS__prbpkt);
         qeptr = qeptr->next;
         i++;
      }
      if (i == 0) {
         sprintf(YS__prbpkt,"              Queue Empty\n");
         YS__SendPrbPkt(TEXTPROBE,qptr->name,YS__prbpkt);
      }
   }
}

/**************************************************************************************/

int YS__QeId(qeptr)         /* Returns the system defined ID or 0 if TrID is 0        */
QE *qeptr;                  /* Pointer to the queue element                           */
{
   if (TraceIDs)
      return qeptr->id;
   else return 0;
}

/**************************************************************************************/
