/* 基于gbn_v3,修改了A_input中的每次更改seqA超时时间的代码*/
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
int seqA, seqB;
int start_count;
int index_buff;
float interval; //超时时间
struct pkts{
    struct pkt packet;
    float start_time;
    int empty;
};
struct pkt next_data; //发送方A即将发送的包
struct pkts buff[1000];

int checksum(struct pkt *packet){ 
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
    buff[index_buff].packet.seqnum = index_buff;
    buff[index_buff].packet.acknum = -1;
    buff[index_buff].empty = 1;
    for (int i=0; i<20; i++){
        buff[index_buff].packet.payload[i] = message.data[i];
    }

    buff[index_buff].packet.checksum = checksum(&buff[index_buff].packet);
    index_buff++;

    if (start_count < seqA + getwinsize()){
        if (buff[start_count].empty != 0){
            next_data = buff[start_count].packet;
            printf("A sends packet:%s to B\n", next_data.payload);
            tolayer3(0, next_data);
            buff[start_count].start_time = get_sim_time();
          if (start_count == seqA){
            starttimer(0, interval);
          }
          start_count++;
        }
        else if (buff[start_count].empty == 0){
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
    printf("ACK is corrupt, ignore!\n");
    return;
    }

    if (packet.acknum >= seqA && packet.acknum <= seqA + getwinsize() - 1){
    seqA = seqA + (packet.acknum - seqA + 1);
    if (seqA == start_count){
        stoptimer(0);
    }
    else if (seqA != start_count){
        starttimer(0, interval - (get_sim_time() - buff[seqA].start_time));
    }
    
    //if (buff[seqA].start_time != 0.0){
       // starttimer(0, interval - (get_sim_time() - buff[seqA].start_time));
     //}

    if (start_count < seqA + getwinsize()){
        if (buff[start_count].empty != 0){
            next_data = buff[start_count].packet;
            printf("A sends packet:%s to B\n", next_data.payload);
            tolayer3(0, next_data);
            buff[start_count].start_time = get_sim_time();
          if (start_count == seqA){
            starttimer(0, interval);
          }
          start_count++;
        }
        else if (buff[start_count].empty == 0){
            printf("A is waiting for message from layer5\n");
        }
    }
    }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{   for (int i=seqA; i < start_count; i++){
    next_data = buff[i].packet;
    tolayer3(0, next_data);
    if (i == seqA){
        starttimer(0, interval);
    }
    printf("A resends with seq=%d: %s to B\n", i, next_data.payload);
  }
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{  seqA = 0;
   start_count = 0;
   interval = 20.0;
   index_buff = 0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{   
    if (packet.checksum != checksum(&packet)){
    printf("The packet:%d from A is corrupt, ignore\n", packet.seqnum);
    return;
    }
    
    else if (packet.seqnum == seqB){
    printf("B has recieved the packet with seqnum:%d from A sucessfully!\n", packet.seqnum);
    tolayer5(1, packet.payload);
    struct pkt ACK;
    ACK.acknum = seqB;
    ACK.checksum = checksum(&ACK); //ACK.seqnum + ACK.acksum + ACK.payload
    tolayer3(1, ACK);
    seqB++;
   }

    else if (packet.seqnum != seqB){
    printf("B does not expect this packet from A, discard\n");
    struct pkt ACK;
    ACK.acknum = seqB - 1;
    ACK.checksum = checksum(&ACK);
    tolayer3(1, ACK);
}
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{ 
    seqB = 0;
}
