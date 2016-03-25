#include "types.h"  
#include "user.h"

//CPU
static int cpu_count = 0;
static int cpu_stime = 0;
static int cpu_retime = 0;
static int cpu_rutime = 0;

static const char* cpu_types[] = { "CPU", "S-CPU", "IO" };

void
f0(void)
{
  int i, j; 
  for (i = 0; i < 100; i++){
    for (j = 0; j < 1000000; j++){} 
  }
}

//S-CPU
static int scpu_count = 0;
static int scpu_stime = 0;
static int scpu_retime = 0;
static int scpu_rutime = 0;

void
f1(void){ 

  int i, j; 
  for (i = 0; i < 100; i++){
    for (j = 0; j < 1000000; j++){} 
    yield();
  } 
}

//IO
static int io_count = 0;
static int io_stime = 0;
static int io_retime = 0;
static int io_rutime = 0;

void
f2(void){ 
  int i; 
  for (i = 0; i < 100; i++){
    sleep(1);
  } 
}

//print avarage statistics for each process group type
void 
printpGroupsAvg(void)
{  
  printf(1, "\n");
  printf(1, "CPU --> Avarage stime:%d\n",    cpu_count == 0 ? 0 : (cpu_stime  / cpu_count));
  printf(1, "CPU --> Avarage retime:%d\n",   cpu_count == 0 ? 0 : (cpu_retime / cpu_count));
  printf(1, "CPU --> Avarage rutime:%d\n\n", cpu_count == 0 ? 0 : (cpu_rutime / cpu_count));

  printf(1, "SCPU --> Avarage stime:%d\n",    scpu_count == 0 ? 0 : (scpu_stime  / scpu_count));
  printf(1, "SCPU --> Avarage retime:%d\n",   scpu_count == 0 ? 0 : (scpu_retime / scpu_count));
  printf(1, "SCPU --> Avarage rutime:%d\n\n", scpu_count == 0 ? 0 : (scpu_rutime / scpu_count));

  printf(1, "IO --> Avarage stime:%d\n",    io_count == 0 ? 0 : (io_stime  / io_count));
  printf(1, "IO --> Avarage retime:%d\n",   io_count == 0 ? 0 : (io_retime / io_count));
  printf(1, "IO --> Avarage rutime:%d\n\n", io_count == 0 ? 0 : (io_rutime / io_count));
}

int
main(int argc, char *argv[])
{
  if (argc < 2){
    printf(1, "No number was entered\n");
    exit();
  }

  int num = 3 * atoi(argv[1]);
  int i;
  int pid;

  for(i = 0; i < num; i++){
    if ((pid = fork()) == 0) { // son
        pid=getpid();
        switch (pid % 3) {
          case 0:
            f0();
            break;
          case 1:
            f1();
            break;
          case 2:
            f2();
            break;
        }       
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
  int wpid;
  int pook = 0;
  while ((wpid = wait2(&retime, &rutime, &stime)) > 0) { // the father waits for all the child proces
    pook++;
    //sum for avarages
    switch (wpid % 3) {
          case 0:
           {
            cpu_count++;        
            cpu_stime  += stime;
            cpu_retime += retime;
            cpu_rutime += rutime;
            break;
          }
          case 1:
          {
            scpu_count++;        
            scpu_stime  += stime;
            scpu_retime += retime;
            scpu_rutime += rutime;
            break;
          }
          case 2: 
          {
            io_count++;        
            io_stime  += stime;
            io_retime += retime;
            io_rutime += rutime;
            break;
          }
    }

    printf(1, "pid:%d  type:%s  stime:%d  rutime:%d  retime:%d\n", 
               wpid, cpu_types[wpid % 3], stime, rutime, retime);      
  }

  // print avarage statistics 
  printpGroupsAvg();
  exit();
}
