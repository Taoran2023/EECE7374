#include "../include/simulator.h"
#include <stdio.h>
#include <math.h>
#include<assert.h>
#include<stdbool.h>
#include<stdlib.h>
/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
int seqA, seqB, ACK; 
float interval;     //time-out time
struct Queue buff1; //buffer on A side
struct Queue buff2; //buffer on B side

int moveStep; //free window size
int A_seqA;   //counter of total output packet number from Layer 5 A
struct QueueNode* left_p, *right_p; // slow and fast pointer for left and right boundry of window
//left_p: next packet waiting for ack
//right_p: last packet already sent
typedef struct pkt QDataType;


typedef struct QueueNode{ //linklist Node structure
    QDataType val;
	  float timeStamp;  // transpot starting time for each packet
    struct QueueNode* next;  //pointer for buffer linklist

    int buf_seq;   // store the sequence number
    int check_arr; // check whether packet is arrived, (B received packet or A received ack)

    struct QueueNode* loopnext, *loopbefore;  // the rotation linklist pointer for calculate the next timer, only implement on A side

}QueueNode;

typedef struct Queue{ //linklist structure
    // the head and tail of singly link list for buffer
    QueueNode* head;
    QueueNode* tail;

    // the head and tail of rotation dual linklist for timer calculation
    QueueNode* loophead;
    QueueNode* looptail;
}Queue;


void QueueInit(Queue* pq){ //linklist init 
    assert(pq);
    pq->head = NULL;
    pq->tail = NULL;
}

void QueuePush(Queue* pq, QDataType x, float time, int seq){ //pushback
    assert(pq);
    QueueNode* newnode = (QueueNode*)malloc(sizeof(QueueNode));
    if (newnode == NULL)
	{
		perror("malloc error\n");
		return;
	}
	newnode->val = x;
	newnode->next = NULL;
	newnode->timeStamp = time;

  newnode->check_arr = 0;
  newnode->buf_seq = seq;
	
	if (pq->tail == NULL) //check empty
	{
		assert(pq->head == NULL); 
		pq->head = pq->tail = newnode; 
	}
	else
	{
		pq->tail->next = newnode; 
		pq->tail = newnode; 
	}
}

void QueuePop(Queue* pq) //pop out head node
{
	assert(pq);
	assert(pq->head && pq->tail);
	if (pq->head->next == NULL)
	{
		free(pq->head);
		pq->head = pq->tail = NULL;
	}
	else
	{
		QueueNode* next = pq->head->next;
		free(pq->head);
		pq->head = next;
	}
}



void QueuePutInPlace(Queue* pq, QDataType x, int headSeq) // for B side, put packet in right place with right order
{ 
  if(x.seqnum < headSeq){
    printf(" duplicate packet from a\n");
    return;
  }
  if (pq->tail == NULL) //check empty
    {
      assert(pq);
      QueueNode* newnode = (QueueNode*)malloc(sizeof(QueueNode));
      newnode->next = NULL;
      newnode->check_arr = 0;
      newnode->buf_seq = headSeq;

      assert(pq->head == NULL); 
      pq->head = pq->tail = newnode; 
    }

  int step = x.seqnum - headSeq;
  struct QueueNode* temp_p;
  int temp_seq;
  temp_seq = headSeq;
  temp_p = pq->head;

  // if the arrived packet is not in order, then create empty space for every previous miss packet
  while(step>=0){
    if(!temp_p){
      assert(pq);
      QueueNode* newnode = (QueueNode*)malloc(sizeof(QueueNode));
      newnode->next = NULL;
      newnode->buf_seq = temp_seq;
      newnode->check_arr = 0;

      pq->tail->next = newnode; 
	  	pq->tail = newnode; 

      temp_p = newnode;

      if(temp_p->buf_seq == x.seqnum){
        printf("1 insert to buffer2: %d\n", x.seqnum);
        temp_p->val = x;
        temp_p->check_arr = 1;
      }

    }
    else if(temp_p->buf_seq == x.seqnum && temp_p->check_arr == 0){
      printf("2 insert to buffer2: %d\n", x.seqnum);
      temp_p->val = x;
      temp_p->check_arr = 1;
    } 

    temp_p = temp_p->next;
    temp_seq++;
    step -- ;
  }

}

//when packet in rotation dual linklist is received an ack, cut this packet of the linklist, and then re-connect the rest.
void cutAndConnect(Queue* pq, QueueNode* loopnode){
  if(loopnode == pq->loophead && loopnode == pq->looptail){
    pq->loophead = NULL;
    pq->looptail =  NULL;
  }
  else if(loopnode == pq->loophead){
    pq->loophead = pq->loophead->loopnext;
    pq->loophead->loopbefore = NULL;
  }
  else if(loopnode == pq->looptail){    
    pq->looptail = pq->looptail->loopbefore;
    pq->looptail->loopnext = NULL;
  }
  else{
    loopnode->loopbefore->loopnext = loopnode->loopnext;
    loopnode->loopnext->loopbefore = loopnode->loopbefore;

    loopnode->loopnext = NULL;
    loopnode->loopbefore = NULL;
  }

}

// when timer A_timerinterrupt was trigger rotated the linklist.
// (use the new head to calculate next timer)
void Rotated(Queue* pq){
  if(pq->loophead == pq->looptail){
    pq->loophead->loopbefore = NULL;
    pq->looptail->loopnext = NULL;
    return;
  }
  else{
    struct QueueNode* temp_p;
    temp_p = pq->loophead;

    pq->loophead = pq->loophead->loopnext;
    pq->loophead->loopbefore = NULL;

    pq->looptail->loopnext = temp_p;
    temp_p->loopbefore = pq->looptail;
    pq->looptail = temp_p;
    pq->looptail->loopnext = NULL;

  }

} 


int checksum(struct pkt *packet){ //check sum
  int checksum = 0;
  checksum+= packet->seqnum;
  checksum+= packet->acknum;
  for (int i=0; i<20; i++){
    checksum+=packet->payload[i];
  }
  return checksum;
}


//------------------------------ main function -------------------------------//

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{  
  printf("output_A start \n");
  QDataType data;
	data.acknum = A_seqA;
	data.seqnum = A_seqA;
   // the seq of packet from A fifth layer

  for (int i=0; i<20; i++){
  data.payload[i] = message.data[i];
  } 
  data.checksum = checksum(&data);

  //put packet in to buffer
  QueuePush(&buff1, data, 0, A_seqA);


	if(left_p == NULL){
		left_p == buff1.tail;
	}

  //when the first packet putted in window, initial the window pointer 
	if(buff1.head==buff1.tail && buff1.head!= NULL){
		left_p = buff1.head;
		right_p = buff1.head;
    
    buff1.loophead = buff1.head;
    buff1.looptail = buff1.tail;

    buff1.loophead->loopbefore = NULL;

		printf("start send %d \n", right_p->buf_seq);
		right_p->timeStamp = get_sim_time();
		tolayer3(0, right_p->val);
		starttimer(0,interval);
		moveStep--;
	}

  //move the right bondary of window, send the accumulated packet from buffer. 
  //put the sended packet into the tail of rotation linklist
  //record the sending time of packet
	while ( moveStep>0 && right_p->next!=NULL){
    QueueNode* temp_p = buff1.looptail;

    buff1.looptail->loopnext = right_p->next;
    buff1.looptail = right_p->next;
    buff1.looptail->loopbefore = temp_p;
    buff1.looptail->loopnext = NULL;

    buff1.loophead->loopbefore = NULL;

		right_p = right_p->next;

		if (right_p == left_p){
			starttimer(0, interval);
			// printf(" A_output catch !!! \n" );
		}

		moveStep--;
		// printf("A send %d \n",right_p->buf_seq);

    if(right_p->check_arr == 0){
      right_p->timeStamp = get_sim_time();
		  tolayer3(0, right_p->val);
    }

	}
  A_seqA++;  
  // printf("outPut_A finish\n\n");

}



/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{  
  // check corruption
  if (packet.checksum != checksum(&packet)){
		// printf("ACK is corrupt, ignore! \n");
		return;
	}

  // set a flag for packet which received ack
  // remove it from rotation linklist
	else if(packet.acknum > seqA){ 
    printf("A rec, but wait for base: %d\n", packet.acknum);
    struct QueueNode* temp;
    temp = left_p;
    while(temp && temp->buf_seq!=packet.acknum){
      // printf("buffer seq = %d \n",temp->buf_seq);
      temp = temp->next;
    }
    if(temp->check_arr!=1){
      cutAndConnect(&buff1, temp);
    }
    temp->check_arr = 1;
  }	

  // when the base file packet received ack, remove all packet with ack flag and in right order
  // move the left bondary of window, and send accumulated packet in buffer
  else if(packet.acknum == seqA){ 
    stoptimer(0);
    left_p->check_arr = 1;
    cutAndConnect(&buff1, left_p);

    while(left_p && left_p->check_arr == 1){
      // printf ("A finish and remove, Seq is %d ,head: %d \n", seqA,buff1.head->buf_seq);
			seqA++;
			moveStep++;
			left_p = left_p->next;		
			QueuePop(&buff1);
    }

		if( !left_p ){
			// printf(" A_input catch !!! \n");
		}
		else{
			// printf(" A_input send %d %d\n",right_p->buf_seq, left_p->buf_seq);

      if(!buff1.loophead||!buff1.looptail){
        buff1.loophead = left_p;
        buff1.looptail = left_p;
      }
      printf("A %d\n",buff1.head->buf_seq);

			while( moveStep>0 && right_p->next!=NULL){

        QueueNode* temp_p = buff1.looptail;
        buff1.looptail->loopnext = right_p->next;
        buff1.looptail = right_p->next;
        buff1.looptail->loopbefore = temp_p;

        buff1.looptail->loopnext = NULL;
        buff1.loophead->loopbefore = NULL;

        right_p = right_p->next;

        moveStep--;
        printf("A send %d \n",right_p->buf_seq);
        if(right_p->check_arr == 0){
          right_p->timeStamp = get_sim_time();
          tolayer3(0, right_p->val);
        }
			}

      // start timer for next waiting packet, use the head of rotation linklist timestamp to calculater timer
      printf("A start timer \n");
      starttimer(0,interval - (get_sim_time() - buff1.loophead->timeStamp));

		}
  }

  else{
		printf("wrong ack, Exp: %d, ack: %d \n", seqA, packet.acknum);
	}
  printf("finis A_input \n \n");
}


/* called when A's timer goes off */
void A_timerinterrupt() 
{ 
  //resend the timeout packet, updated timestamp
  tolayer3(0,buff1.loophead->val);
  buff1.loophead->timeStamp = get_sim_time();

  // rotate the rotation linked
  Rotated(&buff1);
  // use the new head to calculated the timer for next waiting packet
  if(buff1.loophead == buff1.looptail ){
    starttimer(0,interval);
  }
  else{
    starttimer(0,interval-(get_sim_time()-buff1.loophead->timeStamp));
  }

}  


/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{ seqA = 0;
  QueueInit(&buff1);

  // set the time-out depends on window size;
  if(getwinsize()<50){
    interval = 20;
  }
  else if(getwinsize()==50){
    interval = 40;
  }
  else if(getwinsize()==100){
    interval = 45;
  }
  else{
    interval = 50;
  }
  
  A_seqA = 0;
  moveStep = getwinsize();
  left_p = NULL;
  right_p = NULL;

}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{ 
  // check corruption
	if (packet.checksum != checksum(&packet)){
    printf("B has received the corrupt packet from A, ignore, seq \n");
    return;
  }

  // send ack for received packed
  struct pkt ACK;
  ACK.acknum = packet.seqnum;
  ACK.checksum = checksum(&ACK);
  printf("B received %d \n", packet.acknum);
  tolayer3(1, ACK);

  // put packet into buffer B, in right place with right order
  QueuePutInPlace(&buff2,packet,seqB);

  //send cumulate packet to layer 3 in order
  while(1){
    if(!buff2.head){
      // printf("buffer2 is empty \n");
      break;
    }
    else if(buff2.head->check_arr == 0){
      // printf("not in order \n");
      break;
    }
    else if(buff2.head->buf_seq == seqB && buff2.head->check_arr == 1){
      // printf("send to layer5 %d \n", buff2.head->buf_seq);
      tolayer5(1,buff2.head->val.payload);
      seqB++;
      QueuePop(&buff2);
    }

  }

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{ seqB = 0;

}


