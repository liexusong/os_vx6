#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;
  
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_history(void)
{
  char* buf;
  int historyId;

  if(argptr(0, &buf, 128) < 0 || argint(1, &historyId) < 0) // fill parameter 0 and parameter 1 from function signature
		return -1;
  return history(buf, historyId);
}

int
sys_wait2(void)
{
  int* retime = 0;
  int* rutime = 0;
  int* stime  = 0;
  
  if (argptr(0, (void*)&retime, sizeof(int)) < 0 || argptr(1, (void*)&rutime, sizeof(int)) < 0 || argptr(2, (void*)&stime, sizeof(int)) < 0)
    return -1;
  return wait2(retime, rutime, stime);
}

int
sys_wait3(void)
{
  int* retime = 0;
  int* rutime = 0;
  int* stime  = 0;
  int* ctime = 0;
  int *priority = 0;

  if (argptr(0, (void*)&retime, sizeof(int)) < 0 || argptr(1, (void*)&rutime, sizeof(int)) < 0 || argptr(2, (void*)&stime, sizeof(int)) < 0 || argptr(3, (void*)&ctime, sizeof(int)) < 0 || argptr(4, (void*)&priority, sizeof(int)) < 0)
    return -1;
  return wait3(retime, rutime, stime, ctime, priority);
}

int
sys_set_prio(void)
{
  int priority;

  if(argint(0, &priority) < 0) // fill parameter priority
    return -1;
  return set_prio(priority);
}

int
sys_yield(void)
{
  yield();
  return 0;
}
