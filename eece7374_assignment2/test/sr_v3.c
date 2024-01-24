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

/* called from layer 5, passed the data to be sent to other side */
int seqA, seqB;
int start_count;
int index_buff; //记录A的缓存收到上层包的个数
float interval; //超时时间
//int timer_occupy;

struct pkts{
  struct pkt packet;
  float start_time;
  int empty_A;
};

struct pkts_B{
  struct pkt packet;
  int empty;
};

struct pkts buff[1000]; //发送方的缓存数组
struct pkts next_data; //发送方A即将发送的包
struct pkt next_data_B;
struct pkts_B recv_buff[1000];

int checksum(struct pkt *packet){ //计算校验和
  int checksum = 0;
  checksum+= packet->seqnum;
  checksum+= packet->acknum;
  for (int i=0; i<20; i++){
    checksum+=packet->payload[i];
  }
  return checksum;
}


void A_output(message)
  struct msg message;
{ 
  buff[index_buff].packet.seqnum = index_buff;
  buff[index_buff].packet.acknum = -1;
  buff[index_buff].empty_A = 1;
  for (int i=0; i<20; i++){
    buff[index_buff].packet.payload[i] = message.data[i];
  }
  buff[index_buff].packet.checksum = checksum(&buff[index_buff].packet);
  index_buff++;

  if (start_count < seqA + getwinsize()){
    if (buff[start_count].empty_A != 0){
        next_data = buff[start_count];
        printf("A sends the packet:%s with seqnum:%dto B\n", next_data.packet.payload, next_data.packet.seqnum);
        tolayer3(0, next_data.packet);
        buff[start_count].start_time = get_sim_time();
        if (start_count == seqA){
        starttimer(0, interval);
    }
    start_count++;
    }

    else if (buff[start_count].empty_A == 0){
        printf("A is waiting for message from layer5\n");
        return;
    }
  }
}


/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{ 
  if (packet.checksum != checksum(&packet)){
    printf("A has received the corrupt ACK, ignore!\n");
  }

  if (packet.checksum == checksum(&packet)){
   if (seqA == packet.acknum){
    stoptimer(0);
    buff[seqA].packet.acknum = packet.acknum;
    seqA++;
   // timer_occupy = 0;

   for (int i=seqA; i<start_count; i++){
     if (buff[i].packet.acknum != -1){
        seqA++;
     }
     if (buff[i].packet.acknum == -1){
      break;
     }
   }

    if (buff[seqA].start_time != 0.0){
      int remain_time = interval - (get_sim_time() - buff[seqA].start_time);
      starttimer(0, remain_time);
    }

    if (start_count < seqA + getwinsize()){
    if (buff[start_count].empty_A != 0){
        next_data = buff[start_count];
        printf("A sends the packet:%s with seqnum:%dto B\n", next_data.packet.payload, next_data.packet.seqnum);
        tolayer3(0, next_data.packet);
        buff[start_count].start_time = get_sim_time();
        if (start_count == seqA){
        starttimer(0, interval);
    }
    start_count++;
    }
    else if (buff[start_count].empty_A == 0){
        printf("A is waiting for message from layer5\n");
        return;
    }
  }
    //int remain_time = interval - (get_sim_time() - buff[seqA].start_time);
    //starttimer(0, remain_time);
 }

   else if (packet.acknum>seqA && packet.acknum <= seqA + getwinsize() - 1){
    buff[packet.acknum].packet.acknum = packet.acknum;
   }

  // for (int i=seqA; i<start_count; i++){
   //  if (buff[i].packet.acknum != -1){
    //    seqA++;
   //  }
   //  if (buff[i].packet.acknum == -1){
   //   break;
    // }
   //}
  }
}


/* called when A's timer goes off */
void A_timerinterrupt()
{ printf("A is timeout!\n");
  tolayer3(0, buff[seqA].packet);
  printf("A resends packet no.%d to B\n", buff[seqA].packet.seqnum);
  buff[seqA].start_time = get_sim_time();
  starttimer(0, interval);
  for (int i=0, j=seqA; i<getwinsize(), j<start_count; i++, j++){
    if (buff[j].packet.acknum == -1){
      if (get_sim_time() - buff[j].start_time > interval){
        buff[j].start_time = get_sim_time();
        tolayer3(0, buff[j].packet);
        printf("A resends packet no.%d to B\n", buff[j].packet.seqnum);
      }
    }
  }
}  


/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{ start_count = 0;
  index_buff = 0;
  seqA = 0;
  interval = 20.0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{ 
  if (packet.checksum != checksum(&packet)){
    printf("B has received the corrupt packet from A, ignore!\n");
  }

  if (packet.checksum == checksum(&packet)){
    if (packet.seqnum == seqB){
      recv_buff[seqB].packet = packet;
      recv_buff[seqB].empty = 1;
      struct pkt ACK;
      ACK.acknum = seqB;
      ACK.checksum = checksum(&ACK);
      tolayer5(1, packet.payload);
      tolayer3(1, ACK);
      seqB++;
    }
    else if (packet.seqnum > seqB && packet.seqnum < seqB + getwinsize()){
      recv_buff[packet.seqnum].packet = packet;
      recv_buff[packet.seqnum].empty = 1;
      struct pkt ACK;
      ACK.acknum = packet.seqnum;
      ACK.checksum = checksum(&ACK);
      tolayer3(1, ACK);
    }
    
    for (int i = seqB; i<seqB + getwinsize(); i++){
      if (recv_buff[i].empty == 1){
        tolayer5(1, recv_buff[i].packet.payload);
        seqB++;
      }
      if (recv_buff[i].empty == 0){
        break;
      }
    }
 }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{ 
    seqB = 0;
}
