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

#ifndef MSGTRH
#define MSGTRH

#ifdef debug  /************************************************************************\
*                            MESSAGE tracing statements                                *
\**************************************************************************************/

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
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc->name,YS__prbpkt); \
   }

#define TRACE_PROCESS_sendmsg2  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt, \
         "    Process %s[%d] sends message #%d of type %d to processor %d\n", \
         YS__ActProc->name,YS__ActId(YS__ActProc), mptr->id, type, mptr->receiver); \
      if (blkflg == BLOCK) sprintf(YS__prbpkt+strlen(YS__prbpkt), \
         "    - Blocking send, sending process suspends\n"); \
      else \
         sprintf(YS__prbpkt+strlen(YS__prbpkt), \
            "    - Non-blocking send, sending process continues\n"); \
      YS__SendPrbPkt(TEXTPROBE,YS__ActProc->name,YS__prbpkt); \
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


#else  /*******************************************************************************/


#endif  /******************************************************************************/

#endif
