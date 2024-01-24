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
float interval; 	//timer out time
struct Queue buff1; //buffer on A side

int moveStep; //free window size
int A_seqA;   //counter of total output packet from Layer 5 A
struct QueueNode* left_p, *right_p; // slow and fast pointer for left and right boundry of window
//left_p: next packet waiting for ack
//right_p: last packet already sent
typedef struct pkt QDataType;

typedef struct QueueNode{ //linklist Node structure
    QDataType val;
    struct QueueNode* next;
}QueueNode;

typedef struct Queue{ //linklist structure
    QueueNode* head;
    QueueNode* tail;
}Queue;


void QueueInit(Queue* pq){ //linklist init 
    assert(pq);
    pq->head = NULL;
    pq->tail = NULL;
}

void QueuePush(Queue* pq, QDataType x, float time){ //pushback
    assert(pq);
    QueueNode* newnode = (QueueNode*)malloc(sizeof(QueueNode));
    if (newnode == NULL)
	{
		perror("malloc error\n");
		return;
	}
	newnode->val = x;
	newnode->next = NULL;
	
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



int checksum(struct pkt *packet){ //checksum
  int checksum = 0;
  checksum+= packet->seqnum;
  checksum+= packet->acknum;
  for (int i=0; i<20; i++){
    checksum+=packet->payload[i];
  }
  return checksum;
}


//------------------------------ main functions -------------------------------//


/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{   QDataType data;
	data.acknum = A_seqA;
	data.seqnum = A_seqA;
    A_seqA++; // the seq of packet from A fifth layer

    for (int i=0; i<20; i++){
    data.payload[i] = message.data[i];
    } 
    data.checksum = checksum(&data);

	//put packet in to buffer
    QueuePush(&buff1, data, 0);


	if(left_p == NULL){
		left_p == buff1.tail;
	}

  	//when the first packet putted in window, initial the window pointer 
	if(buff1.head==buff1.tail && buff1.head!= NULL){
		left_p = buff1.head;
		right_p = buff1.head;
		printf("start send %d \n", seqA);
		tolayer3(0, right_p->val);
		starttimer(0,interval);
		moveStep--;
	}

  	//move the right bondary of window, send the accumulated packet from buffer. 
	while ( moveStep>0 && right_p->next!=NULL){
		if (right_p == left_p){
			starttimer(0, interval);
			printf(" A_output catch !!! \n");
		}
		right_p = right_p->next;

		moveStep--;
		printf("A send %d \n",seqA);
		tolayer3(0, right_p->val);
	}

}



/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{     
	// check corruption
	if (packet.checksum != checksum(&packet)){
		printf("ACK is corrupt, ignore! \n");
		// printf("	Exp check is %d , actual check is %d \n", packet.checksum, checksum(&packet));
		return;
	}
  	// remove accumulated packet when received ack
	else if(packet.acknum >= seqA){
		stoptimer(0);
		while(seqA<=packet.acknum){
			printf ("Correct transmit, Seq is %d \n", seqA);
			seqA++;
			moveStep++;
			left_p = left_p->next;		
			QueuePop(&buff1);
		}

		if(left_p == right_p || !left_p ){
			printf(" A_input catch !!! \n");
		}
		else{	

			//start new timer for next window
			//move the window, send packet from buffer
			printf("new timer \n");
			starttimer(0,interval);

			while ( moveStep>0 && right_p->next!=NULL){

				right_p = right_p->next;
				moveStep--;
				printf("A send %d \n",seqA);
				tolayer3(0, right_p->val);
			}
		}
	}

	else{
		printf("wrong ack, Exp: %d, ack: %d \n", seqA, packet.acknum);
	}

}

/* called when A's timer goes off */
void A_timerinterrupt() 
{ 
	//timeout,resend all packet in present window, updated timestamp
	printf("time out !!! \n");
	right_p = left_p;
	moveStep = getwinsize();
	starttimer(0, interval);
	printf(" back to %d \n", right_p->val.acknum);

	while ( moveStep>0 && right_p!= NULL && right_p->next!=NULL){

		moveStep--;

		tolayer3(0, right_p->val);
		right_p = right_p->next;

	}

}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{ seqA = 0;
  QueueInit(&buff1);
  interval = 20;
  //
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
	// drop the corrupt packet
	if (packet.checksum != checksum(&packet)){
        printf("B has received the corrupt packet from A, ignore, seq: %d\n", seqB);
        return;
    }
	
	// send duplicate ack for wrong packet
	else if(packet.seqnum != seqB){
		printf("B does not expect this packet from A, Exp: %d, act: %d \n",seqB, packet.seqnum );
		struct pkt ACK;
		ACK.acknum = seqB - 1; //ACK为上一个回复报文的ACK
		ACK.checksum =  checksum(&ACK);
		tolayer3(1, ACK);
	}

	// send ack, and pass data to layer 5 B
	else if (packet.seqnum == seqB && packet.checksum == checksum(&packet)){
		tolayer5(1, packet.payload);
		struct pkt ACK;
		ACK.acknum = seqB;
		ACK.checksum = checksum(&ACK);
		printf("B received %d \n", packet.acknum);
		tolayer3(1, ACK);
		seqB++; 
	}
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{ seqB = 0;

}


