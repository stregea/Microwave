// queue.c  A character queue
#include  "queue.h"     // prototype definitions
#define   QMAX 16       // size of queue
static char qbuff[QMAX];  // the queue
static int  front; 
static int  rear;         // queue pointers
static int  min = 0;      // start of queue 
static int  max = QMAX-1; // end of queue
 
void initq(void){
	min = 0;
	front = 0;
	rear = 0;
	max = QMAX-1;
}

void  qstore(char c){
  rear++;                 	// inc rear
  if(rear > max)
    rear = min;
  if(rear == front){
    rear--;               	// queue is full
    if(rear < min)		//  rewind rear
      rear = max;
  }else
    qbuff[rear] = c;		// store c at rear
}

int qempty(void){
  int flag;
    if(front == rear)
      flag = 1;
    else
      flag = 0;
  return (flag);
}

char  getq(void){
  front++;                // inc front
  if(front > max)
    front = 0;
  return qbuff[front];    // return value at front
}

