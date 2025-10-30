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

#ifndef BQUEUESTRH
#define BQUEUESTRH

#ifdef debug  /************************************************************************\
*                                 POOL tracing statements                              *
\**************************************************************************************/

#define TRACE_POOL_getobj1  \
   if (TraceLevel >= MAXDBLEVEL) { \
      sprintf(YS__prbpkt,"        Getting object from the %s\n",pptr->name);  \
      YS__SendPrbPkt(TEXTPROBE,pptr->name,YS__prbpkt); \
   }

#define TRACE_POOL_getobj2  \
   if (TraceLevel >= MAXDBLEVEL) { \
      sprintf(YS__prbpkt,"        - %s gets new block from system\n",pptr->name);  \
      YS__SendPrbPkt(TEXTPROBE,pptr->name,YS__prbpkt); \
   }

#define TRACE_POOL_retobj  \
   if (TraceLevel >= MAXDBLEVEL) { \
      sprintf(YS__prbpkt,"        Returning object to %s\n",pptr->name);  \
      YS__SendPrbPkt(TEXTPROBE,pptr->name,YS__prbpkt); \
   }

/**************************************************************************************\
*                                QUEUE tracing statements                              *
\**************************************************************************************/

#define TRACE_QUEUE_gethead1  \
   if (TraceLevel >= MAXDBLEVEL-1) { \
      sprintf(YS__prbpkt,"        Getting %s[%d] from head of queue %s[%d]\n",  \
              (qeptr)->name,YS__QeId(qptr),qptr->name,YS__QeId(qptr)); \
      YS__SendPrbPkt(TEXTPROBE,qptr->name,YS__prbpkt); \
   }

#define TRACE_QUEUE_gethead2  \
   if (TraceLevel >= MAXDBLEVEL-1) { \
      sprintf(YS__prbpkt, \
         "        Queue %s[%d] empty\n",qptr->name,YS__QeId(qptr)); \
      YS__SendPrbPkt(TEXTPROBE,qptr->name,YS__prbpkt); \
   }

#define TRACE_QUEUE_gettail1  \
   if (TraceLevel >= MAXDBLEVEL-1) { \
      sprintf(YS__prbpkt,"        Getting %s[%d] from tail of queue %s[%d]\n",  \
              qeptr->name,YS__QeId(qeptr),qptr->name,YS__QeId(qptr)); \
      YS__SendPrbPkt(TEXTPROBE,qptr->name,YS__prbpkt); \
   }

#define TRACE_QUEUE_gettail2  \
   if (TraceLevel >= MAXDBLEVEL-1) { \
      sprintf(YS__prbpkt, \
         "        Queue %s[%d] empty\n",qptr->name,YS__QeId(qptr)); \
      YS__SendPrbPkt(TEXTPROBE,qptr->name,YS__prbpkt); \
   }

#define TRACE_QUEUE_puttail  \
   if (TraceLevel >= MAXDBLEVEL-1) { \
      sprintf(YS__prbpkt,"        Putting %s[%d] on the tail of queue %s[%d]\n",  \
              qeptr->name,YS__QeId(qeptr),qptr->name,YS__QeId(qptr)); \
      YS__SendPrbPkt(TEXTPROBE,qptr->name,YS__prbpkt); \
   }

#define TRACE_QUEUE_puthead  \
   if (TraceLevel >= MAXDBLEVEL-1) { \
      sprintf(YS__prbpkt,"        Putting %s[%d] at the head of queue %s[%d]\n", \
              qeptr->name,YS__QeId(qeptr),qptr->name,YS__QeId(qptr)); \
      YS__SendPrbPkt(TEXTPROBE,qptr->name,YS__prbpkt); \
   }

#define TRACE_QUEUE_takeout  \
   if (TraceLevel >= MAXDBLEVEL-1) { \
      sprintf(YS__prbpkt,"        Taking element %s[%d] from queue %s[%d]\n",  \
              qeptr->name,YS__QeId(qeptr),qptr->name,YS__QeId(qptr)); \
      YS__SendPrbPkt(TEXTPROBE,qptr->name,YS__prbpkt); \
   }

#define TRACE_QUEUE_checkelem1  \
   if (TraceLevel >= MAXDBLEVEL-1) { \
      sprintf(YS__prbpkt,"        Element %s[%d] in %s[%d]\n",  \
           qeptr->name,YS__QeId(qeptr),qptr->name,YS__QeId(qptr)); \
      YS__SendPrbPkt(TEXTPROBE,qptr->name,YS__prbpkt); \
   }

#define TRACE_QUEUE_checkelem2  \
   if (TraceLevel >= MAXDBLEVEL-1) { \
      sprintf(YS__prbpkt,"        Element %s[%d] not in %s[%d]\n",  \
           qeptr->name,YS__QeId(qeptr),qptr->name,YS__QeId(qptr)); \
      YS__SendPrbPkt(TEXTPROBE,qptr->name,YS__prbpkt); \
   }

#define TRACE_QUEUE_next  \
   if (TraceLevel >= MAXDBLEVEL-1)  \
   {  \
      if (qeptr) {  \
         sprintf(YS__prbpkt,"        The element after %s[%d] in queue %s[%d] is "  \
                 ,qeptr->name,YS__QeId(qeptr),qptr->name,YS__QeId(qptr));  \
         if (eptr == NULL) sprintf(YS__prbpkt+strlen(YS__prbpkt),"NULL\n");  \
         else sprintf(YS__prbpkt+strlen(YS__prbpkt), \
                  "%s[%d]\n",eptr->name,YS__QeId(eptr));  \
      }  \
      else {  \
         sprintf(YS__prbpkt,"        The element at the head of queue %s[%d] is ",  \
                 qptr->name,YS__QeId(qptr));  \
         if (eptr == NULL) sprintf(YS__prbpkt+strlen(YS__prbpkt),"NULL\n");  \
         else sprintf(YS__prbpkt+strlen(YS__prbpkt), \
                 "%s[%d]\n",eptr->name,YS__QeId(eptr));  \
      }  \
      YS__SendPrbPkt(TEXTPROBE,qptr->name,YS__prbpkt); \
   }

#define TRACE_QUEUE_prev  \
   if (TraceLevel >= MAXDBLEVEL-1)  \
   {  \
      if (qeptr) {  \
         sprintf(YS__prbpkt,"        The element before %s[%d] in queue %s[%d] is "  \
                 ,qeptr->name,YS__QeId(qeptr),qptr->name,YS__QeId(qptr));  \
         if (eptr == NULL) sprintf(YS__prbpkt+strlen(YS__prbpkt),"NULL\n");  \
         else sprintf(YS__prbpkt+strlen(YS__prbpkt), \
                  "%s[%d]\n",eptr->name,YS__QeId(eptr));  \
      }  \
      else {  \
         sprintf(YS__prbpkt,"        The element at the tail of queue %s[%d] is ",  \
                 qptr->name,YS__QeId(qptr));  \
         if (eptr == NULL) sprintf(YS__prbpkt+strlen(YS__prbpkt),"NULL\n");  \
         else sprintf(YS__prbpkt+strlen(YS__prbpkt), \
                 "%s[%d]\n",eptr->name,YS__QeId(eptr));  \
      }  \
      YS__SendPrbPkt(TEXTPROBE,qptr->name,YS__prbpkt); \
   }

#define TRACE_QUEUE_show(ptr)  \
   if (TraceLevel >= MAXDBLEVEL-1) YS__QueuePrint(ptr); 

#define TRACE_QUEUE_headvalue  \
   if (TraceLevel >= MAXDBLEVEL-1) { \
      sprintf(YS__prbpkt,"        Value of head of queue %s[%d] = %g\n",  \
              qptr->name,YS__QeId(qptr),retval); \
      YS__SendPrbPkt(TEXTPROBE,qptr->name,YS__prbpkt); \
   }

#define TRACE_QUEUE_insert  \
   if (TraceLevel >= MAXDBLEVEL-1) { \
      sprintf(YS__prbpkt,"        Inserting %s[%d] with time %g into queue %s[%d]\n", \
              aptr->name,YS__QeId(aptr),aptr->time,qptr->name,YS__QeId(qptr)); \
      YS__SendPrbPkt(TEXTPROBE,qptr->name,YS__prbpkt); \
   }

#define TRACE_QUEUE_enter  \
   if (TraceLevel >= MAXDBLEVEL-1) { \
      sprintf(YS__prbpkt,"        Entering %s[%d] with time %g into queue %s[%d]\n", \
              aptr->name,YS__QeId(aptr),aptr->time,qptr->name,YS__QeId(qptr)); \
      YS__SendPrbPkt(TEXTPROBE,qptr->name,YS__prbpkt); \
   }

#define TRACE_QUEUE_prinsert  \
   if (TraceLevel >= MAXDBLEVEL-1) { \
      sprintf(YS__prbpkt, \
         "        Inserting %s[%d] with resource priority %g into queue %s[%d]\n",\
              aptr->name,YS__QeId(aptr), \
              aptr->priority, \
              qptr->name,YS__QeId(qptr)); \
      YS__SendPrbPkt(TEXTPROBE,qptr->name,YS__prbpkt); \
   }

#define TRACE_QUEUE_prenter  \
   if (TraceLevel >= MAXDBLEVEL-1) { \
      sprintf(YS__prbpkt, \
         "        Entering %s[%d] with resource priority %g into queue %s[%d]\n", \
              aptr->name,YS__QeId(aptr), \
              aptr->priority, \
              qptr->name,YS__QeId(qptr)); \
      YS__SendPrbPkt(TEXTPROBE,qptr->name,YS__prbpkt); \
   }

#define TRACE_QUEUE_reset  \
   if (TraceLevel >= MAXDBLEVEL-1) { \
      sprintf(YS__prbpkt, \
         "        Clearing queue %s[%d]\n",qptr->name,YS__QeId(qptr)); \
      YS__SendPrbPkt(TEXTPROBE,qptr->name,YS__prbpkt); \
   }

#define TRACE_PRQUEUE_new  \
   if (TraceLevel >= MAXDBLEVEL-1)  { \
      sprintf(YS__prbpkt, \
         "        Creating queue %s[%d]\n",prqptr->name,YS__QeId(prqptr)); \
      YS__SendPrbPkt(TEXTPROBE,prqptr->name,YS__prbpkt); \
   }

#else  /*******************************************************************************/

#define TRACE_POOL_getobj1
#define TRACE_POOL_getobj2
#define TRACE_POOL_retobj
#define TRACE_QUEUE_gethead1
#define TRACE_QUEUE_gethead2
#define TRACE_QUEUE_gettail1
#define TRACE_QUEUE_gettail2
#define TRACE_QUEUE_puttail
#define TRACE_QUEUE_puthead
#define TRACE_QUEUE_takeout
#define TRACE_QUEUE_checkelem1
#define TRACE_QUEUE_checkelem2
#define TRACE_QUEUE_next
#define TRACE_QUEUE_prev
#define TRACE_QUEUE_show(ptr)
#define TRACE_QUEUE_headvalue
#define TRACE_QUEUE_insert
#define TRACE_QUEUE_enter
#define TRACE_QUEUE_prinsert
#define TRACE_QUEUE_prenter
#define TRACE_QUEUE_reset
#define TRACE_PRQUEUE_new

#endif  /******************************************************************************/

#endif
