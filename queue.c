
/* 
  queue.c
  Implementation of a FIFO queue abstract data type.

  by: Steven Skiena
  begun: March 27, 2002
  
  Edited by : Reut Zinman
  Date: March 23, 2016
*/
  
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"

void 
init_queue(struct queue *q)
{
  cprintf("init_queue\n");
  q->first = 0;
  q->last = 0;
  q->count = 0;
}

void 
enqueue(struct queue *q, struct proc *x)
{
  cprintf("enqueue\n");
  if (q->count >= QUEUESIZE){ /* queque is full - do nothing */ }
  else {
    q->last = (q->last+1) % QUEUESIZE;
    q->arr[ q->last ] = x;    
    q->count = q->count + 1;
  }
}

struct proc*
dequeue(struct queue *q)
{
  cprintf("dequeue\n");
  struct proc *x;

  if (q->count <= 0){ /* do nothing */ } 
  else {
    x = q->arr[ q->first ];
    q->first = (q->first+1) % QUEUESIZE;
    q->count = q->count - 1;
  }
  return x;
}

int 
empty(struct queue *q)
{
  if (q->count <= 0) {
    // cprintf("empty\n");
    return 1;
  }
  else {
    cprintf("not empty\n");
    return 0;
  }
}

