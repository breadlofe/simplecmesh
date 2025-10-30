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
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

/**************************************************************************************\
********************************* Global Variables *************************************
****************************************************************************************
**                                                                                    **
**      All global variabes, except those that are are static and only accessed       **
**      in a single file, are defined here.  There is a corresponding extern          **
**      declaration of these variables in the file def.h.  Declarations that          **
**      start with YS__ are not visible to the user.                                  **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

POOL    YS__MsgPool;                     /* Pool of MESSAGE descriptors               */
POOL    YS__EventPool;                   /* Pool of EVENT descriptors                 */
POOL    YS__QueuePool;                   /* Pool of QUEUE descriptors                 */
POOL    YS__SemPool;                     /* Pool of SEMAPHORE descriptors             */
POOL    YS__FlagPool;                    /* Pool of FLAG descriptors                  */
POOL    YS__BarPool;                     /* Pool of BARRIER descriptors               */
POOL    YS__ResPool;                     /* Pool of RESOURCE descriptors              */
POOL    YS__StvarPool;                   /* Pool of IVAR and FVAR descriptors         */
POOL    YS__CondPool;                    /* Pool of CONDITION descriptors             */
POOL    YS__QelemPool;                   /* Pool of QELEM descriptors                 */
POOL    YS__StatPool;                    /* Pool of STATREC descriptors               */
POOL    YS__HistPool;                    /* Pool of histograms of default size        */

QUEUE   *YS__PendRes;                    /* Queue of Resources to be evaluated        */

EVENT   *YS__ActEvnt;                    /* Pointer to the currently occurring event  */
double  YS__Simtime;                     /* The current simulation time               */

int     YS__idctr;                       /* Used to generate unique ids for objects   */
char    YS__prbpkt[1024];                /* Buffer for probe packets                  */
int     YS__msgcounter = 0;              /* System defined unique message ID          */
int     YS__interactive = 0;             /* Flag; set if for viewsim or shsim         */

QUEUE   *YS__ActPrcr;                    /* List of active processors                 */
POOL    YS__PrcrPool;                    /* Pool of PROCESSOR descriptors             */
POOL    YS__PktPool;                     /* Pool of packet descriptors                */

int     YS__Cycles = 0;                  /* Count of accumulated profining cycles     */
double  YS__CycleTime = 1.0;             /* Cycle Time                                */
int     YS__NetCycleTime = 1.0;          /* Network cycle time (in clock cycles)      */

rngen   YS__RNGen;                       /* Random number generator                   */

STATREC *YS__BusyPrcrStat;		 /* Statrec for processor utilization         */
int	YS__BusyPrcrs;			 /* Number of busy processors                 */
int	YS__TotalPrcrs;			 /* Number of total processors 		      */


/********************** Global variables visible to the user *************************/

FVAR    *SIMTIME;                        /* An FVAR equal the current simulation time */
int     TraceLevel = 0;                  /* Controls the amount of trace information  */
int     TraceIDs = 1;                    /* If != 0, show object ids in trace output  */

/**************************************************************************************\
******************************** Utility Routines **************************************
****************************************************************************************
**                                                                                    **
**       These are miscellaneous routines for reporting errors and warnings           **
**       and for returning simulation time.                                           **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

void YS__errmsg(s)        /* Prints error message & terminates simulation             */
char *s;
{
   fprintf(stderr, "\nERROR: %s\n\n",s);
   exit(-1);
}

/**************************************************************************************/

void YS__warnmsg(s)       /* Prints warning message (if TraceLevel > 0)               */
char *s;
{
   if (TraceLevel >= MAXDBLEVEL-4)
      TracePrintTag("warnings","\nWARNING: %s\n\n",s);
}

/**************************************************************************************/

double GetSimTime()          /* Returns the current simulation time                   */
{
   return YS__Simtime;
}

/*****************************************************************************/
/* Trace Output routines:                                                    */
/* These routines print tracing information on the simulator output.         */
/*****************************************************************************/

void YS__SendPrbPkt (  /* Sends a packet to a probe         */
	char type,               /* The type of the probe             */
	const char *name,        /* Name of the object generating the packet */
	const char *data         /* Packet data */
)
{
  /* These functions have a different role with dbsim than with RSIM.
     In RSIM, trace messages go only to simulator output currently. */

  if (type==TEXTPROBE)     /* The only types of probes supported in RSIM  */
    //fprintf(simout,data)
    ;  /* are TEXT PROBES                             */
}

void TracePrint(const char *fmt, ...)     /* Sends user generated messages to text probe */
{
    va_list var;
    va_start (var, fmt);

    //vfprintf(simout,fmt, var);

    va_end(var);
}

void TracePrintTag(const char *tag, const char *fmt, ...)  /* Sends a tagged message to a text probe     */
{                             /* Probes can filter on this tag              */
    va_list var;
    va_start (var, fmt);

   // vfprintf(simout,fmt, var);

    va_end(var);
}


/**************************************************************************************\
******************************** Random Number Generators ******************************
****************************************************************************************
**                                                                                    **
**     Yacsim provides several choices for generating random numbers and it is        **
**     relatively easy for the user to add others.                                    **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/


   /** YacRand Operations **********************************************************\
   *                                                                                *
   *     YacRand() uses a multiplicative congruential random number generator       *
   *     suggested by Shedler in Lavenberg's "Computer Performance Modeling         *
   *     Handbook.  it is the same generator used in the IBM System/360 (save for   *
   *     the difference in 360 and IEEE 754 floating point arithmetic).  The        *
   *     generator is specified as                                                  *
   *                                                                                *
   *        Xn+1 = aXn mod m                                                        *
   *                                                                                *
   *     where a = 16807 ( = 7^5)                                                   *
   *           m = 2147483647 ( = 2^31 - 1)                                         *
   *                                                                                *
   *     YacRand() actually produces a uniformly distributed random number x,       *
   *     0 < x < 1, by dividing the (integer) Xn+1 by the modulus m and returning   *
   *     this value.                                                                *
   *                                                                                *
   *     YacSeed() seeds the random number generator, which otherwise uses a seed   *
   *     of 1.  In order to maintain a correspondence between yacrand and drand48,  *
   *     the seed value, which is a double, should be U(0,1), and it gets           *
   *     multiplied by the modulus to produce an integer seed between 1 and the     *
   *     modulus.  Calling YacSeed() with a seed of 0 causes YacSeed() to use the   *
   *     default value of 1.                                                        *
   *                                                                                *
   \********************************************************************************/


/**************************************************************************************/

#define  MULTIPLIER 16807.
#define  MODULUS    2147483647.

static int YS__yac_ran_val = 1073741824;

/**************************************************************************************/

double YacRand()      /* Generates and returns the next random number                 */
{
   double temp;

   temp = fmod(MULTIPLIER * YS__yac_ran_val,MODULUS);
   YS__yac_ran_val = temp;
   return(temp/MODULUS);
}

/**************************************************************************************/

void YacSeed(seed)    /* Sets the seed for the random number generator                */
double seed;
{
   if (seed != 0.) YS__yac_ran_val = MODULUS * seed;
   else YS__yac_ran_val = 1073741824;
}

/**************************************************************************************/


   /** YacRandom Random Number Generator *******************************************\
   *                                                                                *
   *     YacRandom() uses a multiplicative congruential random number generator     *
   *     that computes the next random number according to                          *
   *                                                                                *
   *        Xn+1 = aXn mod m                                                        *
   *                                                                                *
   *     where a = 16807 ( = 7^5) and m = 2147483647 ( = 2^31 - 1)                  *
   *                                                                                *
   *     and shuffles the output due to the Bays and Durham algorithm to remove     *
   *     serial correlations.                                                       *
   *     ----------------------------------------------------------------------     *
   *     Code for YacRandom was provided by Stephan Volmer & Patrick Dowd           *
   *                                                                                *
   \********************************************************************************/


/**************************************************************************************/

#define INITIALIZED     1
#define NOT_INITIALIZED 0
#define FLAG_UP         1
#define FLAG_DOWN       0
#define MAX_SHUFFLE     32
#define A               16807
#define M               2147483647
#define Q               127773
#define R               2836
#define PI              3.1415926558979324

static int  YS__Stream;
static int  YS__InitFlag = NOT_INITIALIZED;
static long YS__Seed[16];
static long YS__ShuffleTableIndex[16];
static long YS__SeedShuffleTable[16][MAX_SHUFFLE];
static long YS__SeedInit[16]
   = {1973272912L,  747177549L,   20464843L,  640830765L, 1098742207L,
         8126602L,   84743774L,  831312807L,  124667236L, 1172177002L,
      1124933064L, 1223960546L, 1878892440L, 1449793615L,  553303732L,
       438743253L};

/**************************************************************************************/

long YS__NextSeed(StreamNumber)   /* Returns the next seed of a random stream         */
int StreamNumber;                 /* Random stream number                             */
{
  long Seed, Temp;

  Seed = YS__Seed[StreamNumber];
  Temp = Seed / Q;
  Seed = A*(Seed - Q*Temp) - R*Temp;
  if (Seed < 0)
    Seed += M;
  YS__Seed[StreamNumber] = Seed;
  return(Seed);
}

/**************************************************************************************/

double YacRandom()         /* Generates and returns the next random number in [0,1]   */
{
  int Index;
  long Output;

  if (YS__InitFlag != INITIALIZED)
    YS__errmsg("Random variable generator not initialized");
  Index = YS__ShuffleTableIndex[YS__Stream] / (1+(M-1)/MAX_SHUFFLE);
  Output = YS__SeedShuffleTable[YS__Stream][Index];
  YS__ShuffleTableIndex[YS__Stream] = Output;
  YS__SeedShuffleTable[YS__Stream][Index] = YS__NextSeed(YS__Stream);
  return((double) Output*4.65661287525E-10);
}

/**************************************************************************************/

void YacRandomInit()       /* Initializes data structures for random generator        */
{
  int i,j;

  for(i=0; i<16; i++)
    {
      YS__Seed[i] = YS__SeedInit[i];
      YS__ShuffleTableIndex[i] = YS__Seed[i];
      YS__SeedShuffleTable[i][0] = YS__Seed[i];
      for(j=1; j<MAX_SHUFFLE; j++)
        YS__SeedShuffleTable[i][j] = YS__NextSeed(i);
    }
  YS__Stream = 0;
  YS__InitFlag = INITIALIZED;
}

/**************************************************************************************/

void YacRandomStream(StreamNumber)       /* Selects one of the 16 random streams      */
int StreamNumber;                        /* New random stream number                  */

{
  if ((StreamNumber < 0) || (StreamNumber > 15))
    YS__errmsg("Incorrect stream number in SelectStream");
  YS__Stream = StreamNumber;
}

/**************************************************************************************/


   /*** Unix Random Number Generator  **********************************************\
   *                                                                                *
   *     These routines make the Unix rng random() available for use as the         *
   *     internal rng of YACSIM. It divides the output of random to produce a       *
   *     random number between 0 and 1.                                             *
   *                                                                                *
   \********************************************************************************/


/**************************************************************************************/

double UnixRandom()
{
   return (random()*4.65661287525E-10);
}

/**************************************************************************************/


   /*** Internal YACSIM Random Number Generator  ***********************************\
   *                                                                                *
   *   These routines are used to set the internal system random number generator   *
   *   and to provide direct user access to it.                                     *
   *                                                                                *
   \********************************************************************************/


/**************************************************************************************/

void SetSysRand(rng)  /* Specifies the random number generator YACSIM uses internally */
rngen rng;            /* Pointer to the random number generator to use                */
{
   YS__RNGen = rng;
}

/**************************************************************************************/

double SysRand()            /* Returns a random number generated by the internal      */
{                           /* YACSIM random number generator                         */
   double x;

      return YS__RNGen();
}

/**************************************************************************************/


   /*** Derived Random Distributions ***********************************************\
   *                                                                                *
   *   These routines provide various random distributions derived from the         *
   *   internal system random number generator.                                     *
   *                                                                                *
   \********************************************************************************/


/**************************************************************************************/

double RandUniform(LowerBound, UpperBound)   /* Returns uniformly distributed real    */
                                             /* random numbers in the interval [a,b]  */
double LowerBound;                           /* Lower bound of the interval           */
double UpperBound;                           /* Upper bound of the interval           */

{
   if (LowerBound >= UpperBound)
      YS__errmsg("Incorrect interval boundaries in RandUniform");
   if (LowerBound == UpperBound)
      YS__warnmsg("Equal interval boundaries in RandUniform");
   return(LowerBound + (UpperBound - LowerBound) * SysRand());
}

/**************************************************************************************/

double RandExponential(Mean) /* Returns exponentially distributed real random numbers */
                             /* with mean value lambda                                */
double Mean;                 /* Mean value of the distribution                        */

{
   double rn;

   if (Mean <= 0)
      YS__errmsg("Nonpositive mean argument in RandExponential");
   while ( (rn = SysRand()) == 0.0 );
   return ( -(Mean * log(rn)) );
}

/**************************************************************************************/

double RandErlang(Mean,Freedom)  /* Returns Erlang distributed real random numbers    */
double Mean;                     /* Mean value of the distribution                    */
int    Freedom;                  /* Degree of freedom of the distribution             */
{
   double x = 1.0;
   double f = Freedom;
   int    k;
   if ((Mean <= 0) | (Freedom <= 0)) YS__errmsg("Argument <= 0 in RandErlang");
   for(k=1; k<=Freedom; k++) x = x * SysRand();
   return (-((Mean/f) * log(x)));
}

/**************************************************************************************/

#define FLAG_DOWN  0
#define FLAG_UP    1

double RandNormal(Mean,StdDev) /* Returns Gaussian distributed real random numbers    */
double Mean;                   /* Mean value of the distribution                      */
double StdDev;                 /* Standard deviation of the distribution              */
{
   static int    Flag = FLAG_DOWN;
   static double ExtraDev;
   double        Trans, Mag, Ran1, Ran2;

   if (StdDev < 0.0) YS__errmsg("Negative StdDev argument in RandNormal");
   if (Flag == FLAG_DOWN) {
      do {
         Ran1 = 2.0 * SysRand() - 1.0;
         Ran2 = 2.0 * SysRand() - 1.0;
         Mag = Ran1 * Ran1 + Ran2 * Ran2;
      } while((Mag >= 1.0) || (Mag == 0.0));
      Trans = sqrt(-2.0*log(Mag)/Mag);
      ExtraDev = Ran1 * Trans;
      Flag = FLAG_UP;
      return(Mean + StdDev * Ran2 * Trans);
   }
  else
    {
      Flag = FLAG_DOWN;
      return(Mean + StdDev * ExtraDev);
    }
}

/**************************************************************************************/

double RandPareto(mean)
double mean;
{
   return(mean*((1.0/sqrt(SysRand()))-1.0));
}

/**************************************************************************************/

int RandUniformInt(LowerBound, UpperBound)   /* Returns uniformly distributed integer */
                                             /* random numbers in the interval [i,j]  */
int LowerBound;                              /* Lower bound of the interval           */
int UpperBound;                              /* Upper bound of the interval           */
{
   int diff;
   double x;
   int i;

   if (LowerBound >= UpperBound)
      YS__errmsg("Incorrect interval boundaries in RandUniformInt");
   if (LowerBound == UpperBound)
      YS__warnmsg("Equal interval boundaries in RandUniformInt");
   diff = UpperBound - LowerBound + 1;
   x = diff*1.0;
   i = (int) ( x*SysRand() );
   i = i + LowerBound;
   return i;
}

/**************************************************************************************/

int RandBernoulli(p)    /* Returns a Bernoulli distributed binary number              */
double p;               /* Mean of the distribution                                   */
{
   double x;

   if (p < 0.0 || p > 1.0)
      YS__errmsg("Argument not between 0 and 1 in RandBernoulli");
   x = SysRand();
   if (x <= p) return 1;
   else        return 0;
}

/**************************************************************************************/

int RandBinomial(t,p)   /* Returns a number from a Binmomial distribution             */
int t;                  /* Number of trials                                           */
double p;               /* Probability of success for trial                           */
{
   int i;
   double x;
   int cnt = 0;

   if (p < 0.0 || p > 1.0)
      YS__errmsg("Probability argument not between 0 and 1 in RandBinomial");

   for (i=0; i<t; i++) {
      x = SysRand();
      if (x <= p) cnt++;
   }
   return cnt;
}

/**************************************************************************************/

int RandNegBinomial(s,p) /* Returns a number from a negative Binomial distribution    */
int s;                   /* Number of consecutive successes                           */
double p;                /* Probability of success per trial                          */
{
   int i, j;
   int cnt = 0;

   if (p < 0.0 || p > 1.0)
      YS__errmsg("Probability argument not between 0 and 1 in RandGeometric");
   for (i=0; i<s; i++) {
      cnt = cnt + (int)(log(SysRand())/log(1-p));
   }
   return cnt;
}

/**************************************************************************************/

int RandGeometric(p)   /* Returns a geometrically distributed random number           */
double p;              /* Probability of success per trial                            */
{
   if (p < 0.0 || p > 1.0)
      YS__errmsg("Probability argument not between 0 and 1 in RandGeometric");
   return (int)(log(SysRand())/log(1-p));
}

/**************************************************************************************/

int RandPoisson(Mean)  /* Returns a Poisson distributed integer random number         */
double Mean;           /* Mean of the distribution                                    */
{
   static double SqrtMean, LogMean, ExpMean, OldMean = (-1.0);
   double Comp, Output = (-1.0), Tmp = 1.0;

   if (Mean <= 0) YS__errmsg("Mean argument <= 0 in RandPoisson");

   if (Mean != OldMean) {
      OldMean = Mean;
      ExpMean = exp(-Mean);
   }
   do {
      Output++;
      Tmp *= SysRand();
   } while (Tmp > ExpMean);

   return((int) Output);
}

/**************************************************************************************/
