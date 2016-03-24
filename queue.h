#ifndef  _QUEUE_H_
#define _QUEUE_H_

#include "defs.h"
#include "param.h"

#define QUEUESIZE NPROC

struct queue{
  struct proc* arr[QUEUESIZE];             /* body of queue */
  int first;                      /* position of first element */
  int last;                       /* position of last element */
  int count;                      /* number of queue elements */
};

#endif //_QUEUE_H_
