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
int start_count;
int index_buff;
float interval; //超时时间
struct pkt next_data; //发送方A即将发送的包
struct pkt buff[1000];

int checksum(struct pkt *packet){ //计算校验和
  int checksum = 0;
  checksum+= packet->seqnum;
  checksum+= packet->acknum;
  for (int i=0; i<20; i++){
    checksum+=packet->payload[i];
  }
  return checksum;
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
    buff[index_buff].seqnum = index_buff;
    buff[index_buff].acknum = index_buff;
    for (int i=0; i<20; i++){
        buff[index_buff].payload[i] = message.data[i];
    }
    buff[index_buff].checksum = checksum(&buff[index_buff]);
    index_buff++;


    //data.seqnum = QueueSize(&buff1);
    //data.acknum = QueueSize(&buff1);
    //for (int i=0; i<20; i++){
    //data.payload[i] = message.data[i];
    //} 
    //data.checksum = checksum(&data);
    //QueuePush(&buff1, data);
	//QueuePush(&buff2, data);

	//if(StateA != 0) {
    //printf("A is waiting for ACK from B, puts the new message into buffer: %s\n", message.data);
    //return;
    //}

    if (start_count < seqA + getwinsize()){
        next_data = buff[start_count];
        printf("A sends %s to B\n", next_data.payload);
        tolayer3(0, next_data);
        if (start_count == seqA){
            starttimer(0, interval);
        }
        start_count++;
    }

	//if (start_count < seqA + getwinsize()){
	//	next_data = QueueFront(&buff2);
	//	printf("A sends: %s to B\n", next_data.payload);
	//	tolayer3(0, next_data);
	//	if (start_count == seqA){
	//		starttimer(0, interval);
	//	}
	//	struct pkt temp;
	//	temp = next_data;
	//	//temp = QueueFront(&buff2);
	//	QueuePush(&buff3, temp);
	//	QueuePop(&buff2);
	//	start_count++;
    // } 
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{   if (packet.checksum != checksum(&packet)){
    printf("ACK is corrupt, ignore!\n");
    return;
}

if (packet.acknum >= seqA && packet.acknum <= seqA + getwinsize() - 1){
    seqA = seqA + (packet.acknum - seqA + 1);

}
    if (seqA == start_count){
        stoptimer(0);
        printf("A sends next window!\n");
        if (start_count < seqA + getwinsize()){
            next_data = buff[start_count];
            printf("A sends %s to B\n", next_data.payload);
            tolayer3(0, next_data);
            if (start_count == seqA){
                starttimer(0, interval);
            }
            start_count++;
        }

    }

    //if (packet.checksum != checksum(&packet)){
	//printf("ACK is corrupt, ignore!\n");
	//return;
//}

	//if (packet.acknum >= seqA && packet.acknum <= seqA + getwinsize() - 1){
	//start_count = start_count - (packet.acknum - seqA + 1);
    //seqA = seqA + (packet.acknum - seqA + 1);
	//for (int i=0; i<packet.acknum - seqA + 1 ; i++){
	//	QueuePop(&buff1);
	//	QueuePop(&buff3); //
	//}
	//if (seqA == start_count){
	//	stoptimer(0);
	//	printf("A sends next window\n");
//
	//	if (start_count < seqA + getwinsize()){
	//	next_data = QueueFront(&buff2);
	//	printf("A sends: %s to B\n", next_data.payload);
	//	tolayer3(0, next_data);
	//	if (start_count == seqA){
	//		starttimer(0, interval);
	//	}
	//	QueuePop(&buff2);
	//	start_count++;
	//}
	//}
	//else {
	//	starttimer(0, interval);
	//}
  //}
}

/* called when A's timer goes off */
void A_timerinterrupt()
{   for (int i=seqA; i<start_count; i++){
    next_data = buff[i];
    tolayer3(0, next_data);
    printf("A resends with seq=%d: %s to B\n", i, next_data.payload);
}
starttimer(0, interval);
    
    
    
    
    //for (int i=seqA; i< start_count; i++){
	//struct pkt resend_pkts;
	//resend_pkts = QueueFront(&buff3);
	//tolayer3(0, resend_pkts);
	//printf("A resends packet with seq=%d : %s\n", resend_pkts.seqnum, resend_pkts.payload);
	//QueuePop(&buff3);
//}
//starttimer(0, interval);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{  seqA = 0;
   start_count = 0;
   interval = 20;
   index_buff = 0;
  //seqA = 0;
  //start_count = 0;
  //QueueInit(&buff1);
  //QueueInit(&buff2);
  //QueueInit(&buff3);
  //interval = 20;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{   if (packet.seqnum == seqB && packet.checksum == checksum(&packet)){
    tolayer5(1, packet.payload);
    struct pkt ACK;
    ACK.acknum = seqB;
    ACK.checksum = packet.seqnum;
    tolayer3(1, ACK);
    seqB++;
}
    
    
    
   // if (packet.seqnum == seqB && packet.checksum == checksum(&packet)){
	//tolayer5(1, packet.payload);
	//struct pkt ACK;
	//ACK.acknum = seqB;
	//ACK.checksum = packet.seqnum;
	//tolayer3(1, ACK);
	//seqB++; //B下次期望收到的序号
//}

if (packet.seqnum != seqB){
    printf("B does not expect this packet from A\n");
    struct pkt ACK;
    ACK.acknum = seqB - 1;
    ACK.checksum = packet.seqnum;
    tolayer3(1, ACK);
}

//if(packet.seqnum != seqB){
//	printf("B does not expect this packet from A\n");
//	struct pkt ACK;
//	ACK.acknum = seqB - 1; //ACK为上一个回复报文的ACK
//	ACK.checksum = packet.seqnum;
//	tolayer3(1, ACK);
//}

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{ seqB = 0;

}
