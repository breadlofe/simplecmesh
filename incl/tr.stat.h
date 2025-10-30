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

#ifndef STATTRH
#define STATTRH

#ifdef debug

/**************************************************************************************\
*                             STATREC tracing statements                               *
\**************************************************************************************/

#define TRACE_STATREC_new1  \
   if (TraceLevel >= MAXDBLEVEL-2) {  \
      sprintf(YS__prbpkt,"    Creating statistics record %s[%d]\n",  \
              srptr->name,YS__StatrecId(srptr));  \
      if (meanflg)  \
         sprintf(YS__prbpkt+strlen(YS__prbpkt), \
                 "    - Means computed; no histograms computed\n");  \
      else \
         sprintf(YS__prbpkt+strlen(YS__prbpkt), \
                 "    - No Means and no histograms computed\n");  \
      YS__SendPrbPkt(TEXTPROBE,srptr->name,YS__prbpkt); \
   }

#define TRACE_STATREC_new2  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    Creating statistics record %s[%d]\n", \
              srptr->name,YS__StatrecId(srptr));  \
      if (meanflg)  \
         sprintf(YS__prbpkt+strlen(YS__prbpkt), \
                 "    - Means and histograms computed\n");  \
      else \
         sprintf(YS__prbpkt+strlen(YS__prbpkt), \
                 "    - Histograms computed; no means computed\n");  \
      sprintf(YS__prbpkt+strlen(YS__prbpkt), \
              "    - %d bins, bin size = %g, low bin < %g, high bin >= %g\n",  \
              srptr->bins,BININC,LOWBIN,HIGHBIN);  \
      YS__SendPrbPkt(TEXTPROBE,srptr->name,YS__prbpkt); \
   }

#define TRACE_STATREC_sethistsz  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    Setting default histogram pool size to %d\n",i); \
      YS__SendPrbPkt(TEXTPROBE,NULL,YS__prbpkt); \
   }

#define TRACE_STATREC_reset  \
   if (TraceLevel >= MAXDBLEVEL-2) { \
      sprintf(YS__prbpkt,"    Resetting statistics record %s[%d]\n",  \
              srptr->name,YS__StatrecId(srptr)); \
      YS__SendPrbPkt(TEXTPROBE,srptr->name,YS__prbpkt); \
   }

#define TRACE_STATREC_pupdate  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt, \
         "    Updating point statistics record %s[%d]; value = %g, weight = %g\n", \
         srptr->name,YS__StatrecId(srptr),v,t); \
      YS__SendPrbPkt(TEXTPROBE,srptr->name,YS__prbpkt); \
   }

#define TRACE_STATREC_iupdate  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt, \
         "    Updating interval statistics record %s[%d]; value = %g, interval = %g\n", \
         srptr->name,YS__StatrecId(srptr),v,srptr->interval); \
      YS__SendPrbPkt(TEXTPROBE,srptr->name,YS__prbpkt); \
   }
 
/**************************************************************************************\
*                    Trace statements for external histograms                          *
\**************************************************************************************/

#define TRACE_STATREC_create  \
   if (YS__interactive)  { \
      sprintf(YS__prbpkt, "%c%g:%g:%d", HISTCREATE, BININC, LOWBIN, srptr->bins - 2); \
      YS__SendPrbPkt(HISTPROBE,srptr->name,YS__prbpkt); \
   }

#define TRACE_STATREC_isample  \
   if (YS__interactive)  { \
      sprintf(YS__prbpkt, "%c%g:%g", HISTSAMPLE, srptr->lastv, srptr->interval); \
      YS__SendPrbPkt(HISTPROBE,srptr->name,YS__prbpkt); \
   }

#define TRACE_STATREC_psample  \
   if (YS__interactive)  { \
      sprintf(YS__prbpkt, "%c%g:%g", HISTSAMPLE, v, t); \
      YS__SendPrbPkt(HISTPROBE,srptr->name,YS__prbpkt); \
   }

#define TRACE_STATREC_clear  \
   if (YS__interactive)  { \
      sprintf(YS__prbpkt, "%c", HISTRESET); \
      YS__SendPrbPkt(HISTPROBE,srptr->name,YS__prbpkt); \
   }


#else /********************************************************************************/

#define TRACE_STATREC_new1
#define TRACE_STATREC_new2
#define TRACE_STATREC_sethistsz
#define TRACE_STATREC_reset
#define TRACE_STATREC_pupdate
#define TRACE_STATREC_iupdate

#define TRACE_STATREC_create
#define TRACE_STATREC_isample
#define TRACE_STATREC_psample
#define TRACE_STATREC_clear

#endif /*******************************************************************************/

/**************************************************************************************/


#endif
