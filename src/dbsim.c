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

#include "../incl/dbsim.h"
#include <varargs.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <a.out.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/time.h>

/******************************* Defines and Typedefs ********************************/

#define  FILTER_NAME_LENGTH     30   /* Maximum length of a filter name              */
#define  NUM_COMMANDS           12   /* Number of user commands                      */
#define  SHSIM_TMP_DIR       "/tmp"

typedef struct {                 /* Command information, one for each command         */
  char *name;                    /* Name of the command                               */
  int (*func)();                 /* Pointer to the code for the command               */
  char *short_desc;              /* Description used in summary of commands           */
  char *long_desc;               /* Help entry for the command                        */
} command_info;

typedef struct t_info {          /* Tag information, one for each tag                 */
  char *name;                    /* Tag name used for filtering                       */
  struct t_info *prev;           /* Tags can be in lists                              */
  struct t_info *next;
} tag_info;

typedef struct f_info {          /* Filter information, one for each filter           */
  char *name;                    /* Filter name used for indentification              */
  FILE *file;                    /* Either stdout or a file name                      */
  struct t_info *tags;           /* List of tags associated with the filter           */
  struct f_info *prev;           /* Filters can be in lists                           */
  struct f_info *next;
} filter_info;

/********************************* Global Variables ***********************************/

static pid_t    DB__SimPtr = NULL; /* Pointer to the currently active simulation prog */
static char   **DB__Argv;          /* Pointer to simulation arguments, in argv format */
static double   DB__SimTime;       /* updated by CLOCK signal                         */
static int      DB__SimIO;         /* Pointer to socket from DBSIM to simulation      */
static char    *DB__ClrStr;        /* Character string that will clear the terminal   */
static int      DB__Mode;          /* User selected mode                              */
static double   DB__Step;          /* User selected step size                         */
static int      DB__TrLevel;       /* User selected trace level                       */
static int      DB__Break = 1;     /* Breakpoint status                               */

static filter_info *DB__FilterList;               /* head of the filter list          */
static command_info DB__CmdList[NUM_COMMANDS];    /* List of all commands             */
static char   *prog = "DBSIM";   /* program name, used when reporting status          */

/******************************* Function Declarations ********************************/

static int Help();              /* Functions that realize the user commands           */
static int Clear();
static int Mode();
static int Step();
static int Trace();
static int Break();
static int Filter();
static int Load();
static int Exec();
static int Reset();
static int Kill();
static int Quit();

static void PrintMsg();         /* Miscellaneous Functions                            */
static void SendCommand();
static char *GetClearStr();
static void InitCommandList();
static void InitFilter();
static void DoFilter();
static char *StrDup();
static int  AddTag();
static int  DeleteTag();
static int  PrintFilter();
static int  DeleteFilter();
static int  Compare();
static void PutPrompt();
static int  GetUserCmd();
static int  GetSimRsp();

/**************************************************************************************\
************************** User and Driver Interaction Routines ************************
****************************************************************************************
**                                                                                    **
**      These routines provide the interface between DBSIM and the user terminal      **
**      and between dbsim and the YACSIM driver.                                      **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

main(argc, argv)         /* This is where it all starts                               */
int argc;                /* Number of arguments; argv[1] is name of simulatin to run  */
char* argv[];            /* Other argc-2 arguments passed to that simulation program  */
{
   int i;

   DB__ClrStr = GetClearStr();      /* Get string that clears screen from termcap     */
   InitCommandList();               /* Initialize command lists                       */
   InitFilter();                    /* Initialize filter structures                   */
   DB__Mode = FOR_TIME;             /* Initial mode is "for time"                     */
   DB__Step = 1.0;                  /* Initial step size is 1                         */
   DB__TrLevel = 3;                 /* Initial trace level is 3                       */
   printf("\n");

   if (argc > 1) {       /* if any arguments are given, use as sim name and arguments */
      DB__Argv = (char**) malloc((argc)*sizeof(char*));  /* Get remaining arguments   */
      for (i = 0; i < argc-1; i++) {
         DB__Argv[i] = StrDup(argv[i+1]);
      }
      DB__Argv[i] = NULL;           /* execvp expects last argument to be NULL        */
      if (SimStart(DB__Argv)) {     /* Try to load the specified simulation           */
         free(DB__Argv);
         DB__Argv = NULL;
         PrintMsg("Unable to load simulation; invalid simulation file");
      }
   }
   else {                /* No command line arguments                                 */
      DB__Argv = NULL;   /* DB__Argv will be non-NULL only if a simulation is loaded  */
   }

   for (;;) { /* Main command interpreation loop                                      */
      /* Get a command from the terminal and execute it                               */
         while (GetUserCmd()); /* Returns 0 when it needs an ack from the simulation  */
      /* Get a response from the simulation program and processes it                  */
         while (GetSimRsp());  /* Returns 0 when it gets the ack from the simulation  */
   }
}

/**************************************************************************************/

static int GetUserCmd()    /* Gets a user command from the terminal and executes it   */
{
   static char buf[80];     /* Buffer that holds typed command line                   */
   int i;

   PutPrompt();

   gets(buf);                        /* Read an input line from the terminal          */

   if (*buf == '\0') {                             /* User entered a carriage return  */
      if (DB__SimPtr != 0)                         /* If a simulation is active       */
         Exec();                                   /* Run it                          */
      return 0;                                    /* Go next to GetSimRsp on return  */
   }

   for (i=0; i<NUM_COMMANDS; i++) {                /* Compare cmd with all commands   */
      if (tolower(buf[0]) == DB__CmdList[i].name[0])
      {                                            /* Command is valid                */
         (*DB__CmdList[i].func)(buf);              /* Execute it                      */
         if (tolower(buf[0]) == 'e')               /* Command is Exec                 */
            return 0;                              /* Go next to GetSimCmd on return  */
         else                                      /* Command is not Exec             */
            return 1;                              /* Repeat GetUserCmd on return     */
      }
   }
   PrintMsg("Unknown command");                    /* Not a valid command             */
   return 1;                                       /* Repeat GetUserCmd on return     */
}

/**************************************************************************************/

static int GetSimRsp()   /* Get response from command sent to simulation              */
{
   char rsp;
   char buf[1024];
   int result, len, filled, i;
   char clen;

   if (DB__SimPtr == 0) return 0;     /* No simulation loaded                         */

   result = read(DB__SimIO, &rsp, 1); /* Blocking read; wait for response             */
   if (result == 0) {                 /* Simulation probably crashed                  */
      PrintMsg("Simulation exited with status %d",SimDone());
      return 0;                       /* Go next to GetUserCmd on return              */
   }
   else switch(rsp) {                 /* Got a vaild command from the simulation      */
      case ACK:                       /* Simulation ready for another command         */
         return 0;                    /* Go next to GetUserCmd on return              */
      case CLOSE:                     /* Simulation program terminated normally       */
         SimDone();
         PrintMsg("Simulation ended normally");
         return 0;                    /* Go next to GetUserCmd on return              */
      case BRKPT:                     /* Breakpoint hit in the simulation             */
         read(DB__SimIO, &clen, 1);   /* Get comment specified in break point command */
         read(DB__SimIO, buf, (int) clen);
         buf[clen] = '\0';
         PrintMsg("Breakpoint -- %s", buf); 
         return 0;                    /* Go next to GetUserCmd on return              */
      case CLOCK:                     /* Simulation updated simulation time           */
         result = read(DB__SimIO, &DB__SimTime, sizeof(DB__SimTime)); 
         if (result==0) {
            PrintMsg("Simulation exited with status %d",SimDone());
            return 0;                 /* Go next to GetUserCmd on return              */
         }
         return 1;                    /* Repeat GetSimRsp on return                   */
      case TEXTPROBE:                 /* Simulation sent data for a text probe        */
         result = read(DB__SimIO, &len, sizeof(len));
         if (result==0) {
            PrintMsg("Simulation exited with status %d",SimDone());
            return 0;                 /* Go next to GetUserCmd on return              */
         }
         filled = 0;                  /* Filled is the number of characters received  */
         while (filled < len) {       /* Not all characters received                  */
            result = read(DB__SimIO, buf+filled, len-filled);  /* Ask for the rest    */
            filled+=result;           /* Increment filled by number received          */
            if (result==0) {          /* Simulation probably crashed                  */
               PrintMsg("Simulation exited with status %d",SimDone());
               break;
            }
         }
         DoFilter(buf);               /* Process the trace packet received            */
         return 1;                    /* Repeat GetSimRsp on return                   */
      default:
         return 1;                    /* Repeat GetSimRsp on return                   */
   }
}

/**************************************************************************************/

static void PutPrompt()
{
   char *p;

   if (DB__Argv) {          /* There is loaded simulation program                     */
      if (DB__SimPtr != 0)  /* The simulation is executing or ready to execute        */
         printf("%s* | %.4f > ", DB__Argv[0], DB__SimTime); 
      else 
         printf("%s  | %.4f > ", DB__Argv[0], DB__SimTime);
   }
   else                    /*  No simulation loaded                                   */
      printf("no simulation loaded > ");
}

/**************************************************************************************/

int SimDone()           /* returns simulations exit status after it dies              */
{
   int exit_status;
   filter_info *lptr = DB__FilterList;
  
   if (wait(&exit_status) < 0) {  /* Suspends DBSIM until the simulation terminates    */
      PrintMsg("Wait failed");
      exit(errno);
   }
   if (close(DB__SimIO) < 0) {
      PrintMsg("Close failed");
      exit(errno);
   }
   DB__SimPtr = 0;                 /* No simulation loaded                             */

   while (lptr && strcmp(lptr->name,"stdout") !=  0) {
      fflush(lptr->file);
      lptr = lptr->next;
   }

  return exit_status;
}

/**************************************************************************************/

int SimStart(simargs)    /* Attempt to load the simulation specified by filename      */
char **simargs;          /* Arguments; simargs[0] is name of simulation program       */
{
   int fd, sim_desc[2];
   struct sockaddr_un server;
   char *simfile;
   char cmd;
   int result;

   simfile = StrDup(simargs[0]);    /* Get the name of the simulation program         */

   if (DB__SimPtr != 0) { /* If there is a simulation running, kill it                */
      if (kill(DB__SimPtr, SIGQUIT) < 0 && kill(DB__SimPtr, SIGTERM) < 0) {
         PrintMsg("Unable to kill simulation");
         exit(errno);
      }
      SimDone();
   }

   if ((fd = open(simfile, O_RDONLY)) < 0) { /* Make sure given simfile is valid      */
      return errno;
   }
   close(fd);

   if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) { /* Open socket                   */
      PrintMsg("Socket open failed");
      exit(errno);
   }
   server.sun_family = AF_UNIX;
   sprintf(server.sun_path, "%s/.%s-%d", SHSIM_TMP_DIR, prog, getpid());

   if (bind(fd, &server, sizeof(struct sockaddr_un)) < 0) { /* Bind name to socket    */
      PrintMsg("Bind failed");
      exit(errno);
   }
   listen(fd, 1);                    /* Wait for socket to be created                 */

   if ((DB__SimPtr = fork()) < 0) {  /* Fork a child process                          */
      PrintMsg("Fork failed");
      exit(errno);
   }

   if (DB__SimPtr == 0) {            /* This code is executed by the forked child     */

      if (simargs==NULL) {           /* Set up argv vector                            */
         simargs = (char **) malloc(2 * sizeof(char *));
         simargs[1] = NULL;
      }

      if (simargs[0]) free(simargs[0]);        /* Change the program name             */
      simargs[0] = StrDup("YS__Interactive");  /*   to "YS__Interactive               */

      if ((sim_desc[1] = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) { /* Create a socket   */
         PrintMsg("Socket open failed");
         exit(errno);
      }

      if (connect(sim_desc[1], &server, sizeof(struct sockaddr_un)) < 0) { 
         PrintMsg("Connect failed");
         exit(errno);
      }

      if (dup2(sim_desc[1], SIMIO) < 0) {
         PrintMsg("Dup2 failed");
         exit(errno);
      }
      close(sim_desc[0]);

      execvp(simfile, simargs); /* Switch to the simulation program & start executing */
      PrintMsg("Exec failed for file %s",simfile);
      exit(errno);
   }

   else { /* This code is executed by the forking parent                              */

      if ((sim_desc[0] = accept(fd, 0, 0)) < 0) { /* Accept simulation's connection   */
         PrintMsg("Accept failed");
         exit(errno);
      }

      DB__SimIO = sim_desc[0];
      close(sim_desc[1]);
  
      cmd = ACK;
      write(DB__SimIO, &cmd, 1);     /* Synchronize with the child process            */

      cmd = '\000';
      while (cmd != ACK) {           /* Wait for reply by child process               */
          result = read(DB__SimIO, &cmd, 1);
          if (result == 0) {
             PrintMsg("Simulatin exited with status %d",SimDone());
             return;
          }
      }

      DB__SimTime = 0.0;
      unlink(server.sun_path);       /* no need for the temporary file now            */
 
      return 0;
   }
}

/**************************************************************************************/

static void PrintMsg(va_alist)  /* Like prinf, but prepends the program name          */
va_dcl
{
   va_list ap;
   char *fmt, *p, *sval;
   int ival;

   printf("\n");                          /* Skip a line                              */
   printf("    %s: ",prog);               /* Indent & rint the program name           */

   va_start(ap);                          /* Point to first unnamed argument          */
   fmt = va_arg(ap, char*);               /* The first argument is a format string    */

   for (p=fmt; *p; p++) {                 /* Parse the format string                  */
      if (*p!='%') {                      /* Plain character                          */
         putchar(*p);                     /* Print it                                 */
      }     
      else {                              /* Parameter encountered in format string   */
         switch (*++p) {                  /* Switch on the next character after %     */
            case 'd':                     /* Parameter is an integer                  */
               ival = va_arg(ap, int);    /* Get it and convert it to an int value    */
               printf("%d", ival);        /* Print it to the terminal                 */
               break;
            case 's':                     /* Parameter is a character string          */
               for (sval = va_arg(ap, char *); *sval; sval++)  /* Get it              */
                  putchar(*sval);         /* Print it to the terminal                 */
               break;
            default:                      /* Only formats recognized are d and s      */
               putchar(*p);               /* All others printed as characters         */
               break;                     /* I.e., no format converstion              */
         }
      }
   }
   va_end(ap);
   printf("\n\n");                        /* Skip another line                        */
}

/**************************************************************************************/

static char *GetClearStr()     /* Gets & returns the string that clears the screen    */
{
   int i;
   char term_buffer[1024];
   char **area;

   if (tgetent(term_buffer, getenv("TERM")) > 0) {       /* There is a termcap entry  */
      area = (char**) malloc(sizeof(char*) * 4);         /* Look up clear command     */
      for (i=0; i<4; i++) area[i] = (char *) malloc(16); /* Array needed by tgetstr   */
      return (char*)tgetstr("cl", area);                 /* Access termcap table      */
   }
   else {                                                /* No termcap entry          */
      return NULL;
   }
}

/**************************************************************************************/

static char *StrDup(s)   /* Mallocs space and copies s to it                          */
char *s;                 /* String to copy                                            */
{  /* This function is needed since strdup not available on AUX machines              */
   char *sptr;
   sptr = (char*)malloc(strlen(s)+1);   /* Get the space                              */
   if (sptr == NULL) return NULL;       /* Malloc failed                              */
   strcpy(sptr,s);                      /* Copy s to the new string                   */
   return sptr;                         /* Return pointer to the new string           */
}

/**************************************************************************************\
******************************* Command Interpretation *********************************
****************************************************************************************
**                                                                                    **
**      These routines interpret user commands typed at the terminal.                 **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

static void InitCommandList()    /* Intiializes the command list used by GetUserCmd() */
{
   /* This function loads information into the DB__CmdList array.  Each entry in      */
   /* this array is a command_info struct containing a name field, two description    */
   /* fields and a field that points to the function that must be executed to run the */
   /* command.                                                                        */

   /* Command list entry for the help command */

      DB__CmdList[0].name = "help";
      DB__CmdList[0].func = Help;
      DB__CmdList[0].short_desc = 
         "    help    - display a help screen or get help on a command";
      DB__CmdList[0].long_desc = "\n   help [command]\n\n\
      To get a summary of commands, type 'help' with no arguments.\n\
      To get help on a specific command, type 'help commandname'.\n\n";

   /* Command list entry for the clear command */

      DB__CmdList[1].name = "clear";
      DB__CmdList[1].func = Clear;
      DB__CmdList[1].short_desc = 
         "    clear   - clear the screen";
      DB__CmdList[1].long_desc = "\n   clear\n\n\
      Clear the screen.\n\n";

   /* Command list entry for the mode command */

      DB__CmdList[2].name = "mode";
      DB__CmdList[2].func = Mode;
      DB__CmdList[2].short_desc = 
         "    mode    - display or set the running mode";
      DB__CmdList[2].long_desc = "\n   mode [parameter]\n\n\
      This command specifies the mode that will be used to advance simulation\n\
      time.  It uses the variable STEP which can be changed using the step command.\n\n\
      The valid parameters are:\n\n";
      strcat(DB__CmdList[2].long_desc,"\
         'for time'      - The simulation runs for STEP time units.\n\n\
         'until time'    - The simulation runs until time is >= STEP.\n\
                           If time is already at or past STEP, it does\n\
                           not run at all.\n\n\
         'event changes' - The simulation runs until STEP more events have\n\
                           occurred. STEP is truncated to an integer.\n\n\
         'time changes'  - The simulation runs until STEP more time changes\n\
                           have occurred.  STEP is truncated to an integer.\n\n\
      Since each of the mode names begins with a different character, only the first\n\
      character need be entered when setting the mode.\n\n\
      Executing this command with no arguments displays the current mode.\n\n");

   /* Command list entry for the step command */

      DB__CmdList[3].name = "step";
      DB__CmdList[3].func = Step;
      DB__CmdList[3].short_desc = 
         "    step    - display or set the step size";
      DB__CmdList[3].long_desc = "\n   step [size]\n\n\
      This command sets the variable STEP to the value given by the argument.\n\
      If there is no argument, it displays the current step size.\n\n";
      
   /* Command list entry for the tracelevel command */

      DB__CmdList[4].name = "trace";
      DB__CmdList[4].func = Trace;
      DB__CmdList[4].short_desc = 
         "    trace   - display or set the level of trace reporting";
      DB__CmdList[4].long_desc = "\n   tracelevel [level]\n\n\
      This command sets the trace level to the value given by the argument.\n\
      Only Argument values between 0 and 5 are valid.  Non-integer values\n\
      are truncated to an integer.  If there is no argument given, the current\n\
      trace level is displayed.\n\n";

   /* Command list entry for the Breakpoint command */

      DB__CmdList[5].name = "break";
      DB__CmdList[5].func = Break;
      DB__CmdList[5].short_desc = 
         "    break   - turn breakpoints on or off";
      DB__CmdList[5].long_desc = "\n   break [argument]\n\n\
      This command activates or deactivates the breakpoint capability of DBSIM.\n\
      The parameter may be either 'ignore', which deactivates breakpoint\n\
      interrputs or 'enable' which activates them.  Only the first chacter\n\
      of the parameter is used; all the others are ignored.\n\n";

   /* command list entry for the filter command */

      DB__CmdList[6].name = "filter";
      DB__CmdList[6].func = Filter;
      DB__CmdList[6].short_desc = 
         "    filter  - examine or change the current output filters";
      DB__CmdList[6].long_desc = "\n   filter [filtername] [+|=|-] [tag, ... ]\n\n\
      Filters are groups of tags which are matched with incoming trace data.\n\n\
      To view all current filters, type 'filter' with no arguments.\n\n\
      To see a particular filter, give the name as an argument, but no operator.\n\n\
      To add tags to, remove tags from, or set a filter, use the appropriate\n\
      operator (+, -, or =, respectively). Tags are specified in a comma- or\n\
      whitespace-separated list. No spaces are allowed, but the asterisk (*)\n\
      is allowed as a wildcard character on either end of the tag.\n\n";
      strcat(DB__CmdList[6].long_desc,"\
      If an operation is specified but no filter name is given, that operation\n\
      will take place on the default (stdout) filter. Matches with this filter\n\
      are printed on the screen. All other filter names specify files.  If a \n\
      file does not exist, it will be created and matching data will be placed \n\
      in that file. If the file exists, it will be over written.\n\n\
      Filters remain set as different simulations are loaded.\n\n");

   /* Command list entry for the load command */
 
      DB__CmdList[7].name = "load";
      DB__CmdList[7].func = Load;
      DB__CmdList[7].short_desc = 
         "    load    - load a simulation";
      DB__CmdList[7].long_desc = "\n   load filename [args]\n\n\
      Load the given filename. If any args exist, they are passed as\n\
      command line arguments to the simulation program.  If the load\n\
      is successful, the prompt will change to the name of the simu-\n\
      lation program, indicating that the simulation is ready to run.\n\n";

   /* Command list entry for the execute command */

      DB__CmdList[8].name = "execute";
      DB__CmdList[8].func = Exec;
      DB__CmdList[8].short_desc = 
         "    execute - start or continue executing a simulation";
      DB__CmdList[8].long_desc = "\n   execute\n\n\
      The simulation executes according to the mode and step specified by \n\
      the 'mode' and 'step' commands. If a simulation is active, a carriage \n\
      return can be substituted for this command.\n\n";

   /* Command list entry for the reset command */

      DB__CmdList[9].name = "reset";
      DB__CmdList[9].func = Reset;
      DB__CmdList[9].short_desc = 
         "    reset   - restart the current simulation";
      DB__CmdList[9].long_desc = "\n   reset\n\n\
      Restart the current simulation.  Any arguments sent to the \n\
      simulation with the last 'load' command will be reused.\n\n";

   /* Command list entry for the kill command */

      DB__CmdList[10].name = "kill";
      DB__CmdList[10].func = Kill;
      DB__CmdList[10].short_desc = 
         "    kill    - close the current simulation";
      DB__CmdList[10].long_desc = "\n   kill\n\n\
      Stop the current simulation.  A new simulation can then be started \n\
      with the 'load' command.\n\n";

   /* Command list entry for the quit command */

      DB__CmdList[11].name = "quit";
      DB__CmdList[11].func = Quit;
      DB__CmdList[11].short_desc = 
         "    quit    - exit dbsim";
      DB__CmdList[11].long_desc = "\n   quit\n\n\
      Shut down the simulation and exit DBSIM.\n\n";
}


/**************************************************************************************/

static int Help(buf)          /* Prints help information on the commands              */
char *buf;                    /* Points to the Name of the command                    */
{
  int i=0, j;

  while(isspace(buf[i])) i++;        /* Skip over white space in buf[]               */
  while(isalpha(buf[i])) i++;        /* Skip over characters in buf[]                */
  while(isspace(buf[i])) i++;        /* Skip over white space in buf[]               */

  if (buf[i] == '\0') {              /* No command specified, show info on all        */
    printf("\n");
    for (i=0; i<NUM_COMMANDS; i++)   /* Print a short description of every command    */
      printf("%s\n",DB__CmdList[i].short_desc);
      printf("\n");
  }
  else {                             /* A command specified, show detailed info on it */
    for (j=0; j < NUM_COMMANDS && DB__CmdList[j].name[0] != tolower(buf[i]); j++);
    if (j==NUM_COMMANDS) {           /* Specified command is not valid                */
      PrintMsg("No help available for %s", &buf[i]);
      return 1;
    }
    printf(DB__CmdList[j].long_desc); /* Print the detailed description               */
  }
  return 0;
}


/**************************************************************************************/

static int Clear()    /* Clear the screen                                             */
{
  int i;

  if (DB__ClrStr)     /* Terminal has clear command, use it                           */
     puts(DB__ClrStr);
  else                /* No clear command, fake it by outputting some blank lines     */
     for (i=80; i; i--) printf("\n");
  return 0;           /* Command always successful                                    */
}

/**************************************************************************************/

static int Mode(buf)   /* Sets or displays the single stepping mode                   */
char *buf;             /* Buffer that contains the new mode                           */
{
   int i = 0;

   /* skip over the command name */

   while(isspace(buf[i])) i++;      /* Skip over white space                          */
   while(isalpha(buf[i])) i++;      /* Skip over the command name                     */
   while(isspace(buf[i])) i++;      /* Skip over white space                          */

   if (buf[i]=='\0') {    /* If there are no arguments, print the current settings    */
      if (DB__Mode == FOR_TIME) printf("\n   mode = for time\n\n");
      if (DB__Mode == UNTIL_TIME) printf("\n   mode = until time\n\n");
      if (DB__Mode == TIME_CHANGES) printf("\n   mode = time changes\n\n");
      if (DB__Mode == EVENT_CHANGES) printf("\n   mode = event changes\n\n");
      return 0;
   }

   if (tolower(buf[i]) == 'f') {                   /* Execute "for time" mode         */
      DB__Mode = FOR_TIME;
      return 0;                                    /* Command successful              */
   }
   if (tolower(buf[i]) == 'u') {                   /* Execute "until time" mode       */
      DB__Mode = UNTIL_TIME;
      return 0;                                    /* Command successful              */
   }
   if (tolower(buf[i]) == 't') {                   /* Execute "time changes" mode     */
      DB__Mode = TIME_CHANGES;
      return 0;                                    /* Command successful              */
   }
   if (tolower(buf[i]) == 'e') {                   /* Execute "event changes" mode    */
      DB__Mode = EVENT_CHANGES;
      return 0;                                    /* Command successful              */
   }

   /* could not match mode type */
   
   PrintMsg("Unknown mode - not changed");
   return 1;                                       /* Command not successful          */

}

/**************************************************************************************/

static int Step(buf)      /* Sets or displays the current single stepping step size   */
char *buf;                /* Buffer that contains the new step size                   */
{
   int i = 0;
   /* skip over the command name */
   while(isspace(buf[i])) i++;          /* Skip over white space                      */
   while(isalpha(buf[i])) i++;          /* Skip over the command name                 */
   while(isspace(buf[i])) i++;          /* Skip over white space                      */

   if (buf[i]=='\0') {    /* If there are no arguments, print the current settings    */
      printf("\n   step size = %g\n\n",DB__Step);
      return 0;                         /* Command successful                         */
   }

   DB__Step = atof(&buf[i]);
   return 0;                            /* Command successful                         */
}

/**************************************************************************************/

static int Trace(buf)       /* Sets or displays the trace level                       */
char *buf;                  /* Buffer that contains the new trace level               */
{
   int i = 0;

   /* skip over the command name */

   while(isspace(buf[i])) i++;    /* Skip over white space                            */
   while(isalpha(buf[i])) i++;    /* Skip over the command name                       */
   while(isspace(buf[i])) i++;    /* Skip over white space                            */

   if (buf[i]=='\0') {            /* if no arguments, print the current settings      */
      printf("\n   trace level = %d\n\n",DB__TrLevel);
      return 0;                   /* Command successful                               */
   }

   if ((atoi(&buf[i]) < 0) || (atoi(&buf[i]) > 5)) {
     PrintMsg("Tracelevel out of range 0 to 5");
     return 1;                    /* Command not successful                           */
   }

   DB__TrLevel = atoi(&buf[i]);
   return 0;                      /* Command successful                               */
}

/**************************************************************************************/

static int Break(buf)         /* Sets or displays the breakpoint status               */
char *buf;                    /* Buffer containing new breakpoint status              */
{
   int i = 0;

   /* skip over the command name */

   while(isspace(buf[i])) i++;    /* Skip over white space                            */
   while(isalpha(buf[i])) i++;    /* Skip over the command name                       */
   while(isspace(buf[i])) i++;    /* Skip over white space                            */

   if (buf[i]=='\0') {            /* if no arguments, print the current status        */
      if (DB__Break) printf("\n   Breakpoints enabled\n\n");
      else printf("\n   Breakpoints disabled\n\n");
      return 0;                   /* Command successful                               */
   }
   
   if (tolower(buf[i]) == 'i') DB__Break = 0;
   else if (tolower(buf[i]) == 'e') DB__Break = 1;
   else PrintMsg("Invalid breakpoint status; use 'ignore' or 'enable'");
}


/**************************************************************************************/

static int Filter(buf)      /* Sets or displays filters                               */
char *buf;                  /* Buffer containing new filter information               */
{
   char filter_name[FILTER_NAME_LENGTH+1];     /* extra bytes for null terminators */
   char tag[TR_NAME_LENGTH+1];
   filter_info *lptr;
   int (*filter_op)();
   int i=0, k=0;

   while(isspace(buf[i])) i++;        /* Skip over white space in buf[]               */
   while(isalpha(buf[i])) i++;        /* Skip over characters in buf[]                */
   while(isspace(buf[i])) i++;        /* Skip over white space in buf[]               */

   if (buf[i]=='\0') {                /* No arguments -- print current filters        */
      lptr = DB__FilterList;          /* Get a pointer to the filter list             */
      while (lptr) {                  /* Traverse the list                            */
         PrintFilter(lptr->name);     /* Display each filter encourntered             */
         lptr = lptr->next;
      }
      return 0;
   }

   strcpy(filter_name, "stdout");     /* Default filter is stdout                     */

   if (isalpha(buf[i]) || buf[i]=='/') { /* A filter name is given                    */
      for (k=0; (isalnum(buf[i]) || buf[i]=='/') && (k < FILTER_NAME_LENGTH); k++)
         filter_name[k] = buf[i++];      /* Copy the given name to temp buffer        */
      filter_name[k] = '\0';             /* Make sure it is null-terminated           */
   }  /* More characters than FILTER_NAME_LENGTH are lost                             */

   while(isspace(buf[i])) i++;        /* Skip over white space in buf[]               */

   switch(buf[i]) {                   /* Process tag operations                       */
      case '=':                       /* Remove all tags and replace with new ones    */
         DeleteFilter(filter_name);   /* Delete the filter and all its tags           */
         filter_op = AddTag;          /* Set tag operation                            */
         break;                       /* AddTag creates a new filter if needed        */
      case '+':                       /* Add a tag to a filter                        */
         filter_op = AddTag;          /* Set tag operation                            */
         break;
      case '-':                       /* Delete a tag from a filter                   */
         k=i+1;                       /* Skip over the operation symbol               */
         while (isspace(buf[k])) k++; /* And any white space                          */
         if (buf[k]=='\0')            /* No tag name specified, delete the filter     */
            if (DeleteFilter(filter_name) == 0) { /* Found & deleted filter           */
               PrintMsg("Filter %s removed", filter_name); 
               return 0;
            }
            else {                                /* Filter not in the list           */
               PrintMsg("Filter %s not found", filter_name);
               return 1;
            }
         else                                     /* Tag name(s) specified            */
            filter_op = DeleteTag;                /* Set tag operation                */
         break;
      default:                        /* Next character not +, = , or -               */
         if (buf[i]) {                /* Tag, but no operations signal                */
            PrintMsg("Poorly formed filter command");
            return 1;
         }      
         else {                       /* No tags specified                            */
            PrintFilter(filter_name); /* Print all tags in filter                     */
            return 0;
         }
   }

   i++;                               /* Skip over tag operation symbol               */
   while(isspace(buf[i])) i++;        /* Skip over white space in buf[]               */
 
   k = 0;                             /* Find and copy tags to temporary buffer tag   */
   while(buf[i]) {                    /* Process each tag until the end of the list   */
      if (buf[i]==',' || isspace(buf[i])) {   /* Tag separator found                  */ 
         tag[k] = '\0';                       /* Terminate constructed tag            */
         (*filter_op)(filter_name, tag);      /* Apply specified tag operation to it  */
         k=0;                                 /* Start looking for the next tag       */
         while ((buf[i]==',') || isspace(buf[i])) i++;  /* Skip over tag separators   */
      }
      else                            /* Still in the tag                             */
          if (k < TR_NAME_LENGTH)     /* Have not exceeded maximum tag length         */
             tag[k++] = buf[i++];     /* Copy character to temporary buffer           */
          else                        /* Have exceeded maximum tag length             */
             i++;                     /* Skip extra characters                        */
   }
   if (k > 0) {                       /* Last tag not yet processes                   */
      tag[k] = '\0';                  /* Terminate it                                 */
      (*filter_op)(filter_name, tag); /* Apply specified tag operation to it          */
   }

   return 0;
}

/**************************************************************************************/

static int Load(buf)                   /* Load a new simulation program               */
char *buf;                             /* Name of program and its arguments           */
{
   int i, j=0, k=0, simargc = 0, charcnt;
   char arg[30];

   if (DB__SimPtr != 0) {                /* A simulation is running                   */
      PrintMsg("A simulation is already running; use the kill command to stop it");
      return 1;                          /* Command not successful                    */
   }

   i = 0;                                /* Start with the first character of buf     */
   while(isspace(buf[i])) i++;           /* Skip over white space in buf[]            */
   while(isalpha(buf[i])) i++;           /* Skip over command in buf[]                */
   while(isspace(buf[i])) i++;           /* Skip over white space in buf[]            */

   if (buf[i] == '\0') {                 /* Empty buffer                              */
      PrintMsg("No simulation program specified");
      return 1;                          /* Command not successful                    */
   }

   /* Create argv vector to pass to execvp */

   simargc = 0;                         /* Determine the number of arguments          */
   j = i;                               /* Remember starting position of 1st argument */
   while (buf[i]) {                     /* String in buf is NULL terminated           */
      simargc++;                        /* Found another argument                     */
      while (!isspace(buf[i]) && buf[i]) i++;   /* Skip over the the argument         */
      while (isspace(buf[i])) i++;              /* Skip over following white space    */
   }
   i = j;                               /* Restore the starting position              */

   if (DB__Argv != NULL)                /* A New argument vector will be constructed  */
      free(DB__Argv);                   /* Free the old one                           */
   DB__Argv = (char**)malloc( (simargc+1)*sizeof(char*) ); /* Allocate the new one    */

   k = 0;                               /* Copy arguments to DB__Argv vector          */
   for (j=0; j < simargc; j++) {        /* There will be simargc-1 of them            */
      while ( buf[i] != '\0' && !isspace(buf[i]) )  /* Build the jth one in arg       */
         arg[k++] = buf[i++];        
      arg[k]='\0';                                  /* Terminate it                   */
      k=0;
      DB__Argv[j] = StrDup(arg);         /* Copy to DB__Argv from the temp vector arg */
      while(isspace(buf[i])) i++;        /* Skip over white space in buf[]            */
   }
   DB__Argv[j] = NULL;                   /* execvp needs the last arg to be NULL      */
   if (SimStart(DB__Argv)) {             /* Load and execute the simulation           */
      free(DB__Argv);
      DB__Argv = NULL;
      PrintMsg("Unable to load simulation; invalid simulation file");
      return 1;                          /* Command not successful                    */
   }
   return 0;                             /* Command successful                        */ 
}

/**************************************************************************************/

static int Exec() /* Execute one more step as deterimined by "mode" & "step" commands */
{
   char cmd;

   if (DB__SimPtr == 0) {                         /* No simulation loaded             */
      PrintMsg("No simulation loaded");
      return 1;                                   /* Command not successful           */
   }

   cmd = RUN;
   write(DB__SimIO, &cmd, 1);                     /* Send command to YACSIM Driver    */
   write(DB__SimIO, &DB__Mode, sizeof(DB__Mode)); /* Send parameters for RUN command  */
   write(DB__SimIO, &DB__Step, sizeof(DB__Step));
   write(DB__SimIO, &DB__TrLevel, sizeof(DB__TrLevel));
   write(DB__SimIO, &DB__Break, sizeof(DB__Break));
   return 0;                                      /* Command successful               */
}

/**************************************************************************************/

static int Reset()                 /* Reset the current simulation, if there is one   */
{
   if (DB__Argv)                   /* There is a currently loaded simulation          */
      return (SimStart(DB__Argv)); /* SimStart kills it and then restarts it          */
   else {
      PrintMsg("No simulation loaded");
      return 1;                    /* Command not successful                          */
  }
}

/**************************************************************************************/

static int Kill()
{
   if (DB__SimPtr == 0) {
      PrintMsg("No simulation running");
      return 1;
   }

   if (kill(DB__SimPtr, SIGQUIT) < 0 && kill(DB__SimPtr, SIGTERM) < 0) {
      PrintMsg("Unable to kill simulation");
      return 1;
   }
   free(DB__Argv);
   DB__Argv = NULL;        
   DB__SimTime = 0.0;     
   SimDone();
   return 0;
}

/**************************************************************************************/

static int Quit()
{
   if (DB__SimPtr != 0) {     /* There is a simulation running                        */
      if (kill(DB__SimPtr, SIGQUIT) < 0 && kill(DB__SimPtr, SIGTERM) < 0) {
         PrintMsg("Unable to kill simulation");
         return 1;            /* Command not successful                               */
      }
      SimDone();
   }
   exit(0);
}

/**************************************************************************************\
********************************* Filter Routines **************************************
****************************************************************************************
**                                                                                    **
**              These routines perform the filtering operations.                      **
**                                                                                    **
****************************************************************************************
\**************************************************************************************/

static void InitFilter()       /* Initializes the filter list                         */
{
   DB__FilterList = NULL;       /* No filters initially                               */
   AddTag("stdout", "times");   /* This Creates filter stdout and adds times tag      */
}

/**************************************************************************************/

static int AddTag(filter_name, tag) /* Adds a tag to a filter, creating it if needed  */ 
char *filter_name;                  /* Pointer to the name of the filter to add to    */
char *tag;                          /* Pointer to the name of the tag to add          */
{
   filter_info *lptr;
   tag_info *iptr;

   lptr = DB__FilterList;           /* Get a pointer to the head of the filter list   */
   while ((lptr) && strcmp(filter_name, lptr->name)) /* Look for the specified filter */
      lptr = lptr->next;

   if (lptr==NULL) {                /* Filter not in the list; create it              */
      lptr = (filter_info*) malloc(sizeof(filter_info));  /* Allocate it              */
      lptr->name = StrDup(filter_name);      /* Copy filter name to filter structure  */
      lptr->tags = NULL;                     /* No tags in the filter yet             */
      lptr->prev = NULL;                     /* Put filter at head of filter list     */
      lptr->next = DB__FilterList;
      if (lptr->next)
         lptr->next->prev = lptr;
      DB__FilterList = lptr;

      if (strcmp(filter_name, "stdout")) {    /* Filter name NOT equal "stdout        */
         lptr->file = fopen(lptr->name, "w"); /* Open a filter file                   */
         if (lptr->file) {                    /* File successfully created            */
            /* Prepend a header to the file; not implemented yet */
         }
         else {                               /* File created fails                   */
            PrintMsg("Unable to open file %s - filter will be ignored", filter_name);
            return 1;
         }
      }
      else                                    /* Filter name is equal "stdout         */
         lptr->file = NULL;                   /* No file needed for the filter        */
   }

   iptr = (tag_info *) malloc(sizeof(tag_info));  /* Get a tag pointer                */
   iptr->name = StrDup(tag);                  /* Copy the tag name                    */
   iptr->prev = NULL;                         /* Put tag at head of filter's tag list */
   iptr->next = lptr->tags;
   if (iptr->next)
      iptr->next->prev = iptr;
   lptr->tags = iptr;

   return 0;
}

/**************************************************************************************/

static int DeleteTag(filter_name, tag) /* Deletes a tag from a filter                 */
char *filter_name;                     /* Pointer to the filter to delete from        */
char *tag;                             /* Pointer to the tag to delete                */
{
   filter_info *lptr;
   tag_info *iptr;

   lptr = DB__FilterList;              /* Get pointer to the head of the filter list  */
   while ((lptr) && strcmp(filter_name, lptr->name)) /* Look for the specified filter */
      lptr = lptr->next; 

   if (lptr) {                         /* Found filter in the list                    */
      iptr = lptr->tags;               /* Get a pointer to the filter's tag list      */
      while ((iptr) && strcmp(tag, iptr->name))  /* Look for the specified tag        */
         iptr = iptr->next;

      if (iptr) { /* Found it in the filter's list of tags; take it out of the list   */
         if (iptr->prev)               /* Tag not at head of list                     */
            iptr->prev->next = iptr->next; 
         else                          /* Tag at head of list                         */
            lptr->tags = iptr->next;
         if (iptr->next)               /* Tag not at tail of list                     */
            iptr->next->prev = iptr->prev;
         free(iptr->name);             /* Delete tag name                             */
         free(iptr);                   /* and tag structure                           */

         if (lptr->tags==NULL) {       /* This was the last tag -- delete the filter  */
            if (lptr->prev)            /* Filter not at head of list                  */
               lptr->prev->next = lptr->next;
            else                       /* Filter at head of list                      */
               DB__FilterList = lptr->next; 
            if (lptr->next)            /* Filter not at tail of list                  */
               lptr->next->prev = lptr->prev;
            free(lptr->name);          /* Delete the filter name                      */
            free(lptr);                /* and the filter structure                    */
         }
         return 0;
      }
   }

   return 1;                           /* either filter or tag was not found          */
}

/**************************************************************************************/

static int DeleteFilter(filter_name)   /* Delete a filter and all its tags            */
char *filter_name;                     /* Pointer to name of filter to delete         */
{
  filter_info *lptr;
  tag_info *iptr;
  tag_info *last;

   lptr = DB__FilterList;              /* Get a pointer to the list of filters        */
   while ((lptr) && strcmp(filter_name, lptr->name))  /* Look for the filter in list  */
      lptr = lptr->next;

   if (lptr) {                         /* Found it in the list                        */
      iptr = lptr->tags;               /* Get a pointer to the filter's tag list      */
      while (iptr) {                   /* Delete all of them                          */
         last = iptr;
         iptr = iptr->next;
         free(last->name);
         free(last);
      }

      if (lptr->prev)                  /* Now delete the filter itself                */
         lptr->prev->next = lptr->next;
      else
         DB__FilterList = lptr->next;
      if (lptr->next)
         lptr->next->prev = lptr->prev;
      free(lptr->name);
      free(lptr);

      return 0;
   }

   return 1;                           /* The filter was not found                    */
}

/**************************************************************************************/

static int PrintFilter(filter_name)     /* Prints the filter and its tags             */
char *filter_name;                      /* Pointer to the name of filter to print     */
{
  filter_info *lptr;
  tag_info *iptr;

   lptr = DB__FilterList;               /* Get a pointer to the list of filters       */
   while ((lptr) && strcmp(filter_name, lptr->name)) /* Look for the specified filter */
      lptr = lptr->next;       

   if (lptr) {                          /* Found it in the list                       */
      printf("filter name: %s\n       tags: ", filter_name);
      for (iptr = lptr->tags; (iptr); iptr = iptr->next) { /* Print each tag          */
         printf("%s", iptr->name);
         if (iptr->next) printf(", ");
      }
      printf("\n");
      return 0;
   }

  return 1;                             /* The filter was not found                   */
}

/**************************************************************************************/

static void DoFilter(buf)  /* Filter trace packets to approtiate filter file          */
char *buf;                 /* Pointer to the trace packet tag                         */
{
   filter_info *lptr;
   tag_info *iptr;

   if (*buf=='\0') /* If the tag is empty, print the trace data to the terminal       */
      printf("%s", &buf[TR_NAME_LENGTH+1]); /* Start printing after the packet's tag  */

   /* currently a linear search of all tags in all filters */

   lptr = DB__FilterList;      /* Get a pointer to the list of filters                */
   while (lptr) {              /* Check them all                                      */
      iptr = lptr->tags;       /* Get a pointer to the filter's list of tags          */
      while ((iptr) && (!Compare(buf, iptr->name))) /* Look for the specified tag     */
         iptr = iptr->next;
      if (iptr) {              /* Found it                                            */
         if (!strcmp(lptr->name, "stdout") && (*buf!='\0'))  /* Filter is stdout      */
            printf("%s", &buf[TR_NAME_LENGTH+1]);            /* Print to terminal     */
         else
            if (lptr->file)                                  /* Print to a file       */
               fwrite(&buf[TR_NAME_LENGTH+1],                /* Skip the tag          */
                      strlen(&buf[TR_NAME_LENGTH+1]), 1, lptr->file);
      }
      lptr = lptr->next;       /* Get the next filter, we have to check them all      */
   }
}

/**************************************************************************************/

static int Compare(name, tag)   /* Compare tag names to filter tag                    */
char *name;                     /* Name of packet's tag                               */
char  *tag;                     /* Filter tag to compare it with                      */
{
   while (*name && *tag) {      /* More characters left in both tags                  */
      if (*name==*tag) {        /* Corresponding characters agree                     */
         name++; tag++;         /* Bump the pointers                                  */
      }
      else if (*tag=='*') {     /* Tag has a wildcard character in it                 */
         tag++;                 /* Skip over the wildcard character                   */
         while (*name!=*tag) name++;  /* And look for a match after the wildcard      */
      }  /* Note: this is not a true wild card matching algorithm                     */
      else                      /* Tags do not match                                  */
         return 0;
   }
   return 1;
}

/**************************************************************************************/
