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

#ifndef DRIVERTRH
#define DRIVERTRH

#ifdef debug

/**************************************************************************************\
*                             DRIVER tracing statements                                *
\**************************************************************************************/

#define TRACE_DRIVER_run  \
   if (TraceLevel >= MAXDBLEVEL-3) { \
      sprintf(YS__prbpkt,"User activating Simulation Driver\n"); \
      YS__SendPrbPkt(TEXTPROBE,"Driver",YS__prbpkt); \
   }

#define TRACE_DRIVER_reset  \
    if (TraceLevel >= MAXDBLEVEL-3)  { \
       sprintf(YS__prbpkt,"\nDRIVER RESET -------------------------------------------");\
       sprintf(YS__prbpkt+strlen(YS__prbpkt),"<<Simulation Time: %g>>\n\n", \
               YS__Simtime);  \
      YS__SendPrbPkt(TEXTPROBE,"times",YS__prbpkt); \
    }

#define TRACE_DRIVER_body1  \
   if (TraceLevel >= MAXDBLEVEL-3)  { \
      sprintf(YS__prbpkt,"Transferring to process %s[%d]\n", \
         actptr->name,YS__ActId(actptr));  \
      YS__SendPrbPkt(TEXTPROBE,actptr->name,YS__prbpkt); \
   }

#define TRACE_DRIVER_evterminate  \
   if (TraceLevel >= MAXDBLEVEL-3)  { \
      sprintf(YS__prbpkt,"Event %s[%d] terminating\n", \
         YS__ActEvnt->name,YS__ActId(YS__ActEvnt)); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActEvnt->name,YS__prbpkt); \
   }

#define TRACE_DRIVER_evdelete  \
   if (TraceLevel >= MAXDBLEVEL-1)  { \
      sprintf(YS__prbpkt,"        Deleting process %s[%d]\n", \
         actptr->name,YS__ActId(YS__ActEvnt)); \
      YS__SendPrbPkt(TEXTPROBE,"Driver",YS__prbpkt); \
   }

#define TRACE_DRIVER_terminate  \
   if (TraceLevel >= MAXDBLEVEL-3 && actptr->id != 0)  { \
      sprintf(YS__prbpkt,"Process %s[%d] terminating\n", \
         actptr->name,YS__ActId(actptr)); \
      YS__SendPrbPkt(TEXTPROBE,actptr->name,YS__prbpkt); \
   }

#define TRACE_DRIVER_prdelete  \
   if (TraceLevel >= MAXDBLEVEL-1)  { \
      sprintf(YS__prbpkt,"        Deleting process %s[%d]\n", \
         actptr->name,YS__ActId(actptr)); \
      YS__SendPrbPkt(TEXTPROBE,"Driver",YS__prbpkt); \
   }

#define TRACE_DRIVER_join1  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt, \
         "    All child activities complete, process %s[%d] released from Join\n", \
         parptr->name,YS__ActId(parptr)); \
      YS__SendPrbPkt(TEXTPROBE,parptr->name,YS__prbpkt); \
   }

#define TRACE_DRIVER_interrupt  \
   if (TraceLevel >= MAXDBLEVEL-3)  { \
      sprintf(YS__prbpkt,"Driver interrupt occurs, returning to user\n"); \
      YS__SendPrbPkt(TEXTPROBE,"Driver",YS__prbpkt); \
   }

#define TRACE_DRIVER_interrupt1  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    Driver interrupt flag set\n"); \
      YS__SendPrbPkt(TEXTPROBE,"Driver",YS__prbpkt); \
   }

#define TRACE_DRIVER_empty  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    ------ Ready list empty, "); \
      sprintf(YS__prbpkt+strlen(YS__prbpkt), \
         "checking for pending resources\n");  \
      YS__SendPrbPkt(TEXTPROBE,"Driver",YS__prbpkt); \
   }

#define TRACE_DRIVER_body3  \
   if (TraceLevel >= MAXDBLEVEL-3)  { \
      sprintf(YS__prbpkt,"\nTIME STEP COMPLETED: Time = %g\n\n",YS__Simtime); \
      YS__SendPrbPkt(TEXTPROBE,"Driver",YS__prbpkt); \
   }

#define TRACE_DRIVER_body4  \
   if (TraceLevel >= MAXDBLEVEL-4) { \
      sprintf(YS__prbpkt,"\nREADY & EVENT LISTS EMPTY: Time = %g\n\n",YS__Simtime); \
      YS__SendPrbPkt(TEXTPROBE,"Driver",YS__prbpkt); \
   }

#define TRACE_DRIVER_simtime  \
    if (TraceLevel >= MAXDBLEVEL-3)  { \
       sprintf(YS__prbpkt,"--------------------------------------------------------"); \
       sprintf(YS__prbpkt+strlen(YS__prbpkt),"<<Simulation Time: %g>>\n", \
               YS__Simtime);  \
      YS__SendPrbPkt(TEXTPROBE,"times",YS__prbpkt); \
    }

#define TRACE_DRIVER_body2  \
   if (TraceLevel >= MAXDBLEVEL-3)  { \
      sprintf(YS__prbpkt,"Initiating event %s[%d]\n",  \
              YS__ActEvnt->name,YS__ActId(YS__ActEvnt)); \
      YS__SendPrbPkt(TEXTPROBE,"Driver",YS__prbpkt); \
   }

#define TRACE_DRIVER_body6  \
   if (TraceLevel >= MAXDBLEVEL-1) { \
      sprintf(YS__prbpkt,"        Deleting event %s[%d]\n",  \
              YS__ActEvnt->name,YS__ActId(YS__ActEvnt)); \
      YS__SendPrbPkt(TEXTPROBE,"Driver",YS__prbpkt); \
   }

#define TRACE_DRIVER_join2  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
     sprintf(YS__prbpkt, \
        "    All child activities complete, process %s[%d] released from Join\n", \
        YS__ActEvnt->parent->name,YS__ActId(YS__ActEvnt->parent)); \
     YS__SendPrbPkt(TEXTPROBE,YS__ActEvnt->parent->name,YS__prbpkt); \
   }

#define TRACE_DRIVER_condrelease  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt,"    - Activity %s[%d] released from condition %s[%d]\n", \
              actptr->name,YS__ActId(actptr),conptr1->name,YS__ActId(conptr1)); \
      YS__SendPrbPkt(TEXTPROBE,actptr->name,YS__prbpkt); \
   }

#define TRACE_DRIVER_evalcond   \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    Evaluating condition %s[%d]; new state = %d\n", \
              conptr1->name,YS__QeId(conptr1),conptr1->state); \
      YS__SendPrbPkt(TEXTPROBE,"Driver",YS__prbpkt); \
   }



#else

/**************************************************************************************/

#define TRACE_DRIVER_run
#define TRACE_DRIVER_reset
#define TRACE_DRIVER_body1
#define TRACE_DRIVER_terminate
#define TRACE_DRIVER_prdelete
#define TRACE_DRIVER_join1
#define TRACE_DRIVER_interrupt
#define TRACE_DRIVER_interrupt1
#define TRACE_DRIVER_empty
#define TRACE_DRIVER_body3
#define TRACE_DRIVER_body4
#define TRACE_DRIVER_simtime
#define TRACE_DRIVER_body6
#define TRACE_DRIVER_body2
#define TRACE_DRIVER_join2
#define TRACE_DRIVER_flgrelease
#define TRACE_DRIVER_condrelease
#define TRACE_DRIVER_evalcond
#endif

/**************************************************************************************/


#endif
