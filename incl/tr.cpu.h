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

#ifndef MODULESTRH
#define MODULESTRH

#ifdef debug  /************************************************************************\
*                           PROCESSOR tracing statements                               *
\**************************************************************************************/

#define TRACE_PROCESSOR_processor  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    Creating processor %s[%d]\n",prptr->name,ProcessorId(prptr)); \
      YS__SendPrbPkt(TEXTPROBE,prptr->name,YS__prbpkt); \
   }

#define TRACE_PROCESSOR_attach  \
   if (TraceLevel >= MAXDBLEVEL-2)  { \
      sprintf(YS__prbpkt,"    Attaching activity %s[%d] to processor %s[%d]\n", \
              aptr->name,YS__ActId(aptr),pptr->name,ProcessorId(pptr)); \
      YS__SendPrbPkt(TEXTPROBE,pptr->name,YS__prbpkt); \
   }

#else  /*******************************************************************************/

#define TRACE_PROCESSOR_processor
#define TRACE_PROCESSOR_attach

#endif  /******************************************************************************/

#endif

