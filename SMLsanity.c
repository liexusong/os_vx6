#include "types.h"  
#include "user.h"

//CPU-bound Loop
void
CPU_Loop(void)
{
  int i, j; 
  for (i = 0; i < 100; i++){
    for (j = 0; j < 1000000; j++){} 
  }
}

int
main(int argc, char *argv[])
{
  int i;
  int pid;

  // test arguments
  if (argc != 1){
  	printf(1, "Unvaild parameter for SMLSanity test\n");
  	exit();
  }

  // verify shecduler policy
  #ifndef SCHEDFLAG_SML
  	printf(1, "Scheduler policy is not SML\n");
  	exit();
  #endif

  for(i = 0; i < 20; i++){
  	if ((pid = fork()) == 0) { // son
      pid = getpid();
      switch (pid % 3) {
        case 0:
          set_prio(1);  // Process priority = 1
          break;
        case 1:
          set_prio(2);  // Process priority = 2
          break;
        case 2:
          set_prio(3);  // Process priority = 3
          break;
        }   
        
        CPU_Loop();    
        exit();
    }
    else if (pid < 0) {        // fork failed
      printf(1, "fork() failed!\n"); 
      exit();
    }
  }

  int retime = 0;
  int rutime = 0;
  int stime  = 0;
  int ctime = 0;
  int priority = 0;
  int wpid;
  // the father waits for all the child proces
  while ((wpid = wait3(&retime, &rutime, &stime, &ctime, &priority)) > 0) {        
    printf(1,"pid: %d ,priority - %d, Termination time: %d\n", wpid, priority, (ctime+retime+rutime+stime));
  }

  exit();
}