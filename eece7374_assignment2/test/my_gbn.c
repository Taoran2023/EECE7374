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
//int lastsucess = 0;
int start_count = 0;
//int lastsequence = 0;
float interval; //超时时间
struct Queue buff1; //发送方A的缓存，若A还没收到上一个包的ACK，同时上层又给三层发送数据，则A将上层的数据先放到缓存中。数据结构采用队列

struct pkt next_data; //发送方A即将发送的包
typedef struct pkt QDataType;

typedef struct QueueNode{ //队列链表中每个元素的结构
    QDataType val;
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

void QueuePush(Queue* pq, QDataType x){ //队尾进队
    assert(pq);
    QueueNode* newnode = (QueueNode*)malloc(sizeof(QueueNode));
    if (newnode == NULL)
	{
		perror("malloc error\n");
		return;
	}
	newnode->val = x;
	newnode->next = NULL;
	
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

QDataType QueueFront(Queue* pq) //获取队列的第一个元素，但不弹出，相当于发送方发送缓存中的第一个包，但是再接受到ACK之前都不会弹出队列，以便重传
{
	assert(pq);
	assert(pq->head && pq->tail); //
	return pq->head->val;
}

bool QueueEmpty(Queue* pq){ //判断队列是否为空
    assert(pq);
    return pq->head == NULL;
}

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

moveSteps int  -- max = windowsize, the boundary of right_p
left_p  *  -- wait for ack
right_p  * -- already send

*/ 

int moveStep;
struct QueueNode* left_p, *right_p;
int A_seqA, B_seqB; //occupide buffer size


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
    QueuePush(&buff1, data);


	if(left_p == NULL){
		left_p == buff1.tail;
	}

	if(buff1.head==buff1.tail && buff1.head!= NULL){
		left_p = buff1.head;
		right_p = buff1.head;
		tolayer3(0, right_p->val);
		// starttimer(0, interval);
		moveStep--;
	}
	// QueuePush(&buff2, data);


	while ( moveStep>0 && right_p->next!=NULL){

		if (right_p == left_p){
			starttimer(0, interval);
			printf(" A_output catch !!! \n");

		}

		right_p = right_p->next;
		moveStep--;

		tolayer3(0, right_p->val);
	

	}

}



/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{   if (packet.checksum != checksum(&packet)){
		printf("ACK is corrupt, ignore! \n");
		printf("	Exp check is %d , actual check is %d \n", packet.checksum, checksum(&packet));

		return;
	}

	printf ("Correct transmit, Seq is %d \n", seqA);

	if(packet.acknum == seqA){

		seqA++;
		moveStep++;
		// struct QueueNode* temp = left_p;
		left_p = left_p->next;
		// free(temp);
		if(left_p == right_p){
			stoptimer(0);
			printf(" A_input catch !!! \n");
			printf("stop timer by p=p \n");
		}
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

		tolayer3(0, right_p->val);
		right_p = right_p->next;

	}

}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{ seqA = 0;
  start_count = 0;
  QueueInit(&buff1);
  interval = 20+getwinsize();

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
        printf("B has received the corrupt packet from A, ignore\n");
        return;
    }
	
	if (packet.seqnum == seqB && packet.checksum == checksum(&packet)){
		tolayer5(1, packet.payload);
		struct pkt ACK;
		ACK.acknum = seqB;
		ACK.checksum = checksum(&ACK);
		tolayer3(1, ACK);
		seqB++; //B下次期望收到的序号
		}

	else if(packet.seqnum != seqB){
		printf("B does not expect this packet from A, Exp: %d, act: %d \n",seqB, packet.seqnum );
		struct pkt ACK;
		ACK.acknum = seqB - 1; //ACK为上一个回复报文的ACK
		ACK.checksum =  checksum(&ACK);
		tolayer3(1, ACK);
	}

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{ seqB = 0;
  B_seqB = 0;

}
