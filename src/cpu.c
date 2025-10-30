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
#include "../incl/tr.cpu.h"

extern void YS__PacketSender();
extern void YS__PacketReceiver();

/**************************************************************************************\
*                              PROCESSOR Operations                                    *
\**************************************************************************************/

PROCESSOR *NewProcessor(pname, id, slice, noiports, nooports, routingfn)
                                /* Creates a new processor                            */
char *pname;                    /* User assigned name                                 */
int id;                         /* user assigned id for the processor                 */
double slice;                   /* Time slice for multiprogramming                    */
int noiports;                   /* Number of network input ports from this processor  */
int nooports;                   /* Number of network output ports to this processor   */
rtfunc routingfn;               /* Packet routing function; only used if noiports > 1 */
{
   PROCESSOR *prptr;
   int i;

   prptr = (PROCESSOR*)YS__PoolGetObj(&YS__PrcrPool);
   strncpy(prptr->name,pname,31);
   prptr->name[31] = '\0';
   prptr->id = id;
   prptr->next = NULL;
   prptr->status = IDLE;
   if (slice > 0.0) 
      prptr->RdyList = NewResource(pname,RRPRWP,1,slice);
   else 
      prptr->RdyList = NewResource(pname,RRPRWP,1,1.0e24);
   prptr->RdyList->qdisc = RDYLIST;
   if (noiports < 0 || noiports > MAXFANOUT || nooports < 0 || nooports > MAXFANOUT)
      YS__errmsg("Number of processor ports exceeds maximum allowed");
   prptr->noiports = noiports;
   prptr->nooports = nooports;
   if (noiports > 1)  prptr->router = routingfn;
   else               prptr->router = NULL;

   prptr->localmsgs = YS__NewQueue(pname);
   prptr->localsender = NewOSEvent("LocalSender",YS__PacketSender,NODELETE,0);
   prptr->localsender->pp = prptr;
   ActivitySetArg(prptr->localsender,NULL,-1);
   for (i=0; i<noiports; i++) {
      prptr->iports[i] = NULL;
      prptr->iportmsg[i] = YS__NewQueue(pname);
      prptr->iportevt[i] = NewOSEvent("NetSender",YS__PacketSender,NODELETE,0);
      prptr->iportevt[i]->pp = prptr;
      ActivitySetArg(prptr->iportevt[i],NULL,i);
   }

   for (i=0; i<nooports; i++) {
      prptr->oports[i] = NULL;
   }
   
   prptr->MsgList = YS__NewQueue ("ProcessorMsgList");
   prptr->WaitingProcesses = YS__NewQueue ("ProcessorWaitingProcesses");
   
   TRACE_PROCESSOR_processor;
   YS__TotalPrcrs++;
   YS__QueuePutTail(YS__ActPrcr,(QE*)prptr);
   return prptr;
}

/**************************************************************************************/

void ProcessorConnectIPort(pptr, index, iport) /* Connect a net iport to a processor  */
PROCESSOR *pptr;                               /* Pointer to the processor            */
int index;                                     /* Port position on the processor      */
IPORT *iport;                                  /* Port to connect                     */
{
   if (index >= 0 && index < pptr->noiports)   /* Valid port position                 */
      pptr->iports[index] = iport;
   else 
      YS__errmsg("Incorrect processor port connection index");
}

/**************************************************************************************/

void ProcessorConnectOPort(pptr, index, oport) /* Connect a net oport to a processor  */
PROCESSOR *pptr;                               /* Pointer to the processor            */
int index;                                     /* Port position on the processor      */
OPORT *oport;                                  /* Port to connect                     */
{
   EVENT *eptr;

   if (index >= 0 && index < pptr->nooports) {  /* Valid port position                */
      pptr->oports[index] = oport;
      oport->pr = pptr;
      eptr = NewOSEvent("NetReceiver",YS__PacketReceiver,NODELETE,0);
      eptr->pp = pptr;
      ActivitySetArg(eptr,NULL,index);
      ActivitySchedSema(eptr,oport->pktavail,INDEPENDENT);
   }
   else 
      YS__errmsg("Incorrect processor port connection index");
}

/**************************************************************************************/

void ProcessorAttach(pptr,aptr)  /* Attaches a process to a processor                 */
PROCESSOR *pptr;                 /* Pointer to the processor module                   */
ACTIVITY *aptr;                  /* Pointer to the activity                           */
{
   YS__PSDelay();

   TRACE_PROCESSOR_attach;
   aptr->pp = pptr;
}


/**************************************************************************************/

int ProcessorId(pptr)          /* Returns the system define Id or 0 if TrID is 0   */
PROCESSOR *pptr;               /* Pointer to the module                               */
{
   if (TraceIDs) {
      if (pptr) return pptr->id;
      else YS__errmsg("Null processor pointer referenced");
   }
   else return 0;
}

/**************************************************************************************/

void ProcessorUtilRept(type)   /* Prints out the processor util statistics            */
int type;                      /* GLOBAL, LOCAL, or BOTH                              */
{
   int i,j;
   double total = 0.0;
   STATREC *srptr;
   PROCESSOR *pp;
   
   printf ("\nPROCESSOR UTILIZATION REPORT:\n\n");
   if (type == GLOBAL || type == BOTH) {  /* Print global statistics                  */
      srptr = YS__BusyPrcrStat;
      StatrecUpdate(srptr,0.0,YS__Simtime);
      if (srptr) {             /* Global processor utilization stats collected        */

         printf("     Number of processors = %d;", YS__TotalPrcrs);
         printf(" Time interval = %g\n",StatrecInterval(srptr));
         printf("     Utilization = %g",StatrecMean(srptr));
         printf(" (standard deviation = %g)\n",StatrecSdv(srptr));
         printf("     Efficiency = %6.2f%s\n\n",
                (StatrecMean(srptr)/YS__TotalPrcrs)*100,"%");

         if (srptr->hist) {    /* Histogram constructed, print it out                 */
            for (i=0; i<=YS__TotalPrcrs; i++) total += srptr->hist[i];
            if (total != 0.0) {
               printf("     # of Busy    %s of\n","%");
               printf("     Processors   time\n");
               printf("     ----------   ----\n");
               for (i=0; i<=YS__TotalPrcrs; i++)  {
                  printf("         %d      %6.2f%s |",
                         i, (srptr->hist[i]/total)*100,"%");
                  for (j=1; j<(int)((srptr->hist[i]/total)*50); j++) printf("%s","*");
                  printf(" (%g)\n",srptr->hist[i]);
               }
            }
            printf("\n");
         }
      }
      else YS__warnmsg("Processor utiliation not computed");
   }

   if (type == LOCAL || type == BOTH) {  /* Print individual processor utilization    */
      pp = (PROCESSOR*)YS__QueueNext(YS__ActPrcr,NULL);
      while (pp != NULL) {               /* For each processor                        */
         srptr = QueueStatPtr(pp->RdyList,UTIL);
         if (srptr) {                    /* Stats collected for this processor        */
            printf("     Processor %s (id %d) was busy %6.2f%s of the time",
                    pp->name,pp->id, StatrecMean(srptr)*100,"%");
            printf(" (%g time units)\n",
               StatrecMean(srptr)*StatrecInterval(YS__BusyPrcrStat));
         }
         pp = (PROCESSOR*)YS__QueueNext(YS__ActPrcr,pp);
      }
   }
}

/**************************************************************************************/

void ProcessorGlobalUtil(hist)   /* Stats collected on the # of busy processors       */
int hist;                        /* HIST to construct histogram, NOHIST otherwise     */
{
   if (hist == HIST)             /* Histograms constructed                            */
      YS__BusyPrcrStat = NewStatrec("Processor Utilization", INTERVAL, MEANS, HIST,
                                      YS__TotalPrcrs-1, 1.0, (double)YS__TotalPrcrs);
   else                          /* Histograms not constructed                        */
      YS__BusyPrcrStat = NewStatrec("Processor Utilization", INTERVAL, MEANS, NOHIST,
                                      0, 0.0, 0.0);
}

/**************************************************************************************/

void ProcessorLocalUtil(pptr)    /* Stats collected on status BUSY or IDLE of procr   */
PROCESSOR *pptr;                 /* Pointer to the processror, GLOBAL or ALL for all  */
{
   PROCESSOR *pp;

   if (pptr)                     /* Initiate collection for processror *pptr only     */
      QueueCollectStats(pptr->RdyList,UTIL,MEANS,NOHIST,0,0.0,0.0);
   else {                        /* Initiate collection for all processors            */
      pp = (PROCESSOR*)YS__QueueNext(YS__ActPrcr,NULL);
      while (pp != NULL) {       /* YS__ActPrcr is a queue of all processors          */
         QueueCollectStats(pp->RdyList,UTIL,MEANS,NOHIST,0,0.0,0.0);
         pp = (PROCESSOR*)YS__QueueNext(YS__ActPrcr,pp);
      }
   }
}         

/**************************************************************************************/

double ProcessorGetUtil(pptr)    /* Returns % of busy time for processors             */
PROCESSOR *pptr;                 /* Pointer to the processor, GLOBAL or ALL for all   */
{
   if (pptr)                     /* Only one processor's utilizaiton returned         */
      return StatrecMean(QueueStatPtr(pptr->RdyList,UTIL));
   else                          /* Average of number of busy processors returned     */
      return StatrecMean(YS__BusyPrcrStat);
}

/**************************************************************************************/

STATREC *ProcessorStatPtr(pptr)  /* Returns a pointer to a processor's stat rec       */
PROCESSOR *pptr;                 /* Pointer to the processor, GLOBAL or ALL for all   */
{
   if (pptr)                     /* Processors stat rec returned                      */
      return QueueStatPtr(pptr->RdyList,UTIL);
   else                          /* Global utilization stat rec returned              */
      StatrecUpdate (YS__BusyPrcrStat, (double) YS__BusyPrcrs, YS__Simtime);      
      return YS__BusyPrcrStat;
}

/**************************************************************************************/

