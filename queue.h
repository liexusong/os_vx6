#ifndef  _QUEUE_H_
#define _QUEUE_H_

#include "param.h"
//#include "defs.h"

#define QUEUESIZE NPROC

typedef struct {
  struct proc* arr[QUEUESIZE];             /* body of queue */
  int first;                      /* position of first element */
  int last;                       /* position of last element */
  int count;                      /* number of queue elements */
} queue;

void init_queue(queue *q); 

void enqueue(queue *q, struct proc *x);

struct proc* dequeue(queue *q);

int empty(queue *q);

#endif
