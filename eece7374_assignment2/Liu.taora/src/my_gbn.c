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
int StateA, StateB, seqA, seqB, ACK; //StateA=0，代表A等待上层传下来，StateA=1代表A等待ACK
int start_count = 0;
float interval; //timer time length
struct Queue buff1; //buffer on A side

int moveStep; //free window size
int A_seqA;   //counter of total output packet from Layer 5 A
struct QueueNode* left_p, *right_p; // slow and fast pointer
//left_p: next packet waiting for ack
//right_p: last packet already sent

float lastRTT = 0;


struct pkt next_data; //发送方A即将发送的包
typedef struct pkt QDataType;

typedef struct QueueNode{ //队列链表中每个元素的结构
    QDataType val;
	float timeStamp;
    struct QueueNode* next;
}QueueNode;

typedef struct Queue{ //队列的结构
    QueueNode* head;
    QueueNode* tail;
}Queue;


void QueueInit(Queue* pq){ //队列初始化 
    assert(pq);
    pq->head = NULL;
    pq->tail = NULL;
}

void QueuePush(Queue* pq, QDataType x, float time){ //队尾进队
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
	
	if (pq->tail == NULL) //判断队列是否为空
	{
		assert(pq->head == NULL); //如果队列为空，头指针也应为空
		pq->head = pq->tail = newnode; //将新节点同时设置为队列的头节点和尾节点
	}
	else
	{
		pq->tail->next = newnode; //将新节点同时设置为队列的头节点和尾节点
		pq->tail = newnode; //更新队列的尾指针为新节点
	}
}

void QueuePop(Queue* pq) //队头出队，在程序中相当于发送方A收到了包的ACK并将该包弹出缓存队列
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

// QDataType QueueFront(Queue* pq) //获取队列的第一个元素，但不弹出，相当于发送方发送缓存中的第一个包，但是再接受到ACK之前都不会弹出队列，以便重传
// {
// 	assert(pq);
// 	assert(pq->head && pq->tail); //
// 	return pq->head->val;
// }

// bool QueueEmpty(Queue* pq){ //判断队列是否为空
//     assert(pq);
//     return pq->head == NULL;
// }

int QueueSize(Queue* pq)  //返回队列的元素数量
{
	assert(pq);
	QueueNode* cur = pq->head;
	int count = 0;
	while (cur)
	{
		cur = cur->next;
		count++;
	}
	return count;
}

int checksum(struct pkt *packet){ //计算校验和
  int checksum = 0;
  checksum+= packet->seqnum;
  checksum+= packet->acknum;
  for (int i=0; i<20; i++){
    checksum+=packet->payload[i];
  }
  return checksum;
}


/*

// moveSteps int  -- max = windowsize, the boundary of right_p
// left_p  *  -- wait for ack
// right_p  * -- already send

// */ 


/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{   QDataType data;
	data.acknum = A_seqA;
	data.seqnum = A_seqA;
    A_seqA++;

    for (int i=0; i<20; i++){
    data.payload[i] = message.data[i];
    } 
    data.checksum = checksum(&data);
	
    QueuePush(&buff1, data, 0);


	if(left_p == NULL){
		left_p == buff1.tail;
	}

	if(buff1.head==buff1.tail && buff1.head!= NULL){
		left_p = buff1.head;
		right_p = buff1.head;
		printf("start send %d \n", seqA);
		right_p->timeStamp = get_sim_time();
		tolayer3(0, right_p->val);
		starttimer(0,interval);
		moveStep--;
	}


	while ( moveStep>0 && right_p->next!=NULL){

		if (right_p == left_p){
			starttimer(0, interval);
			printf(" A_output catch !!! \n");
		}

		right_p = right_p->next;
		moveStep--;
		printf("A send %d \n",seqA);
		right_p->timeStamp = get_sim_time();
		tolayer3(0, right_p->val);
	}

}



/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{   if (packet.checksum != checksum(&packet)){
		printf("ACK is corrupt, ignore! \n");
		// printf("	Exp check is %d , actual check is %d \n", packet.checksum, checksum(&packet));
		return;
	}

	else if(packet.acknum >= seqA){
		stoptimer(0);
		float tempt = left_p->timeStamp;
		while(seqA<=packet.acknum){
			printf ("Correct transmit, Seq is %d \n", seqA);
			seqA++;
			moveStep++;
			left_p = left_p->next;		
			QueuePop(&buff1);
		}
		printf("period1  = %f, for seq: %d \n",get_sim_time() - tempt, A_seqA );

		// printf("period2  = %f, for seq: %d \n",get_sim_time() - left_p->timeStamp, A_seqA );

		if(left_p == right_p || !left_p ){
			// stoptimer(0);
			printf(" A_input catch !!! \n");
			// printf("stop timer by p=p \n");
		}
		else{	
			// printf("buffer size : %d \n", QueueSize(&buff1));
			printf("new timer \n");
			// printf("part timer = %f, for seq: %d \n",get_sim_time() - left_p->timeStamp, A_seqA );

			if(interval - (get_sim_time() - left_p->timeStamp)>0){

				// printf("part timer = %f, for seq: %d \n",get_sim_time() - left_p->timeStamp, A_seqA );
				starttimer(0,interval - (get_sim_time() - left_p->timeStamp));
				// starttimer(0,interval);
			}
			else{
				starttimer(0,interval);


			}
		
			while ( moveStep>0 && right_p->next!=NULL){

				right_p = right_p->next;
				moveStep--;
				printf("A send %d \n",right_p->val.seqnum);
				right_p->timeStamp = get_sim_time();
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
	printf("time out !!! \n");
	right_p = left_p;
	moveStep = getwinsize();

	while ( moveStep>0 && right_p!= NULL && right_p->next!=NULL){

		if (right_p == left_p){
			printf(" back to %d \n", right_p->val.acknum);
			starttimer(0, interval);
		}

		moveStep--;

		right_p->timeStamp = get_sim_time();
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
	if (packet.checksum != checksum(&packet)){
        printf("B has received the corrupt packet from A, ignore, seq: %d\n", seqB);
		struct pkt ACK;
		ACK.acknum = seqB - 1; //ACK为上一个回复报文的ACK
		ACK.checksum =  checksum(&ACK);
		tolayer3(1, ACK);
        // return;
    }
	
	else if(packet.seqnum != seqB){
		printf("B does not expect this packet from A, Exp: %d, act: %d \n",seqB, packet.seqnum );
		struct pkt ACK;
		ACK.acknum = seqB - 1; //ACK为上一个回复报文的ACK
		ACK.checksum =  checksum(&ACK);
		tolayer3(1, ACK);
	}

	else if (packet.seqnum == seqB && packet.checksum == checksum(&packet)){
		tolayer5(1, packet.payload);
		struct pkt ACK;
		ACK.acknum = seqB;
		ACK.checksum = checksum(&ACK);
		printf("B received %d \n", packet.acknum);
		tolayer3(1, ACK);
		seqB++; //B下次期望收到的序号
	}
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{ seqB = 0;

}

// float get_interval_adv()
// {
// float temp_interval;

// return temp_interval; 
// }

