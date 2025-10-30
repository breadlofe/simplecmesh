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
#include "../incl/tr.stat.h"
#include "malloc.h"

#define LOWBIN   srptr->hist[srptr->bins]     /* Max value for the low histogram bin  */
#define HIGHBIN  srptr->hist[srptr->bins+1]   /* Min value for the high histogram bin */
#define BININC   srptr->hist[srptr->bins+2]   /* Historgram bin size                  */

/**************************************************************************************\
********************************* STATREC Operations ***********************************
****************************************************************************************
**                                                                                    **
**      Statistics records are used to collect and display statistics for a           **
**      simulation.  Each STATREC can compute a running mean, standard deviation,     **
**      max, mean and histogram.  Collected statistics may be displayed in            **
**      a stadard format, or they can be constructed in any user defined format       **
**      using operations that return all relevant STATREC information                 **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

STATREC *NewStatrec(srname,typ,meanflg,histflg,nbins,lowbin,highbin)
/* Creates and returns a pointer to a new statistics record                           */
char    *srname;         /* User defnined name                                        */
int     typ;             /* POINT or INTERVAL                                         */
int     meanflg;         /* MEANS or NOMEANS                                          */
int     histflg;         /* HIST, NOHIST                                              */
int     nbins;           /* Number of histogram bins                                  */
double  lowbin;          /* Max of low bin                                            */
double  highbin;         /* Min of high bin                                           */
{
   STATREC *srptr;
   int i;

   PSDELAY;

   srptr = (STATREC*)YS__PoolGetObj(&YS__StatPool);
   srptr->id = YS__idctr++;
   strncpy(srptr->name,srname,31);
   srptr->name[31] = '\0';
   srptr->meanflag = meanflg;
   srptr->sum = 0.0;
   srptr->sumsq = 0.0;
   srptr->sumwt = 0.0;
   srptr->samples = 0;
   srptr->time0 = YS__Simtime;   /* Beginning of sampling interval                    */
   srptr->time1 = YS__Simtime;   /* End of sampling interval                          */
   srptr->interval = 0.0;        /* Interval of last interval staterec update         */
   srptr->intervalerr = 0;       /* Set if interval of interval statrec is ever neg   */

   if (typ == POINT)  srptr->type = PNTSTATTYPE;
   else if (typ == INTERVAL) srptr->type = INTSTATTYPE;
   else YS__errmsg("Invalid statistics record type, use POINT or INTERVAL");

   if (histflg == HIST) {        /* Histograms will be collected                      */
      srptr->bins = nbins+2;
      if ((srptr->bins+4)*sizeof(double) <= YS__HistPool.objsize) {/* Size <= default */
         srptr->hist = (double*)YS__PoolGetObj(&YS__HistPool);
         srptr->hist = srptr->hist + 1;   /* Preserve the pool pointer                */
      }
      else {                                                      /* Size > default   */
         srptr->hist = (double*)malloc((srptr->bins+4)*sizeof(double));
         if (srptr->hist == NULL) YS__errmsg("Malloc fails in NewStatrec");
      }
      for (i = 0; i<srptr->bins; i++) *((srptr->hist)+i) = 0.0;   /* Clear all bins   */
      if (nbins > 0)
         BININC = (highbin-lowbin)/nbins;                         /* Bin size         */
      else
         BININC = 0.0;
      LOWBIN = lowbin;
      HIGHBIN = highbin;
      TRACE_STATREC_new2;        /* Creating statistics record ...                    */
/*    TRACE_STATREC_create;      /* Sends bin size, low bin and # of bins to view sim */
   }
   else if (histflg == NOHIST) { /* Histograms will not be collected                  */
      srptr->hist = NULL;
      srptr->bins = 0;
      TRACE_STATREC_new1;        /* Creating Statistics record ...                    */
   }
   else YS__errmsg("Invalid histogram flag, use HIST or NOHIST");
   return srptr;
}

/**************************************************************************************/

void StatrecSetHistSz(i)    /* Sets the default size for the pool of histograms       */
int i;                      /* New size                                               */
{
   TRACE_STATREC_sethistsz;   /* Setting default histogram pool size to ...           */
   if (YS__HistPool.p_head == NULL) YS__HistPool.objsize = i;
   else
      YS__warnmsg("Can only set the default histogram size before calling NewStatrec");
}

/**************************************************************************************/

int YS__StatrecId(srptr)    /* Returns the system defined ID or 0 if TrID is 0        */
STATREC *srptr;             /* Pointer to the statrec                                 */
{
   if (TraceIDs) return srptr->id;
   else return 0;
}

/**************************************************************************************/

void StatrecReset(srptr)                 /* Resets the statrec                        */
STATREC *srptr;                          /* Pointer to the statrec                    */
{
   int i;

   PSDELAY;

   if (srptr->hist)  for (i=0; i<srptr->bins; i++) *(srptr->hist+i) = 0.00;
   if (srptr->meanflag == MEANS) {
      srptr->sum = 0.0;
      srptr->sumsq = 0.0;
      srptr->sumwt = 0.0;
   }
   srptr->samples = 0;
   srptr->time0 = YS__Simtime;   /* Beginning of sampling interval                    */
   srptr->time1 = YS__Simtime;   /* End of sampling interval                          */
   srptr->intervalerr = 0;       /* Set if interval of interval statrec is ever neg   */
   srptr->interval = 0.0;        /* Interval of last interval staterec update         */
   TRACE_STATREC_reset;          /* Resetting statistics records ...                  */
/* TRACE_STATREC_clear;          /* Sending "reset"signal to viewsim                  */
}

/**************************************************************************************/

void StatrecUpdate(srptr,v,t)            /* Updates a statrec                         */
STATREC *srptr;                          /* Pointer to the statrec                    */
double v;                                /* New value                                 */
double t;                                /* Weight or interval                        */
{
   int i;

   PSDELAY;

   if (srptr->type == INTSTATTYPE) {   /* This is an interval statistics record       */
      if (srptr->samples == 0) {       /* This is the first sample                    */
         srptr->lastt = t;             /* Updates of interval statrecs require last   */
         srptr->lastv = v;             /* two values to compute an interval, so this  */
         srptr->maxval = 0.0;          /* sample only initializes the first of them   */
         srptr->minval = 0.0;          /* Max & min refers to intervals not values    */
      }
      else {                           /* This is not the first sample                */
         srptr->interval = t - srptr->lastt;                /* New interval computed  */
         if (srptr->interval < 0.0) srptr->intervalerr =1;  /* Not a valid interval   */
         if (srptr->meanflag == MEANS) {                    /* Collect means          */
            srptr->sum += srptr->lastv * srptr->interval;   /* Accumulate sums        */
            srptr->sumsq += srptr->interval * srptr->lastv * srptr->lastv;
            srptr->sumwt += srptr->interval;
         }
         if (srptr->hist) {                                    /* Collect histograms  */
            if (srptr->interval >= 0.0)  {                     /* Inteval is valid    */
               for (i=0; i<srptr->bins-1; i++)                 /* Find the bin        */
                  if (srptr->lastv < LOWBIN + i*BININC) break; /* and increment it    */
               srptr->hist[i] += srptr->interval;
         }  }
/*       TRACE_STATREC_isample /* Sending interval statrec update sample to viewsim   */
         if (v > srptr->maxval) srptr->maxval = v;
         if (v < srptr->minval) srptr->minval = v;
         srptr->lastv = v;     /* Remember last value & time to compute next interval */
         srptr->lastt = t;
      }
      srptr->samples++;
      srptr->time1 = YS__Simtime;             /* New end of sampling interval         */
      TRACE_STATREC_iupdate;                  /* Updating interval statistics record  */
   }
    else if (srptr->type == PNTSTATTYPE) {     /* This is an point statistics record   */
      if (srptr->samples == 0) {              /* This is the first sample             */
         srptr->maxval = v;
         srptr->minval = v;
      }
      if (srptr->meanflag == MEANS) {         /* Collect means                        */
         srptr->sum += v*t;                   /* Accumlate sums                       */
         srptr->sumsq += t*v*v;
         srptr->sumwt += t;
      }
      if (srptr->hist) {                      /* Collect histograms                   */
         for (i=0; i<srptr->bins-1; i++)      /* Find the bin                         */
           if (v < LOWBIN + i*BININC) break;
         srptr->hist[i] += t;                 /* and increment it                     */
      }
/*    TRACE_STATREC_psample    /* Sending point statrec update sample to viewsim      */
      if (v > srptr->maxval) srptr->maxval = v;
      if (v < srptr->minval) srptr->minval = v;
      srptr->samples++;
      srptr->time1 = YS__Simtime;             /* New end of sampling interval         */
      TRACE_STATREC_pupdate;                  /* Updating point statistics record ... */
   }
   else YS__errmsg("Invalid statistic record type, use POINT or INTERVAL");
}

/**************************************************************************************/

void StatrecReport(srptr)                /* Generates and displays a statrec report   */
STATREC *srptr;                          /* Pointer to the statrec                    */
{
   int i,j;
   double total = 0.0;
   double stddev;
   double x;

   PSDELAY;

   if (srptr->type == PNTSTATTYPE) {
      TracePrintTag("statrpt","\nStatistics Record %s:\n",srptr->name);
      TracePrintTag("statrpt",
             "   Number of samples = %d,   Max Value = %g,   Min Value = %g\n",
             srptr->samples,srptr->maxval,srptr->minval);
      if (srptr->time1 > srptr->time0)
         TracePrintTag("statrpt","   Sampling interval = %g,   Sampling rate = %g\n",
             srptr->time1 - srptr->time0, srptr->samples/(srptr->time1 - srptr->time0));
      else TracePrintTag("statrpt","   Sampling interval = 0,   Sampling rate = inf\n");
      if (srptr->sumwt != 0.0 && srptr->sumwt != 1.0) {
         x = ((srptr->sumsq-(srptr->sum/srptr->sumwt)
              *(srptr->sum/srptr->sumwt)*srptr->sumwt)/(srptr->sumwt-1.0));
         if (x > 0.0)
            stddev = sqrt(x);
         else
            stddev = 0.0;
      }
      else stddev = 0.0;
      if (stddev == -0.0) stddev = 0.0;
      if (srptr->meanflag == MEANS)
         TracePrintTag("statrpt","   Mean = %g,   Standard Deviation = %g\n",
                    srptr->sum/srptr->sumwt,stddev);
      if (srptr->hist)  {
         for (i=0; i<srptr->bins; i++) total += srptr->hist[i];
         if (total != 0.0) {
            TracePrintTag("statrpt","      Bin         Value\n");
            TracePrintTag("statrpt","      ---         -----\n");
            TracePrintTag("statrpt","       %s      %6.3f (%5.2f%s) |",
                   "-",srptr->hist[0],(srptr->hist[0]/total)*100,"%");
            for (j=0; j<(int)((srptr->hist[0]/total)*50); j++)
               TracePrintTag("statrpt","%s","*");
            TracePrintTag("statrpt","\n");
            for (i=1; i<srptr->bins; i++)  {
               TracePrintTag("statrpt","    %6.3f    %6.3f (%5.2f%s) |",
                    LOWBIN+(i-1)*BININC,srptr->hist[i],(srptr->hist[i]/total)*100,"%");
               for (j=0; j<(int)((srptr->hist[i]/total)*50); j++)
                  TracePrintTag("statrpt","%s","*");
               TracePrintTag("statrpt","\n");
         }  }
         else YS__warnmsg("All histogram entries = 0");
      }
      TracePrintTag("statrpt","End of Report\n\n");
   }
   else if (srptr->type == INTSTATTYPE) {
      TracePrintTag("statrpt","\nStatistics Record %s:\n",srptr->name);
      if (srptr->intervalerr != 0) YS__warnmsg("Invalid statistics; interval error");
      TracePrintTag("statrpt",
             "   Number of intervals = %d, Max Value = %g, Min Value = %g\n",
             srptr->samples-1,srptr->maxval,srptr->minval);
      if (srptr->time1 > srptr->time0)
         TracePrintTag("statrpt","   Sampling interval = %g,   Sampling rate = %g\n",
             srptr->time1 - srptr->time0, srptr->samples/(srptr->time1 - srptr->time0));
      else TracePrintTag("statrpt","   Sampling interval = 0,   Sampling rate = inf\n");
      if (srptr->sumwt != 0.0 && srptr->sumwt != 1.0) {
         x = ((srptr->sumsq-(srptr->sum/srptr->sumwt)
              *(srptr->sum/srptr->sumwt)*srptr->sumwt)/(srptr->sumwt-1.0));
         if (x > 0.0)
            stddev = sqrt(x);
         else
            stddev = 0.0;
      }
      else stddev = 0.0;
      if (stddev == -0.0) stddev = 0.0;
      if (srptr->meanflag == MEANS)
         TracePrintTag("statrpt","   Mean = %g,   Standard Deviation = %g\n",
                    srptr->sum/srptr->sumwt,stddev);
      if (srptr->hist)  {
         for (i=0; i<srptr->bins; i++) total += srptr->hist[i];
         if (total != 0.0) {
               TracePrintTag("statrpt","      Bin         Value\n");
            TracePrintTag("statrpt","      ---         -----\n");
            TracePrintTag("statrpt","       %s      %6.3f (%5.2f%s) |",
                   "-",srptr->hist[0],(srptr->hist[0]/total)*100,"%");
            for (j=0; j<(int)((srptr->hist[0]/total)*50); j++)
               TracePrintTag("statrpt","%s","*");
            TracePrintTag("statrpt","\n");
            for (i=1; i<srptr->bins; i++)  {
               TracePrintTag("statrpt","    %6.3f    %6.3f (%5.2f%s) |",
                    LOWBIN+(i-1)*BININC,srptr->hist[i],(srptr->hist[i]/total)*100,"%");
               for (j=0; j<(int)((srptr->hist[i]/total)*50); j++)
                  TracePrintTag("statrpt","%s","*");
               TracePrintTag("statrpt","\n");
         }  }
         else YS__warnmsg("All histogram entries = 0");
      }
      TracePrintTag("statrpt","End of Report\n\n");
   }
   else YS__errmsg("Invalid statistic record type, use POINT or INTERVAL");
}

/**************************************************************************************/

int StatrecSamples(srptr)                /* Returns the number of samples             */
STATREC *srptr;                          /* Pointer to the statrec                    */
{
   PSDELAY;

   return srptr->samples;
}

/**************************************************************************************/

double StatrecMaxVal(srptr)              /* Returns the maximum sample value          */
STATREC *srptr;                          /* Pointer to the statrec                    */
{
   PSDELAY;

   return srptr->maxval;
}

/**************************************************************************************/

double StatrecMinVal(srptr)              /* Returns the minimum sample value          */
STATREC *srptr;                          /* Pointer to the statrec                    */
{
   PSDELAY;

   return srptr->minval;
}

/**************************************************************************************/

int StatrecBins(srptr)                   /* Returns the number of bins                */
STATREC *srptr;                          /* Pointer to the statrec                    */
{
   PSDELAY;

   if (srptr->hist) return srptr->bins;
   else YS__errmsg("Histograms not calculated for this statistics record");
   return 0.0;         /* Never executed; prevents warning messages on some compilers */
}

/**************************************************************************************/

double StatrecLowBin(srptr)              /* Returns the low bin upper bound           */
STATREC *srptr;                          /* Pointer to the statrec                    */
{
   PSDELAY;

   if (srptr->hist) return LOWBIN;
   else YS__errmsg("Histograms not calculated for this statistics record");
   return 0.0;         /* Never executed; prevents warning messages on some compilers */
}

/**************************************************************************************/

double StatrecHighBin(srptr)             /* Returns the high bin lower bound          */
STATREC *srptr;                          /* Pointer to the statrec                    */
{
   PSDELAY;

   if (srptr->hist) return HIGHBIN;
   else YS__errmsg("Histograms not calculated for this statistics record");
   return 0.0;         /* Never executed; prevents warning messages on some compilers */
};

/**************************************************************************************/

double StatrecBinSize(srptr)             /* Returns the bin size                      */
STATREC *srptr;                          /* Pointer to the statrec                    */
{
   PSDELAY;

   if (srptr->hist) return BININC;
   else YS__errmsg("Histograms not calculated for this statistics record");
   return 0.0;         /* Never executed; prevents warning messages on some compilers */
}

/**************************************************************************************/

double StatrecHist(srptr,i)         /* Returns the value of the ith histogram element */
STATREC *srptr;                     /* Pointer to the statrec                         */
int i;
{
   PSDELAY;

   if (srptr->hist) return srptr->hist[i];
   else YS__errmsg("Histograms not calculated for this statistics record");
   return 0.0;         /* Never executed; prevents warning messages on some compilers */
}

/**************************************************************************************/

double StatrecMean(srptr)                /* Returns the mean                          */
STATREC *srptr;                          /* Pointer to the statrec                    */
{
   PSDELAY;

   if (srptr->meanflag == MEANS) return srptr->sum/srptr->sumwt;
   else YS__errmsg("Means not calculated for this statistics record");
   return 0.0;         /* Never executed; prevents warning messages on some compilers */
}

/**************************************************************************************/

double StatrecSdv(srptr)                 /* Returns the standard deviation            */
STATREC *srptr;                          /* Pointer to the statrec                    */
{
   PSDELAY;

   if (srptr->meanflag == MEANS)
      return sqrt((double)((srptr->sumsq-(srptr->sum/srptr->sumwt)*
                  (srptr->sum/srptr->sumwt)*srptr->sumwt)/(srptr->sumwt-1)));
   else YS__errmsg("Std. Dev. not calculated for this statistics record");
   return 0.0;         /* Never executed; prevents warning messages on some compilers */
}

/**************************************************************************************/

double StatrecInterval(srptr)            /* Returns the sampling interval             */
STATREC *srptr;                          /* Pointer to the statrec                    */
{
   PSDELAY;

   return srptr->time1 - srptr->time0;
}

/**************************************************************************************/

double StatrecRate(srptr)                /* Returns the sampling rate                 */
STATREC *srptr;                          /* Pointer to the statrec                    */
{
   PSDELAY;

   return srptr->samples / (srptr->time1 - srptr->time0);
}

/**************************************************************************************/

void StatrecEndInterval(srptr)           /* Terminates a sampling interval            */
STATREC *srptr;                          /* Pointer to the statrec                    */
{
   PSDELAY;

   srptr->time1 = YS__Simtime;
}

/**************************************************************************************/
