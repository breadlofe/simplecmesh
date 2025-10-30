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

#ifndef EVLSTTRH
#define EVLSTTRH

#ifdef debug  /************************************************************************\
*                           EVENT LIST tracing statements                              *
\**************************************************************************************/

#define TRACE_EVLST_gethead1  \
   if (TraceLevel >= MAXDBLEVEL) { \
      sprintf(YS__prbpkt, \
         "        Event List empty\n"); \
      YS__SendPrbPkt(TEXTPROBE,"EventList",YS__prbpkt); \
   }

#define TRACE_EVLST_gethead2  \
   if (TraceLevel >= MAXDBLEVEL) { \
      sprintf(YS__prbpkt,"        Getting %s[%d] from the head of the Event List\n",  \
              retptr->name,YS__QeId(retptr)); \
      YS__SendPrbPkt(TEXTPROBE,"EventList",YS__prbpkt); \
   }

#define TRACE_EVLST_headval  \
   if (TraceLevel >= MAXDBLEVEL) { \
      sprintf(YS__prbpkt,"        Value of the Event List head = %g\n",  \
              retval); \
      YS__SendPrbPkt(TEXTPROBE,"EventList",YS__prbpkt); \
   }

#define TRACE_EVLST_delete  \
   if (TraceLevel >= MAXDBLEVEL) { \
      sprintf(YS__prbpkt,"        Deleting element %s[%d] from the Event List\n", \
              aptr->name,YS__QeId(aptr)); \
      YS__SendPrbPkt(TEXTPROBE,"EventList",YS__prbpkt); \
   }

#define TRACE_EVLST_show  \
   if (TraceLevel >= MAXDBLEVEL)  \
      YS__EventListPrint();

#define TRACE_EVLST_init1  \
   if (TraceLevel >= MAXDBLEVEL) { \
      sprintf(YS__prbpkt,"Using calendar queue implementation for the event list\n"); \
      YS__SendPrbPkt(TEXTPROBE,"EventList",YS__prbpkt); \
   }

#define TRACE_EVLST_init2  \
   if (TraceLevel >= MAXDBLEVEL) { \
      sprintf(YS__prbpkt, \
         "Using simple linear search implementation for the event list\n"); \
      YS__SendPrbPkt(TEXTPROBE,"EventList",YS__prbpkt); \
   }

#define TRACE_EVLST_insert  \
   if (TraceLevel >= MAXDBLEVEL) { \
      sprintf(YS__prbpkt,"        Inserting %s[%d] with time %g into the Event List\n", \
              aptr->name,YS__QeId(aptr),aptr->time); \
      YS__SendPrbPkt(TEXTPROBE,"EventList",YS__prbpkt); \
   }


#else  /*******************************************************************************/


#endif  /******************************************************************************/


#endif


