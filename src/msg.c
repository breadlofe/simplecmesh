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
#include "../incl/tr.msg.h"
#include "../incl/tr.net.h"
#include "malloc.h"

/**************************************************************************************/

void SetPacketSize(sz)
int sz;
{
   if (sz > 0) YS__PacketSize = sz;
   else YS__errmsg("Packet size must be > 0");
}

/**************************************************************************************/

void ProcessorSendMsg(dest,buf,bytes,blkflg,type,pktsz) /* Sends a msg to a processor */
int dest;                             /* ID of the destination process                */
char *buf;                            /* Pointer to the message contents              */
int bytes;                            /* Size of the message contents                 */
int blkflg;                           /* NOBLOCK or one of the blocking modes         */
int type;                             /* User defined type                            */
int pktsz;                            /* LONG or SHORT packet size                    */
{
   MESSAGE   *mptr;
   char      *charptr;
   int       i;
   PACKET    *packet;
   int       port;
   PROCESSOR *pr;
   
   PSDELAY;
   
   mptr            = (MESSAGE*)YS__PoolGetObj(&YS__MsgPool);
   if (YS__ActProc && YS__ActProc->pp)         /* Called from an attached process     */
      mptr->source = (ACTIVITY*)YS__ActProc;
   else
      YS__errmsg("Only attached processes can call ProcessorSendMsg()");
   mptr->srccpu    = YS__ActProc->pp->id;
   mptr->id        = YS__msgcounter++;         /* System defined unique message ID    */
   mptr->next      = NULL;
   mptr->receiver  = (PROCESS*)dest;           /* Receiver is declared as PROCESS*    */
   mptr->msgtype   = type;
   mptr->pktsz     = pktsz;
   mptr->sendtime  = YS__Simtime;
   mptr->blockflag = blkflg;
   mptr->destflag  = PROCESSORDEST;            /* Send to a process                   */
   TRACE_PROCESS_sendmsg2;  /* Process sends message to process ...                   */

   if (buf != NULL && bytes != 0) { /* Buffer the message data                        */
      mptr->bufptr = (char *)malloc(bytes);
      if (mptr->bufptr == NULL) YS__errmsg("Malloc fails in ProcessSendMsg");
      charptr = mptr->bufptr;
      for (i = 0; i<bytes; i++) *charptr++ = *buf++;  /* Copy the data to buffer      */
      mptr->msgsize = bytes;
   }
   else {
      mptr->msgsize = 0;
      mptr->bufptr = NULL;
   }

   pr = mptr->source->pp;
   if (pr == NULL)
      YS__errmsg("Sender must be attached to a processor to call ProcessorSendMsg");

   if (pr->id == (int)mptr->receiver) {  /* Sender and receiver on the same processor */
      YS__QueuePutTail(pr->localmsgs,mptr);      /* Queue the message                 */
      if (pr->localsender != NULL) {             /* Sender idle                       */
         YS__RdyListAppend(pr->localsender);     /* Wake it up                        */
         pr->localsender = NULL;
      }
   }
   else { /* Must use the network to deliver the message                              */
      if (pr->router)  /* More than one network input port on this processor          */
         port = (pr->router)(pr->id, mptr->receiver, pr->id);
      else
         port = 0;
      YS__QueuePutTail((pr->iportmsg)[port],mptr);  /* Add msg to port's msg list     */
      if (pr->iportevt[port] != NULL) {             /* Port Sender idle               */
         YS__RdyListAppend((pr->iportevt)[port]);   /* Wake it up                     */
         pr->iportevt[port] = NULL;
      }
   }
   if (blkflg == NOBLOCK) return;                   /* Sender does not block          */
   else YS__Suspend();
}

/**************************************************************************************/

int ProcessorReceiveMsg(buf, bytes, blockflg, typ, sender) 
                             /* Copies received data into buf and returns its size    */
char *buf;                   /* Pointer to data buffer                                */
int bytes;                   /* maximum number of bytes to receive                    */
int blockflg;                /* BLOCK or NOBLOCK                                      */
int typ;                     /* Receive only this type                                */
int  sender;                 /* Receive from this processor only                      */
{
   MESSAGE *mptr;
   char *mptr1;
   int i;
   int bytestomove;
   double delay;

   PSDELAY;

   if (YS__ActProc == NULL || YS__ActEvnt != NULL)
      YS__errmsg("ProcessReceiveMsg() can only be invoked from within a process body");

   while (1)  {
/*      TRACE_PROCESS_msgrecv1;    /* Process checks for incoming message of given type */
      for( mptr = (MESSAGE*)(YS__QueueNext(YS__ActProc->pp->MsgList,NULL)); 
           mptr != NULL;
           mptr = (MESSAGE*)(YS__QueueNext(YS__ActProc->pp->MsgList,mptr)) )  {
              if ( (mptr->msgtype == typ || typ == ANYTYPE) && 
                   (mptr->srccpu == sender || sender == ANYSENDER) ) {
                 TRACE_PROCESS_msgrecv2;          /* Message available                */
                 mptr1 = mptr->bufptr;            /* Copy the message contents        */
                 if (bytes < 0 ) {
                    bytestomove = 0;
                     YS__warnmsg("Trying to receive a negative number of bytes");
                 }
                 else if (bytes < mptr->msgsize) bytestomove = bytes;
                 else bytestomove = mptr->msgsize;
                 if (bytestomove > 0 && buf == NULL) 
                    YS__errmsg("ProcessReceiveMsg() has a null receive buffer");
                 TRACE_PROCESS_msgrecv4;          /* Receiving message                */
                 if (mptr1 != NULL) {
                    for (i = 0; i < bytestomove; i++) *buf++ = *mptr1++;
                    delay =   YS__OSDelays[MSG_DELIVER_DELAY][0]
                            + YS__OSDelays[MSG_DELIVER_DELAY][1]*bytestomove;
                    if (delay > 0.0)
                       ProcessDelay(delay);  /* Delay for time to deliver data        */
                 }
                 if (bytestomove < mptr->msgsize) {
                    YS__warnmsg("Not all message bytes received");
                    mptr->msgsize = mptr->msgsize - bytestomove;
                    return -2;
                 }
                 YS__QueueDelete(YS__ActProc->pp->MsgList,mptr);
                 if (mptr->source && mptr->blockflag == BLOCK) { /* Sender blocked */
                    TRACE_PROCESS_msgrecv3;    /* Blocked sending process released */
                    YS__RdyListAppend(mptr->source);
                 }
                 if (mptr->bufptr != NULL) free(mptr->bufptr);
                 YS__PoolReturnObj(&YS__MsgPool,mptr);
                 return bytestomove;    
              }
      }
      /* No available message matches type and sender or none available */
      if (blockflg == NOBLOCK)  {
         TRACE_PROCESS_msgrecv5;   /* No messagess available; non-blocking receive,   */
         return -1;                 /* process continues                               */
      }
      TRACE_PROCESS_msgrecv6;                /* No messages available;                */
      YS__ActProc->waitmsgtype = typ;  /* blocking receive, process suspends    */
      YS__ActProc->waitmsgsrc = (PROCESS*)sender;
      YS__ActProc->status = WAIT_MSG;
      if (YS__ActProc->statptr)               /* Collecting process statistics         */
         StatrecUpdate(YS__ActProc->statptr,(double)WAIT_MSG,YS__Simtime);
      YS__QueuePutTail(YS__ActProc->pp->WaitingProcesses,YS__ActProc);
      YS__Suspend();                         /* Waiting for a message                 */
   }
}

/**************************************************************************************/

int ProcessorCheckMsg(typ, sender) /* Checks for a message of given type and sender   */
int typ;                           /* Type to check for                               */ 
int sender;                        /* Sender to check for                             */
{
   MESSAGE *mptr;

   PSDELAY;

   if (YS__ActProc == NULL || YS__ActEvnt != NULL)
      YS__errmsg("ProcessCheckMsg() can only be invoked from within a process body");
   TRACE_PROCESS_msgchk1;    /* Process checks for incoming message of given type */
   for( mptr = (MESSAGE*)YS__QueueNext(YS__ActProc->pp->MsgList,NULL); 
        mptr != NULL;
        mptr = (MESSAGE*)YS__QueueNext(YS__ActProc->pp->MsgList,mptr) )  {
      if ( (mptr->msgtype == typ || typ == ANYTYPE) && 
           (mptr->srccpu == sender || sender == ANYSENDER) )  {
         TRACE_PROCESS_msgchk2;
         return mptr->msgsize;
      }
   }
   TRACE_PROCESS_msgchk3;   return -1;
}

/**************************************************************************************/

void YS__PacketSender()
{
   PACKET *packet;
   MESSAGE *mptr;
   PROCESSOR *pr;
   int port;
   int pkts;
   double delay;
   PROCESS *pptr;
   int localmsg;

   port = ActivityArgSize(ME);
   mptr = (MESSAGE *)ActivityGetArg(ME);
   pr = YS__ActEvnt->pp;
   switch (EventGetState()) {

      case 0:  /* Event idle state */

         if (port < 0)  {     /* This event is the local packet sender                */
            mptr = (MESSAGE *)YS__QueueGetHead(pr->localmsgs);
            localmsg = 1;
         }
         else {               /* This event is the packet sender for a port           */
            mptr = (MESSAGE *)YS__QueueGetHead(pr->iportmsg[port] );
            localmsg = 0;
         }
         ActivitySetArg(ME,mptr,port);   /* Remember the message                      */
         if (mptr == NULL) {             /* No more messages to send                  */
            if (port < 0)
               pr->localsender = YS__ActEvnt;
            else 
               pr->iportevt[port] = YS__ActEvnt;
            EventSetState(ME, 0);        /* Sleep until another message ready to send */
            return;
         }

         if (localmsg)
            delay =   YS__OSDelays[MSG_BUFOUT_DELAY][0]
                    + YS__OSDelays[MSG_BUFOUT_DELAY][1]*(mptr->msgsize);
         else 
            delay =   YS__OSDelays[MSG_ROUTING_DELAY][0]
                    + YS__OSDelays[MSG_BUFOUT_DELAY][0]
                    + YS__OSDelays[MSG_BUFOUT_DELAY][1]*(mptr->msgsize);
       
         EventReschedTime(delay,1);    /* Delay for time to buffer message            */
         return;

      case 1:  /* Deliver message if to same processor, else packetize it */
         if (mptr->blockflag == BLOCK_UNTIL_BUFFERED)
            YS__RdyListAppend(mptr->source);        /* Release blocked sender         */

         if (mptr->destflag == PROCESSDEST) {
            if ( pr == mptr->receiver->pp) {           /* Sending to the same processor  */
               YS__QueuePutTail(mptr->receiver->MsgList, mptr);
               if ( mptr->receiver->status == WAIT_MSG
                    && (mptr->msgtype == mptr->receiver->waitmsgtype ||
                        mptr->receiver->waitmsgtype == ANYTYPE)
                    && (mptr->source == (ACTIVITY*)(mptr->receiver->waitmsgsrc) || 
                        mptr->receiver->waitmsgsrc == ANYSENDER)
                  )
                  YS__RdyListAppend(mptr->receiver);
               if (   mptr->blockflag == BLOCK_UNTIL_PACKETIZED 
                   || mptr->blockflag == BLOCK_UNTIL_SENT       )
                  YS__RdyListAppend(mptr->source);        /* Release blocked sender      */
               EventReschedTime(0.0, 0); /* Start processing the next message            */
               return;
            }
            else {                                /* Sending to a different processor    */
               pkts = YS__PacketizeMessage(mptr);
               delay =   YS__OSDelays[MSG_PACKETIZE_DELAY][0] 
                       + YS__OSDelays[MSG_PACKETIZE_DELAY][1]*mptr->pktsz
                       + YS__OSDelays[MSG_PACKETIZE_DELAY][2]*pkts;
               EventReschedTime(delay,2); /* Delay for time to packetize the message     */
               return;
            }
         }
         else {  /* Sending message to a processor                                    */
            if ( pr->id == (int)(mptr->receiver)) { /* Sending to the same processor  */
               YS__QueuePutTail(pr->MsgList, mptr); /* Deliver message                */
               for( pptr = (PROCESS*)(YS__QueueNext(pr->WaitingProcesses,NULL)); 
                    pptr != NULL;
                    pptr = (PROCESS*)(YS__QueueNext(pr->WaitingProcesses,pptr)) )  {
                  if ( (mptr->msgtype == pptr->waitmsgtype ||
                           pptr->waitmsgtype == ANYTYPE)
                       && (mptr->srccpu == (int)(pptr->waitmsgsrc) || 
                           pptr->waitmsgsrc == ANYSENDER)
                     ) 
                  {
                     YS__QueueDelete(pr->WaitingProcesses,pptr);
                     YS__RdyListAppend(pptr);
                     pptr = NULL;
                  }
                  if (   mptr->blockflag == BLOCK_UNTIL_PACKETIZED
                      || mptr->blockflag == BLOCK_UNTIL_SENT  )
                     YS__RdyListAppend(mptr->source);        /* Release blocked sender      */
               }
               EventReschedTime(0.0, 0); /* Start processing the next message         */
               return;
            }
            else {                                /* Sending to a different processor */
               pkts = YS__PacketizeMessage(mptr);
               delay =   YS__OSDelays[MSG_PACKETIZE_DELAY][0] 
                       + YS__OSDelays[MSG_PACKETIZE_DELAY][1]*mptr->pktsz
                       + YS__OSDelays[MSG_PACKETIZE_DELAY][2]*pkts;
               EventReschedTime(delay,2); /* Delay for time to packetize the message  */
               return;
            }
         }
      case 2:  /* Send the packets */
         if (mptr->blockflag == BLOCK_UNTIL_PACKETIZED )
            YS__RdyListAppend(mptr->source);        /* Release blocked sender      */
         while (mptr->packets != NULL)	{
            if (PacketSend(mptr->packets, pr->iports[port]) == 0.0) {
               EventReschedSema (IPortSemaphore(pr->iports[port]), 2);
               return;
	    }
            mptr->packets = mptr->packets->next;
         }
/*         if (mptr->blockflag == BLOCK_UNTIL_SENT || mptr->blockflag == BLOCK)*/
         if (mptr->blockflag == BLOCK_UNTIL_SENT)
            YS__RdyListAppend(mptr->source);          /* Release blocked sender      */
         EventReschedTime(0.0, 0);  /* Start processing the next message             */
         return;

      default:

         YS__errmsg("Wrong Event state in PacketSender");
   }
}

/**************************************************************************************/

static int YS__PacketizeMessage(mptr)
                                  /* Breaks msg into packets & returns # of packets   */
MESSAGE *mptr;                    /* Pointer to the message                           */
{
   int flitcount = 0;             /* Count of flits put into packets                  */
   int seqno = 0;                 /* Count of packets created                         */
   PACKET *lastpacket = NULL;     /* Packet queue tail, head is mptr->packets         */
   PACKET *tmppacket;

   do {
      if (mptr->destflag == PROCESSDEST)
         tmppacket =
            NewPacket(seqno++,mptr,mptr->pktsz,mptr->srccpu,mptr->receiver->pp->id);
      else 
         tmppacket =
            NewPacket(seqno++,mptr,mptr->pktsz,mptr->srccpu,mptr->receiver);
      tmppacket->next = NULL;
      /* Put new packet in a queue pointed to by mptr->packets                        */
      if (lastpacket == NULL) { /* Queue is empty, put packet at its head             */
         mptr->packets = tmppacket;
         lastpacket = tmppacket;
      }
      else {                    /* Queue not empty, put packet at its tail            */
         lastpacket->next = tmppacket;
         lastpacket = tmppacket;
      }
      flitcount = flitcount + mptr->pktsz;
   } while (flitcount < mptr->msgsize);

   mptr->pktorecv = seqno;
   mptr->pktosend = seqno;
   return seqno;                /* Return the number of packets created               */
}

/**************************************************************************************/

void YS__PacketReceiver()
{  
   OPORT *oport;
   PROCESSOR *prptr;
   PACKET *pkt;
   IPORT  *iport;
   MESSAGE *mptr;
   int   i;
   int portindex;
   double delay;
   int lastpkt;
   PROCESS *pptr;

   portindex = ActivityArgSize(ME);
   prptr = YS__ActEvnt->pp;
   pkt = (PACKET*)ActivityGetArg(ME);
   oport = prptr->oports[portindex];
   lastpkt = 0;

   switch (EventGetState()) {

      case 0:
         pkt = PacketReceive(oport);
         if (pkt == NULL) {
            EventReschedSema (oport->pktavail, 0);
            return;
         }

         mptr = pkt->data.mesgptr;

         if (mptr->destflag == PROCESSDEST) { /* Received message sent to a process   */
            if (prptr == mptr->receiver->pp) { /* Packet has arrived at destination   */
               mptr->pktorecv--;
	       if (mptr->msgtype == MSGACK) {  /* The msg is made of exactly one pkt */
		  YS__RdyListAppend(mptr->receiver);
		  YS__PoolReturnObj(&YS__PktPool,pkt);
		  EventReschedTime(0.0, 0);   /* Wait for another message          */
		  return;
	       }
	       else {
		  if (mptr->pktorecv == 0)  {  /* All of msg's packets have arrived */
		     lastpkt = 1;
		     YS__QueuePutTail(mptr->receiver->MsgList, mptr); /* Deliver msg  */
		     if ( mptr->receiver->status == WAIT_MSG
			  && (mptr->msgtype == mptr->receiver->waitmsgtype ||
			      mptr->receiver->waitmsgtype == ANYTYPE)
                          && (mptr->source == (ACTIVITY*)(mptr->receiver->waitmsgsrc) || 
			      mptr->receiver->waitmsgsrc == ANYSENDER)
			)
			YS__RdyListAppend(mptr->receiver);
		  }  /* Return arrived pkt to pool and wait for others irrespective of
			whether this is the last pkt or not			  */
		  YS__PoolReturnObj(&YS__PktPool,pkt);
		  delay =   YS__OSDelays[MSG_BUFIN_DELAY][0]
			  + YS__OSDelays[MSG_BUFIN_DELAY][1]*mptr->pktsz
			  + YS__OSDelays[MSG_BUFIN_DELAY][2]*lastpkt;
		  EventReschedTime(delay, 0); /* Delay for time to buffer pkt      */
		  return;
	       }
	    }  /* Else, pkt has not arrived at dest. Fall through to case 1 */
         }
         else {  /* Received message sent to a processor                              */
            if (prptr->id == (int)(mptr->receiver)) { /* Packet has arrived at dest.  */
               mptr->pktorecv--;
               if (mptr->pktorecv == 0)  {   /* All of message's packets have arrive  */
                  lastpkt = 1;
                  YS__QueuePutTail(prptr->MsgList, mptr); /* Deliver message           */
                  for( pptr = (PROCESS*)(YS__QueueNext(prptr->WaitingProcesses,NULL)); 
                       pptr != NULL;
                       pptr = (PROCESS*)(YS__QueueNext(prptr->WaitingProcesses,pptr))) {
                     if ( (mptr->msgtype == pptr->waitmsgtype ||
                              pptr->waitmsgtype == ANYTYPE)
                          && (mptr->srccpu == (int)(pptr->waitmsgsrc) || 
                              pptr->waitmsgsrc == ANYSENDER)
                        ) 
                     {
                        YS__QueueDelete(prptr->WaitingProcesses,pptr);
                        YS__RdyListAppend(pptr);
                     }
                  }
               }
               YS__PoolReturnObj(&YS__PktPool,pkt);
               delay =   YS__OSDelays[MSG_BUFIN_DELAY][0]
                       + YS__OSDelays[MSG_BUFIN_DELAY][1]*mptr->pktsz
                       + YS__OSDelays[MSG_BUFIN_DELAY][2]*lastpkt;
               EventReschedTime(delay, 0);  /* Delay for time to buffer incoming pkt  */
               return;
            }
         }  /* Else, pkt has not arrived at dest. Fall through to case 1 */  

      case 1:    /* Packet not yet arrived at its destination, reroute it             */
         if (prptr->nooports == 1) i = 0;
         else if (prptr->router != NULL)
            if (mptr->destflag == PROCESSDEST)
               i = (prptr->router)(mptr->srccpu,
                                   mptr->receiver->pp->id,
                                   prptr->id);
            else 
               i = (prptr->router)(mptr->srccpu,
                                   mptr->receiver,
                                   prptr->id);
         else YS__errmsg("Packet misrouted");
         iport = prptr->iports[i];
         if (PacketSend(pkt, iport) == 0.0) {    /* Try to send the packet on         */
            ActivitySetArg(ME, pkt, portindex);  /* Send fails, remember the packet   */
            EventReschedSema(IPortSemaphore(iport), 1);
            return;
         }
         delay =   YS__OSDelays[MSG_ROUTING_DELAY][0]
                 + YS__OSDelays[MSG_PASSTHRU_DELAY][0]
                 + YS__OSDelays[MSG_PASSTHRU_DELAY][1]*mptr->pktsz;
         EventReschedTime(delay, 0);   /* Delay for time to pass packet to oport      */
         return;
   }
}
/**************************************************************************************/
