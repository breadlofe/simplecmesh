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

#ifndef UQUEUESTRH
#define UQUEUESTRH

#ifdef debug  /************************************************************************\
*                            SEMAPHORE tracing statements                              *
\**************************************************************************************/

#define TRACE_SEMAPHORE_new  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt,"    Creating semaphore %s[%d] with value %d\n",  \
              semptr->name,YS__QeId(semptr),semptr->val); \
      YS__SendPrbPkt(TEXTPROBE,semptr->name,YS__prbpkt); \
   }

#define TRACE_SEMAPHORE_init  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt,"    Initializing semaphore %s[%d]; new value = %d\n",  \
              sptr->name,YS__QeId(sptr),i); \
      YS__SendPrbPkt(TEXTPROBE,sptr->name,YS__prbpkt); \
   }

#define TRACE_SEMAPHORE_p1  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      YS__SendPrbPkt(TEXTPROBE,sptr->name,YS__prbpkt); \
   }

#define TRACE_SEMAPHORE_p2  \
   if (TraceLevel >= MAXDBLEVEL-2) {\
   sptr->name,YS__QeId(sptr),sptr->val); \
      YS__SendPrbPkt(TEXTPROBE,sptr->name,YS__prbpkt); \
   }


#define TRACE_SEMAPHORE_v1p  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      if (YS__ActEvnt != NULL) { \
         sprintf(YS__prbpkt,"    Event %s[%d] signalling semaphore %s[%d]\n", \
                 YS__ActEvnt->name,YS__ActId(YS__ActEvnt),sptr->name,YS__QeId(sptr)); \
         sprintf(YS__prbpkt+strlen(YS__prbpkt), \
            "    - No processes or events waiting; new semaphore value = %d\n", \
                 sptr->val); \
      }  \
      YS__SendPrbPkt(TEXTPROBE,sptr->name,YS__prbpkt); \
   }

#define TRACE_SEMAPHORE_v2p  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      if (YS__ActEvnt != NULL)  \
         sprintf(YS__prbpkt,"    Event %s[%d] signalling semaphore %s[%d]\n",  \
                 YS__ActEvnt->name,YS__ActId(YS__ActEvnt),sptr->name,YS__QeId(sptr)); \
      YS__SendPrbPkt(TEXTPROBE,sptr->name,YS__prbpkt); \
   }

#define TRACE_SEMAPHORE_v3  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt, \
         "    - Activity %s[%d] released; semaphore value unchanged\n", \
              aptr->name,YS__ActId(aptr)); \
      YS__SendPrbPkt(TEXTPROBE,aptr->name,YS__prbpkt); \
   }

#define TRACE_SEMAPHORE_s1p  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      if (YS__ActEvnt != NULL) { \
         sprintf(YS__prbpkt,"    Event %s[%d] setting semaphore %s[%d]\n", \
                 YS__ActEvnt->name,YS__ActId(YS__ActEvnt),sptr->name,YS__QeId(sptr)); \
         sprintf(YS__prbpkt+strlen(YS__prbpkt), \
            "    - No processes or events waiting; new semaphore value = %d\n", \
                 sptr->val); \
      }  \
      YS__SendPrbPkt(TEXTPROBE,sptr->name,YS__prbpkt); \
   }

#define TRACE_SEMAPHORE_s2p  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      if (YS__ActEvnt != NULL)  \
         sprintf(YS__prbpkt,"    Event %s[%d] setting semaphore %s[%d]\n",  \
                 YS__ActEvnt->name,YS__ActId(YS__ActEvnt),sptr->name,YS__QeId(sptr)); \
      YS__SendPrbPkt(TEXTPROBE,sptr->name,YS__prbpkt); \
   }

/**************************************************************************************\
*                             BARRIER tracing statements                               *
\**************************************************************************************/

#define TRACE_BARRIER_new  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt,"    Creating barrier %s[%d] with count %d\n",  \
              barptr->name,YS__QeId(barptr),barptr->cnt); \
      YS__SendPrbPkt(TEXTPROBE,barptr->name,YS__prbpkt); \
   }

#define TRACE_BARRIER_init  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
         sprintf(YS__prbpkt,"    Initializing Barrier %s[%d]; new count = %d\n", \
                 bptr->name,YS__QeId(bptr),i); \
      YS__SendPrbPkt(TEXTPROBE,bptr->name,YS__prbpkt); \
   }

#define TRACE_BARRIER_sync1  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt,"    Process %s[%d] synchronizing on barrier %s[%d]\n",  \
      YS__SendPrbPkt(TEXTPROBE,bptr->name,YS__prbpkt); \
   }

#define TRACE_BARRIER_sync2  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt,"    - %d processes waiting at barrier;  process waits\n", \
              bptr->initcnt - bptr->cnt); \
      YS__SendPrbPkt(TEXTPROBE,bptr->name,YS__prbpkt); \
   }

#define TRACE_BARRIER_sync3  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt,"    - Releasing all waiting processes\n"); \
      YS__SendPrbPkt(TEXTPROBE,bptr->name,YS__prbpkt); \
   }

#define TRACE_BARRIER_sync4  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt,"    - Releasing process %s[%d]\n", \
              procptr->name,YS__ActId(procptr)); \
      YS__SendPrbPkt(TEXTPROBE,procptr->name,YS__prbpkt); \
   }

/**************************************************************************************\
*                             FLAG tracing statements                                *
\**************************************************************************************/

#define TRACE_FLAG_new  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt,"    Creating flag %s[%d]\n",  \
              flgptr->name,YS__QeId(flgptr)); \
      YS__SendPrbPkt(TEXTPROBE,flgptr->name,YS__prbpkt); \
   }

#define TRACE_FLAG_wait1  \
   if (TraceLevel >= MAXDBLEVEL-2) {  \
      sprintf(YS__prbpkt,"    Process %s[%d] waiting at flag %s[%d]\n",  \
      YS__SendPrbPkt(TEXTPROBE,fptr->name,YS__prbpkt); \
   }

#define TRACE_FLAG_wait2  \
   if (TraceLevel >= MAXDBLEVEL-2) {  \
      sprintf(YS__prbpkt,"    - Flag not set, process suspends\n");  \
      YS__SendPrbPkt(TEXTPROBE,fptr->name,YS__prbpkt); \
   }

#define TRACE_FLAG_wait3  \
   if (TraceLevel >= MAXDBLEVEL-2) {  \
      sprintf(YS__prbpkt,"    - Flag is set, process continues\n"); \
      YS__SendPrbPkt(TEXTPROBE,fptr->name,YS__prbpkt); \
   }

#define TRACE_FLAG_set1p  \
   if (TraceLevel >= MAXDBLEVEL-2) {  \
      if (YS__ActEvnt != NULL)  \
         sprintf(YS__prbpkt,"    Event %s[%d] setting flag %s[%d]\n",  \
                 YS__ActEvnt->name,YS__ActId(YS__ActEvnt),fptr->name,YS__QeId(fptr)); \
      else  \
         sprintf(YS__prbpkt,"    User program setting flag %g[%d]\n",  \
                 fptr->name,YS__QeId(fptr));  \
      sprintf(YS__prbpkt+strlen(YS__prbpkt),"    - %d activities waiting\n",num); \
      YS__SendPrbPkt(TEXTPROBE,fptr->name,YS__prbpkt); \
   }

#define TRACE_FLAG_flgrelease  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    - Activity %s[%d] released from flag %s[%d]\n", \
              actptr->name,YS__ActId(actptr),fptr->name,YS__ActId(fptr)); \
     YS__SendPrbPkt(TEXTPROBE,actptr->name,YS__prbpkt); \
   }

#define TRACE_FLAG_release1p  \
   if (TraceLevel >= MAXDBLEVEL-2) {  \
      if (YS__ActEvnt != NULL)  \
         sprintf(YS__prbpkt, \
            "    Event %s[%d] releasing all activities waiting at flag %s[%d]\n", \
                 YS__ActEvnt->name,YS__ActId(YS__ActEvnt),fptr->name,YS__QeId(fptr)); \
      else  \
         sprintf(YS__prbpkt,"    User program setting flag %s[%d]\n",  \
                 fptr->name,YS__QeId(fptr));  \
      sprintf(YS__prbpkt+strlen(YS__prbpkt),"    - %d activities waiting\n",num); \
      YS__SendPrbPkt(TEXTPROBE,fptr->name,YS__prbpkt); \
   }

/**************************************************************************************\
*                              STVAR tracing statements                                *
\**************************************************************************************/

#define TRACE_IVAR_new  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
         sprintf(YS__prbpkt, \
            "    Creating integer-valued state variable %s[%d] = %d\n", \
                 svptr->name,YS__SvId(svptr),svptr->val.ival);  \
      YS__SendPrbPkt(TEXTPROBE,svptr->name,YS__prbpkt); \
   }

#define TRACE_FVAR_new  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
         sprintf(YS__prbpkt,"    Creating real valued state variable %s[%d] = %g\n", \
                 svptr->name,YS__SvId(svptr),svptr->val.fval);  \
      YS__SendPrbPkt(TEXTPROBE,svptr->name,YS__prbpkt); \
   }

#define TRACE_STVAR_set  \
   if (TraceLevel >= MAXDBLEVEL-2) {  \
      if (svptr->type == IVARTYPE) \
         sprintf(YS__prbpkt,"    State Variable %s[%d] modified; new value = %d\n", \
                 svptr->name,YS__SvId(svptr),svptr->val.ival); \
      if (svptr->type == FVARTYPE) \
         sprintf(YS__prbpkt,"    State Variable %s[%d] modified; new value = %g\n", \
                 svptr->name,YS__SvId(svptr),svptr->val.fval); \
      YS__SendPrbPkt(TEXTPROBE,svptr->name,YS__prbpkt); \
   }

#define TRACE_STVAR_condrelease  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt,"    - Activity %s[%d] released from condition %s[%d]\n", \
              actproc->name,YS__ActId(actproc), \
              ((CONDITION*)(qeptr->optr))->name,YS__ActId(qeptr->optr)); \
      YS__SendPrbPkt(TEXTPROBE,actproc->name,YS__prbpkt); \
   }

#define TRACE_STVAR_evalcond   \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    Evaluating condition %s[%d]; new state = %d\n", \
              ((CONDITION*)(qeptr->optr))->name, \
              YS__QeId(qeptr->optr), \
              ((CONDITION*)(qeptr->optr))->state); \
      YS__SendPrbPkt(TEXTPROBE,"Driver",YS__prbpkt); \
   }

#define TRACE_QUEUE_show(ptr)  \
   if (TraceLevel >= MAXDBLEVEL) YS__QueuePrint(ptr);

/**************************************************************************************\
*                           CONDITION tracing statements                               *
\**************************************************************************************/

#define TRACE_CONDITION_new  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt,"    Creating condition %s[%d]\n",  \
              condptr->name,YS__QeId(condptr)); \
      YS__SendPrbPkt(TEXTPROBE,condptr->name,YS__prbpkt); \
   }

#define TRACE_CONDITION_wait1  \
   if (TraceLevel >= MAXDBLEVEL-2) {  \
      sprintf(YS__prbpkt,"    Process %s[%d] waiting at condition %s[%d]\n", \
      YS__SendPrbPkt(TEXTPROBE,cptr->name,YS__prbpkt); \
   }

#define TRACE_CONDITION_wait2  \
   if (TraceLevel >= MAXDBLEVEL-2) {  \
      sprintf(YS__prbpkt,"    Condition false, process suspends\n");  \
      YS__SendPrbPkt(TEXTPROBE,cptr->name,YS__prbpkt); \
   }

#define TRACE_CONDITION_wait3  \
   if (TraceLevel >= MAXDBLEVEL-2) {  \
      sprintf(YS__prbpkt,"    Condition true, process continues\n");  \
      YS__SendPrbPkt(TEXTPROBE,cptr->name,YS__prbpkt); \
   }

/**************************************************************************************\
*                            RESOURCE tracing statements                               *
\**************************************************************************************/

#define TRACE_RESOURCE_new  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt,"    Creating resource %s[%d] with %s queueing discipline\n", \
              rptr->name,YS__QeId(rptr),qdiscname[rptr->qdisc]); \
      YS__SendPrbPkt(TEXTPROBE,rptr->name,YS__prbpkt); \
   }

#define TRACE_RESOURCE_use1  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt+strlen(YS__prbpkt), \
         "%g unit(s) of service from resource %s[%d]\n", \
              timeinc,rptr->name,YS__QeId(rptr)); \
      YS__SendPrbPkt(TEXTPROBE,rptr->name,YS__prbpkt); \
   }

#define TRACE_RESOURCE_use2 \
   if (TraceLevel >= MAXDBLEVEL-1) { \
      sprintf(YS__prbpkt, \
          "        Activity %s[%d] added to processor %s[%d]'s ready list\n", \
             aptr->name,YS__ActId(aptr),rptr->name,YS__QeId(rptr)); \
      YS__SendPrbPkt(TEXTPROBE,aptr->name,YS__prbpkt); \
    }

#define TRACE_RESOURCE_use3y  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt, \
         "    Activity %s[%d] completes service from resource %s[%d]\n", \
              aptr->name,YS__ActId(aptr),rptr->name,YS__QeId(rptr));  \
      YS__SendPrbPkt(TEXTPROBE,rptr->name,YS__prbpkt); \
   }

#define TRACE_RESOURCE_use3p  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      if (aptr->pp != NULL && aptr->rscptr == aptr->pp->RdyList) { \
         sprintf(YS__prbpkt, \
            "    Activity %s[%d] completes service from processor %s[%d]\n", \
                 aptr->name,YS__ActId(aptr),rptr->name,YS__QeId(rptr));  \
         YS__SendPrbPkt(TEXTPROBE,rptr->name,YS__prbpkt); \
      } \
      else { \
         sprintf(YS__prbpkt, \
            "    Activity %s[%d] completes service from resource %s[%d]\n", \
                 aptr->name,YS__ActId(aptr),rptr->name,YS__QeId(rptr));  \
         YS__SendPrbPkt(TEXTPROBE,rptr->name,YS__prbpkt); \
      } \
   }

#define TRACE_RESOURCE_use4y  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt, \
         "    Activity %s[%d] completes ",aptr->name,YS__ActId(aptr)); \
      sprintf(YS__prbpkt+strlen(YS__prbpkt), \
         "a time slice of service from resource %s[%d]\n",rptr->name,YS__QeId(rptr)); \
      YS__SendPrbPkt(TEXTPROBE,rptr->name,YS__prbpkt); \
   }

#define TRACE_RESOURCE_use4p  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      if (aptr->pp != NULL && aptr->rscptr == aptr->pp->RdyList) { \
         sprintf(YS__prbpkt, \
            "    Activity %s[%d] completes ",aptr->name,YS__ActId(aptr)); \
         sprintf(YS__prbpkt+strlen(YS__prbpkt), \
            "a time slice of service from processor %s[%d]\n", \
            rptr->name,YS__QeId(rptr)); \
         YS__SendPrbPkt(TEXTPROBE,rptr->name,YS__prbpkt); \
      } \
      else  { \
         sprintf(YS__prbpkt, \
            "    Activity %s[%d] completes ",aptr->name,YS__ActId(aptr)); \
         sprintf(YS__prbpkt+strlen(YS__prbpkt), \
            "a time slice of service from resource %s[%d]\n",rptr->name,YS__QeId(rptr)); \
         YS__SendPrbPkt(TEXTPROBE,rptr->name,YS__prbpkt); \
      } \
   }

#define TRACE_RESOURCE_use5  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    Process %s[%d] completes ",pptr->name,YS__ActId(pptr)); \
      sprintf(YS__prbpkt+strlen(YS__prbpkt), \
         "a time slice of service from processor %s[%d]\n",rptr->name,YS__QeId(rptr)); \
      YS__SendPrbPkt(TEXTPROBE,rptr->name,YS__prbpkt); \
   }

#define TRACE_RESOURCE_use6  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt, \
         "    Process %s[%d] completes service from processor %s[%d]\n", \
              pptr->name,YS__ActId(pptr),rptr->name,YS__QeId(rptr));  \
      YS__SendPrbPkt(TEXTPROBE,rptr->name,YS__prbpkt); \
   }

#define TRACE_RESOURCE_takeout  \
   if (TraceLevel >= MAXDBLEVEL-1) { \
      sprintf(YS__prbpkt,"        Taking element %s[%d] from queue %sj[%d]\n",  \
              actptr->name,YS__ActId(actptr),rptr->name,YS__QeId(rptr)); \
      YS__SendPrbPkt(TEXTPROBE,rptr->name,YS__prbpkt); \
   }

#define TRACE_RESOURCE_activate1  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt, \
         "    Activity %s[%d] resumes service with %g units of service required\n", \
              actptr->name,YS__ActId(actptr),actptr->time); \
      YS__SendPrbPkt(TEXTPROBE,actptr->name,YS__prbpkt); \
   }

#define TRACE_RESOURCE_activate2  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt, \
         "    Preempting activity %s[%d] with %g units of service still required\n", \
              rptr->serving->name,YS__ActId(rptr->serving), \
              rptr->serving->time - YS__Simtime); \
      YS__SendPrbPkt(TEXTPROBE,rptr->serving->name,YS__prbpkt); \
   }

#define TRACE_RESOURCE_activate3  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt, \
         "    Activity %s[%d] resumes service with %g units of service required\n", \
               prptr->name,YS__ActId(prptr),prptr->time-YS__Simtime); \
      YS__SendPrbPkt(TEXTPROBE,prptr->name,YS__prbpkt); \
   }

#define TRACE_RESOURCE_activate4  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    Adjusting service times for resource %s[%d]; ", \
              rptr->name,YS__QeId(rptr)); \
      sprintf(YS__prbpkt+strlen(YS__prbpkt), \
         "old size = %d, new size = %d\n",rptr->oldsize,rptr->size);  \
      YS__SendPrbPkt(TEXTPROBE,rptr->name,YS__prbpkt); \
   }

#define TRACE_RESOURCE_activate5  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt, \
         "    Activity %s[%d] resumes service with %g units of service required\n", \
              actptr->name,YS__ActId(actptr),actptr->time); \
      YS__SendPrbPkt(TEXTPROBE,actptr->name,YS__prbpkt); \
   }

#else  /*******************************************************************************/

#define TRACE_SEMAPHORE_new
#define TRACE_SEMAPHORE_init
#define TRACE_SEMAPHORE_reset
#define TRACE_SEMAPHORE_p1
#define TRACE_SEMAPHORE_p2
#define TRACE_SEMAPHORE_v1p
#define TRACE_SEMAPHORE_v2p
#define TRACE_SEMAPHORE_v3
#define TRACE_SEMAPHORE_s1p
#define TRACE_SEMAPHORE_s2p

#define TRACE_BARRIER_new
#define TRACE_BARRIER_init
#define TRACE_BARRIER_reset
#define TRACE_BARRIER_sync1
#define TRACE_BARRIER_sync2
#define TRACE_BARRIER_sync3
#define TRACE_BARRIER_sync4

#define TRACE_FLAG_new
#define TRACE_FLAG_wait
#define TRACE_FLAG_set1p
#define TRACE_FLAG_release1p

#define TRACE_RESOURCE_new
#define TRACE_RESOURCE_use1a
#define TRACE_RESOURCE_use1b
#define TRACE_RESOURCE_use1c
#define TRACE_RESOURCE_use1d
#define TRACE_RESOURCE_use1e
#define TRACE_RESOURCE_use1f
#define TRACE_RESOURCE_use1g
#define TRACE_RESOURCE_use1h
#define TRACE_RESOURCE_use1i
#define TRACE_RESOURCE_use1j
#define TRACE_RESOURCE_use3
#define TRACE_RESOURCE_use4
#define TRACE_RESOURCE_use5
#define TRACE_RESOURCE_use6
#define TRACE_RESOURCE_takeout
#define TRACE_RESOURCE_activate1
#define TRACE_RESOURCE_activate2
#define TRACE_RESOURCE_activate3
#define TRACE_RESOURCE_activate4
#define TRACE_RESOURCE_activate5

#define TRACE_IVAR_new
#define TRACE_FVAR_new
#define TRACE_STVAR_notify1
#define TRACE_STVAR_notify2
#define TRACE_STVAR_condrelease
#define TRACE_STVAR_evalcond

#define TRACE_CONDITION_new
#define TRACE_CONDITION_wait1
#define TRACE_CONDITION_wait2
#define TRACE_CONDITION_wait3

#endif  /******************************************************************************/


#endif

