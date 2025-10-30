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

#ifndef ACTTRH
#define ACTTRH

#ifdef debug  /************************************************************************\
*                           ACTIVITY tracing statements                                *
\**************************************************************************************/

#define TRACE_ACTIVITY_setarg  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    Setting argument for %s[%d]\n", \
      aptr->name,YS__ActId(aptr));  \
      YS__SendPrbPkt(TEXTPROBE,aptr->name,YS__prbpkt); \
   }

#define TRACE_ACTIVITY_getarg   \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt,"    Getting argument from %s[%d]\n", \
      aptr->name,YS__ActId(aptr)); \
      YS__SendPrbPkt(TEXTPROBE,aptr->name,YS__prbpkt); \
   }

#define TRACE_ACTIVITY_fork  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt,"    - Forking schedule, parent process is %s[%d]\n",  \
             aptr->parent->name,YS__ActId(aptr->parent)); \
      YS__SendPrbPkt(TEXTPROBE,aptr->name,YS__prbpkt); \
   }

#define TRACE_ACTIVITY_block  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt,"    - Blocking schedule, parent process %s[%d] suspends\n", \
             aptr->parent->name,YS__ActId(aptr->parent)); \
      YS__SendPrbPkt(TEXTPROBE,aptr->name,YS__prbpkt); \
   }

#define TRACE_ACTIVITY_schedule1  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    Scheduling activity %s[%d] to occur in %g time units\n", \
              aptr->name,YS__ActId(aptr),timeinc); \
      YS__SendPrbPkt(TEXTPROBE,aptr->name,YS__prbpkt); \
   }

#define TRACE_ACTIVITY_schedule2  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt, \
         "    Scheduling activity %s[%d] to wait for semaphore %s[%d]\n",  \
         aptr->name,YS__ActId(aptr),sptr->name,YS__ActId(sptr)); \
      YS__SendPrbPkt(TEXTPROBE,aptr->name,YS__prbpkt); \
   }

#define TRACE_ACTIVITY_schedule3 \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt, \
         "    - Semaphore %s[%d] = %d; %s is decremented and %s is activated\n", \
         sptr->name,YS__ActId(sptr),sptr->val,sptr->name,aptr->name); \
      YS__SendPrbPkt(TEXTPROBE,sptr->name,YS__prbpkt); \
   }

#define TRACE_ACTIVITY_schedule4  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt,"    - Semaphore %s[%d] = %d; %s waits\n",  \
              sptr->name,YS__ActId(sptr),sptr->val,aptr->name); \
      YS__SendPrbPkt(TEXTPROBE,sptr->name,YS__prbpkt); \
   }

#define TRACE_ACTIVITY_schedule5  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
        sprintf(YS__prbpkt, \
           "    Scheduling activity %s[%d] to wait for condition %s[%d]\n", \
           aptr->name,YS__ActId(aptr),cptr->name,YS__QeId(cptr)); \
      YS__SendPrbPkt(TEXTPROBE,aptr->name,YS__prbpkt); \
  }

#define TRACE_ACTIVITY_schedule6  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
        sprintf(YS__prbpkt, \
           "    Scheduling activity %s[%d] to use resource %s[%d] for %g time units\n", \
           aptr->name,YS__ActId(aptr),rptr->name,YS__QeId(rptr),timeinc); \
      YS__SendPrbPkt(TEXTPROBE,aptr->name,YS__prbpkt); \
  }

#define TRACE_ACTIVITY_schedule7  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      if (aptr->type == EVTYPE || aptr->type==OSEVTYPE)  \
         sprintf(YS__prbpkt,"    Scheduling event %s[%d] to occur immediately\n", \
                 aptr->name,YS__ActId(aptr)); \
      else if (aptr->type == PROCTYPE || aptr->type==OSPRTYPE || aptr->type==USRPRTYPE) \
         sprintf(YS__prbpkt, \
            "    Scheduling process %s[%d] to start immediately\n",  \
            aptr->name,YS__ActId(aptr));  \
      YS__SendPrbPkt(TEXTPROBE,aptr->name,YS__prbpkt); \
   }

#define TRACE_ACTIVITY_schedule8 \
   if (TraceLevel >= MAXDBLEVEL-2) { \
        sprintf(YS__prbpkt,"    Scheduling activity %s[%d] to wait for flag %s[%d]\n", \
                aptr->name,YS__ActId(aptr),fptr->name,YS__QeId(fptr)); \
      YS__SendPrbPkt(TEXTPROBE,aptr->name,YS__prbpkt); \
  }

#define TRACE_ACTIVITY_schedule11  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    - Condition false, activity scheduled on condition\n"); \
      YS__SendPrbPkt(TEXTPROBE,aptr->name,YS__prbpkt); \
   }

#define TRACE_ACTIVITY_schedule12  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    - Resource ");  \
      if (rptr->status != PENDING) sprintf(YS__prbpkt+strlen(YS__prbpkt), \
         "notified\n");  \
      else sprintf(YS__prbpkt+strlen(YS__prbpkt),"already pending\n");  \
      YS__SendPrbPkt(TEXTPROBE,aptr->name,YS__prbpkt); \
   }

#define TRACE_ACTIVITY_schedule13  \
   if (TraceLevel >= MAXDBLEVEL-2) {  \
      sprintf(YS__prbpkt,"    - Flag not set,activity scheduled to wait on flag\n"); \
      YS__SendPrbPkt(TEXTPROBE,fptr->name,YS__prbpkt); \
   }

#define TRACE_ACTIVITY_schedule14  \
   if (TraceLevel >= MAXDBLEVEL-2) {  \
      sprintf(YS__prbpkt,"    - Flag is set, activity scheduled now\n"); \
      YS__SendPrbPkt(TEXTPROBE,fptr->name,YS__prbpkt); \
   }

#define TRACE_ACTIVITY_schedule15  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    - Condition true, activity scheduled now\n"); \
      YS__SendPrbPkt(TEXTPROBE,aptr->name,YS__prbpkt); \
   }

/**************************************************************************************\
*                           PROCESS Tracing Statements                                 *
\**************************************************************************************/

#define TRACE_PROCESS_process  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    Creating process %s[%d]\n",prptr->name,YS__ActId(prptr)); \
      YS__SendPrbPkt(TEXTPROBE,prptr->name,YS__prbpkt); \
   }

#define TRACE_PROCESS_transfer  \
   if (TraceLevel >= MAXDBLEVEL-1)  { \
      sprintf(YS__prbpkt, \
         "        Transferring to %s[%d]\n",prptr->name,YS__ActId(prptr)); \
      YS__SendPrbPkt(TEXTPROBE,prptr->name,YS__prbpkt); \
   }

#define TRACE_PROCESS_suspend1  \
   if (TraceLevel >= MAXDBLEVEL-1)  { \
      sprintf(YS__prbpkt, \
         "        Suspending %s[%d]",YS__ActProc->name,YS__ActId(YS__ActProc)); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc,YS__prbpkt); \
   }

#define TRACE_PROCESS_suspend2  \
   if (TraceLevel >= MAXDBLEVEL-1)  { \
       sprintf(YS__prbpkt, \
         " returning to %s[%d]\n",YS__ActProc->name,YS__ActId(YS__ActProc)); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc->name,YS__prbpkt); \
   }

#define TRACE_PROCESS_suspend3  \
   if (TraceLevel >= MAXDBLEVEL-1)  { \
       sprintf(YS__prbpkt,"\n"); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc,YS__prbpkt); \
   }

#define TRACE_PROCESS_sleep  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt, \
         "    Process %s[%d] goes to sleep\n", \
         YS__ActProc->name,YS__ActId(YS__ActProc)); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc->name,YS__prbpkt); \
   }

#define TRACE_PROCESS_wakeup  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt, \
         "    Process %s[%d] wakes up after sleeping\n", \
         YS__ActProc->name,YS__ActId(YS__ActProc)); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc->name,YS__prbpkt); \
   }

#define TRACE_PROCESS_join1  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt, \
         "    Process %s[%d] executing Join\n", \
         YS__ActProc->name,YS__ActId(YS__ActProc)); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc->name,YS__prbpkt); \
   }

#define TRACE_PROCESS_join2  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt, \
         "    - %d active child activities exist, current process waits\n",  \
         YS__ActProc->children); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc->name,YS__prbpkt); \
   }

#define TRACE_PROCESS_join3  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt, \
         "    - No active child activities exist, current process continues\n"); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc->name,YS__prbpkt); \
   }

#define TRACE_PROCESS_delay   \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    Process %s[%d] delaying for %g time units\n",  \
              YS__ActProc->name,YS__ActId(YS__ActProc),timeinc); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc->name,YS__prbpkt); \
   }

#define TRACE_PROCESS_nice  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    Process %s[%d] nicing itself\n",  \
              YS__ActProc->name,YS__ActId(YS__ActProc)); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc->name,YS__prbpkt); \
   }

#define TRACE_PROCESS_waitcpu  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt, \
         "    Process %s[%d] ready for %g time units on processor %s[%d]\n",  \
         YS__ActProc->name,YS__ActId(YS__ActProc),timeinc, \
         YS__ActProc->pp->name,YS__ActId(YS__ActProc->pp)); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc->name,YS__prbpkt); \
   }

#define TRACE_PROCESS_sendmsg1  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      if (YS__ActEvnt != NULL) \
         sprintf(YS__prbpkt, \
            "    Event %s[%d] sends message #%d of type %d to process %s[%d]\n", \
            YS__ActEvnt->name,YS__ActId(YS__ActEvnt), mptr->id, type, \
            mptr->receiver->name, YS__ActId(mptr->receiver) ); \
      else if (YS__ActProc != NULL) \
         sprintf(YS__prbpkt, \
            "    Process %s[%d] sends message #%d of type %d to process %s[%d]\n", \
            YS__ActProc->name,YS__ActId(YS__ActProc), mptr->id, type, \
            mptr->receiver->name, YS__ActId(mptr->receiver) ); \
      else \
         sprintf(YS__prbpkt, \
            "    Non-activity sends message #%d of type %d to process %s[%d]\n", \
            mptr->id, type, mptr->receiver->name, YS__ActId(mptr->receiver) ); \
      if (blkflg == BLOCK) sprintf(YS__prbpkt+strlen(YS__prbpkt), \
         "    - Blocking send, sending process suspends\n"); \
      else if (YS__ActEvnt == NULL && YS__ActProc != NULL) \
         sprintf(YS__prbpkt+strlen(YS__prbpkt), \
            "    - Non-blocking send, sending process continues\n"); \
      if (YS__ActEvnt != NULL) \
         YS__SendPrbPkt(TEXTPROBE,YS__ActEvnt->name,YS__prbpkt); \
      else if (YS__ActProc != NULL) \
         YS__SendPrbPkt(TEXTPROBE,YS__ActProc->name,YS__prbpkt); \
      else \
         YS__SendPrbPkt(TEXTPROBE,"User",YS__prbpkt); \
   }

#define TRACE_PROCESS_msgrecv1  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt, \
         "    Process %s[%d] tries to receive message of type %d " , \
         YS__ActProc->name,YS__ActId(YS__ActProc),typ); \
      if (sender) \
         sprintf(YS__prbpkt+strlen(YS__prbpkt), \
            "from process %s[%d]\n",sender->name,YS__ActId(sender)); \
      else \
         sprintf(YS__prbpkt+strlen(YS__prbpkt),"\n"); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc->name,YS__prbpkt); \
   }

#define TRACE_PROCESS_msgrecv2  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    - Message available\n"); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc->name,YS__prbpkt); \
   }

#define TRACE_PROCESS_msgrecv3  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    Blocked sending process %s[%d] released\n", \
              mptr->source->name,YS__ActId(mptr->source)); \
      YS__SendPrbPkt(TEXTPROBE,mptr->source->name,YS__prbpkt); \
   }

#define TRACE_PROCESS_msgrecv4   \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      if (mptr->source == NULL) \
         sprintf(YS__prbpkt,"    Receiving %d characters of message #%d of type %d\n", \
                bytestomove,mptr->id,mptr->msgtype); \
      else \
         sprintf(YS__prbpkt, \
         "    Receiving %d characters of message #%d of type %d from process %s[%d]\n", \
         bytestomove,mptr->id,mptr->msgtype,mptr->source->name,YS__ActId(mptr->source)); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc->name,YS__prbpkt); \
   }

#define TRACE_PROCESS_msgrecv5  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt, \
         "    - No messages available; non-blocking receive, process continues\n"); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc->name,YS__prbpkt); \
   }

#define TRACE_PROCESS_msgrecv6   \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt, \
         "    - No messages available; blocking receive, process suspends\n"); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc->name,YS__prbpkt); \
   }

#define TRACE_PROCESS_msgchk1  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt, \
         "    Process %s[%d] checks for message of type %d " , \
         YS__ActProc->name,YS__ActId(YS__ActProc),typ); \
      if (sender) \
         sprintf(YS__prbpkt+strlen(YS__prbpkt), \
            "from processor %d[%d]\n",sender,YS__ActId(sender)); \
      else \
         sprintf(YS__prbpkt+strlen(YS__prbpkt),"\n"); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc->name,YS__prbpkt); \
   }

#define TRACE_PROCESS_msgchk2  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    - Message available\n"); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc->name,YS__prbpkt); \
   }

#define TRACE_PROCESS_msgchk3  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    - No message available\n"); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc->name,YS__prbpkt); \
   }

/**************************************************************************************\
*                             EVENT Tracing Statements                                 *
\**************************************************************************************/

#define TRACE_EVENT_event  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    Creating event %s[%d]\n",eptr->name,YS__ActId(eptr)); \
      YS__SendPrbPkt(TEXTPROBE,eptr->name,YS__prbpkt); \
   }

#define TRACE_EVENT_reschedule1  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    Rescheduling event %s[%d] to occur in %g time units\n", \
              YS__ActEvnt->name,YS__ActId(YS__ActEvnt),timeinc); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActEvnt->name,YS__prbpkt); \
   }

#define TRACE_EVENT_reschedule2  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt, \
         "    Rescheduling event %s[%d] to wait for semaphore %s[%d]\n",  \
         YS__ActEvnt->name,YS__ActId(YS__ActEvnt),sptr->name,YS__ActId(sptr)); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActEvnt->name,YS__prbpkt); \
   }

#define TRACE_EVENT_reschedule3 \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt, \
         "    - Semaphore %s[%d] = %d; %s is decremented and %s is activated\n", \
         sptr->name,YS__ActId(sptr),sptr->val,sptr->name,YS__ActEvnt->name); \
      YS__SendPrbPkt(TEXTPROBE,sptr->name,YS__prbpkt); \
   }

#define TRACE_EVENT_reschedule4  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt,"    - Semaphore %s[%d] = %d; %s waits\n",  \
              sptr->name,YS__ActId(sptr),sptr->val,YS__ActEvnt->name); \
      YS__SendPrbPkt(TEXTPROBE,sptr->name,YS__prbpkt); \
   }

#define TRACE_EVENT_reschedule5 \
   if (TraceLevel >= MAXDBLEVEL-2) { \
     sprintf(YS__prbpkt,"    Rescheduling event %s[%d] to wait for flag %s[%d]\n", \
             YS__ActEvnt->name,YS__ActId(YS__ActEvnt),fptr->name,YS__QeId(fptr)); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActEvnt->name,YS__prbpkt); \
  }

#define TRACE_EVENT_reschedule6  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    - Flag ");  \
      if (fptr->status != PENDING) sprintf(YS__prbpkt+strlen(YS__prbpkt), \
         "notified\n");  \
      else sprintf(YS__prbpkt+strlen(YS__prbpkt),"already pending\n");  \
      YS__SendPrbPkt(TEXTPROBE,YS__ActEvnt->name,YS__prbpkt); \
   }

#define TRACE_EVENT_reschedule7  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
     sprintf(YS__prbpkt, \
        "    Scheduling event %s[%d] to wait for condition %s[%d]\n", \
        YS__ActEvnt->name,YS__ActId(YS__ActEvnt),cptr->name,YS__QeId(cptr)); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActEvnt->name,YS__prbpkt); \
  }

#define TRACE_EVENT_reschedule8  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    - Condition ");  \
      if (cptr->status != PENDING) sprintf(YS__prbpkt+strlen(YS__prbpkt), \
         "notified\n");  \
      else sprintf(YS__prbpkt+strlen(YS__prbpkt),"already pending\n");  \
      YS__SendPrbPkt(TEXTPROBE,YS__ActEvnt->name,YS__prbpkt); \
   }

#define TRACE_EVENT_reschedule9  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
     sprintf(YS__prbpkt, \
        "    Rescheduling event %s[%d] to use resource %s[%d] for %g time units\n", \
        YS__ActEvnt->name,YS__ActId(YS__ActEvnt),rptr->name,YS__QeId(rptr),timeinc); \
     YS__SendPrbPkt(TEXTPROBE,YS__ActEvnt->name,YS__prbpkt); \
  }

#define TRACE_EVENT_reschedule10  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    - Resource ");  \
      if (rptr->status != PENDING) sprintf(YS__prbpkt+strlen(YS__prbpkt), \
         "notified\n");  \
      else sprintf(YS__prbpkt+strlen(YS__prbpkt),"already pending\n");  \
      YS__SendPrbPkt(TEXTPROBE,YS__ActEvnt->name,YS__prbpkt); \
   }

#define TRACE_EVENT_reschedule11  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    Rescheduling event %s[%d]\n", \
              YS__ActEvnt->name,YS__ActId(YS__ActEvnt)); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActEvnt->name,YS__prbpkt); \
   }

#define TRACE_EVENT_settype  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    Setting type of event %s[%d] to %d\n", \
         eptr->name,YS__ActId(eptr),etype); \
      YS__SendPrbPkt(TEXTPROBE,eptr->name,YS__prbpkt); \
   }

#define TRACE_EVENT_setdelflg  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    Event %s[%d] set to deleting mode\n", \
         YS__ActEvnt->name,YS__ActId(YS__ActEvnt)); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActEvnt->name,YS__prbpkt); \
   }

#else  /*******************************************************************************/

#define TRACE_ACTIVITY_setarg
#define TRACE_ACTIVITY_getarg
#define TRACE_ACTIVITY_fork
#define TRACE_ACTIVITY_block
#define TRACE_ACTIVITY_schedule1
#define TRACE_ACTIVITY_schedule2
#define TRACE_ACTIVITY_schedule3
#define TRACE_ACTIVITY_schedule4
#define TRACE_ACTIVITY_schedule5
#define TRACE_ACTIVITY_schedule6
#define TRACE_ACTIVITY_schedule7
#define TRACE_ACTIVITY_schedule8
#define TRACE_ACTIVITY_schedule9
#define TRACE_ACTIVITY_schedule11
#define TRACE_ACTIVITY_schedule12

#define TRACE_PROCESS_process
#define TRACE_PROCESS_transfer
#define TRACE_PROCESS_suspend1
#define TRACE_PROCESS_suspend2
#define TRACE_PROCESS_suspend3
#define TRACE_PROCESS_join1
#define TRACE_PROCESS_join2
#define TRACE_PROCESS_join3
#define TRACE_PROCESS_delay
#define TRACE_PROCESS_nice
#define TRACE_PROCESS_waitcpu
#define TRACE_PROCESS_sendmsg1
#define TRACE_PROCESS_msgrecv1
#define TRACE_PROCESS_msgrecv2
#define TRACE_PROCESS_msgrecv3
#define TRACE_PROCESS_msgrecv4
#define TRACE_PROCESS_msgrecv5
#define TRACE_PROCESS_msgrecv6
#define TRACE_PROCESS_msgrecv2
#define TRACE_PROCESS_msgchk1
#define TRACE_PROCESS_msgchk2
#define TRACE_PROCESS_msgchk3

#define TRACE_EVENT_event
#define TRACE_EVENT_reschedule1
#define TRACE_EVENT_reschedule2
#define TRACE_EVENT_reschedule3
#define TRACE_EVENT_reschedule4
#define TRACE_EVENT_reschedule5
#define TRACE_EVENT_reschedule6
#define TRACE_EVENT_reschedule7
#define TRACE_EVENT_reschedule8
#define TRACE_EVENT_reschedule9
#define TRACE_EVENT_reschedule10
#define TRACE_EVENT_reschedule11
#define TRACE_EVENT_settype
#define TRACE_EVENT_setdelflg

#endif  /******************************************************************************/

#endif

