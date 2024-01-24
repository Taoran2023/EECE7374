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
struct Queue buff2; //buffer on B side


int moveStep; //free window size
int A_seqA;   //counter of total output packet from Layer 5 A
struct QueueNode* left_p, *right_p; // slow and fast pointer
//left_p: next packet waiting for ack
//right_p: last packet already sent


struct pkt next_data; //发送方A即将发送的包
typedef struct pkt QDataType;

typedef struct QueueNode{ //队列链表中每个元素的结构
    QDataType val;
	  float timeStamp;
    struct QueueNode* next;

    int buf_seq; // only for SR B side
    int check_arr; // check whether arrive
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

void QueuePush(Queue* pq, QDataType x, float time, int seq){ //队尾进队
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



void QueuePutInPlace(Queue* pq, QDataType x, int headSeq){

  if(x.seqnum < headSeq){
    printf(" duplicate packet from a\n");
    return;
  }
  if (pq->tail == NULL) //判断队列是否为空
    {
      assert(pq);
      QueueNode* newnode = (QueueNode*)malloc(sizeof(QueueNode));
      newnode->next = NULL;
      newnode->check_arr = 0;
      newnode->buf_seq = headSeq;

      assert(pq->head == NULL); //如果队列为空，头指针也应为空
      pq->head = pq->tail = newnode; //将新节点同时设置为队列的头节点和尾节点
    }

  int step = x.seqnum - headSeq;
  struct QueueNode* temp_p;
  int temp_seq;
  temp_seq = headSeq;
  temp_p = pq->head;

  while(step>=0){
    
    if(!temp_p){
      assert(pq);
      QueueNode* newnode = (QueueNode*)malloc(sizeof(QueueNode));
      newnode->next = NULL;
      newnode->buf_seq = temp_seq;
      newnode->check_arr = 0;

      pq->tail->next = newnode; //将新节点同时设置为队列的头节点和尾节点
	  	pq->tail = newnode; //更新队列的尾指针为新节点

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

  QueuePush(&buff1, data, 0, A_seqA);


	if(left_p == NULL){
		left_p == buff1.tail;
	}

	if(buff1.head==buff1.tail && buff1.head!= NULL){
		left_p = buff1.head;
		right_p = buff1.head;
    // retrans_p = left_p;
		printf("start send %d \n", right_p->buf_seq);
		right_p->timeStamp = get_sim_time();
		tolayer3(0, right_p->val);
		starttimer(0,interval);
		moveStep--;
	}


	while ( moveStep>0 && right_p->next!=NULL){
		right_p = right_p->next;

		if (right_p == left_p){
			starttimer(0, interval);
			printf(" A_output catch !!! \n" );
		}

		moveStep--;
		printf("A send %d \n",right_p->buf_seq);

    if(right_p->check_arr == 0){
      right_p->timeStamp = get_sim_time();
		  tolayer3(0, right_p->val);
    }

	}
  A_seqA++;  
  printf("outPut_A finish\n\n");

}



/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{  
  printf("start A_inpu \n");
  if (packet.checksum != checksum(&packet)){
		printf("ACK is corrupt, ignore! \n");
		// printf("	Exp check is %d , actual check is %d \n", packet.checksum, checksum(&packet));
		return;
	}

	else if(packet.acknum > seqA){ // set the correct pactet check_arr
    printf("A rec, but wait for base: %d\n", packet.acknum);
    struct QueueNode* temp;
    temp = left_p;
    while(temp->buf_seq!=packet.acknum){
      temp = temp->next;
    }
    temp->check_arr = 1;
  }	

  else if(packet.acknum == seqA){ // move left_p and pop
    stoptimer(0);
    left_p->check_arr = 1;
    while(left_p && left_p->check_arr == 1){
      printf ("A finish and remove, Seq is %d ,head: %d \n", seqA,buff1.head->buf_seq);
			seqA++;
			moveStep++;
			left_p = left_p->next;		
			QueuePop(&buff1);
    }

		if( !left_p ){
			printf(" A_input catch !!! \n");
		}
		else{

			while( moveStep>0 && right_p->next!=NULL){
        right_p = right_p->next;
        moveStep--;
        printf("A send %d \n",seqA);
        if(right_p->check_arr == 0){
          right_p->timeStamp = get_sim_time();
          tolayer3(0, right_p->val);
        }
			}


      printf("A start timer \n");

      struct QueueNode* retrans_p, *find_p,*timer_p;
      retrans_p = left_p;
      find_p = left_p;

      while(find_p && find_p!=right_p->next){
        if((find_p && find_p->timeStamp < retrans_p->timeStamp) && (find_p->check_arr==0)){
          retrans_p = find_p;
        } 
        find_p = find_p->next;
      }
      printf("A stat part timer = %f \n",get_sim_time() - retrans_p->timeStamp );


      starttimer(0,interval - (get_sim_time() - retrans_p->timeStamp));




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
  printf("timer interrupt \n");
  struct QueueNode* retrans_p, *find_p,*timer_p;
  retrans_p = left_p;
  timer_p = left_p;
  find_p = left_p;
  int size = 0;
  while(find_p && find_p!=right_p->next){
    if((find_p && find_p->timeStamp < retrans_p->timeStamp) && (find_p->check_arr==0)){
      retrans_p = find_p;
    } 
    printf("     sis-stamp = %f, seq = %d, check = %d\n",get_sim_time() - find_p->timeStamp, find_p->buf_seq, find_p->check_arr);
    find_p = find_p->next;
    size++;
  } 
  find_p=left_p;
  while(find_p && find_p!=right_p->next){
    if((find_p && find_p->timeStamp < timer_p->timeStamp) && (find_p->check_arr==0)&& find_p!=retrans_p){ // find_p->timeStamp - retrans_p->timeStamp >0.01){
      // timer_p = retrans_p;
      timer_p = find_p;
    } 
    find_p = find_p->next;
  }

      // printf("part timer = %f, for seq: %d \n",interval - (get_sim_time() - timer_p->timeStamp), retrans_p->buf_seq );
      // printf("size = %d \n",size);

  starttimer(0,interval - (get_sim_time() - timer_p->timeStamp));


  // find_p = left_p;
  // while(find_p && find_p!=right_p->next){
  //   if(find_p && find_p->timeStamp - retrans_p->timeStamp<0.000001 && (find_p->check_arr==0)){
  //     tolayer3(0,find_p->val); 
  //     // printf("re-send %d \n", find_p->buf_seq);
  //     find_p ->timeStamp = get_sim_time();
  //   } 
  //   find_p = find_p->next;
  // }
  tolayer3(0,retrans_p->val); 
  printf("re-send %d \n", retrans_p->buf_seq);
  retrans_p ->timeStamp = get_sim_time();
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
    printf("B has received the corrupt packet from A, ignore, seq \n");
    return;
  }
  struct pkt ACK;
  ACK.acknum = packet.seqnum;
  ACK.checksum = checksum(&ACK);
  printf("B received %d \n", packet.acknum);
  tolayer3(1, ACK);

  //push in place
  QueuePutInPlace(&buff2,packet,seqB);

  // printf("finish insert\n");
  // to layer 3 in order
  int i = 5;
  while(i>0){
    if(!buff2.head){
      printf("buffer2 is empty \n");
      break;
    }
    else if(buff2.head->check_arr == 0){
      printf("not in order \n");
      break;
    }
    else if(buff2.head->buf_seq == seqB && buff2.head->check_arr == 1){
      printf("send to layer5 %d \n", buff2.head->buf_seq);
      tolayer5(1,buff2.head->val.payload);
      seqB++;
      QueuePop(&buff2);
      printf("pop\n");
    }
    else{
      struct QueueNode* temp;
      temp = buff2.head;
      while(temp){
        printf("error!! now buffer has: %d, %d \n", temp->buf_seq,temp->check_arr);
        temp = temp->next;
      }
      printf("\n");
    }
  i--;

  }
  printf("finish B input \n \n");

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{ seqB = 0;

}


