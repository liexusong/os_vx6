#include "types.h"
#include "stat.h"
#include "user.h"

void
f0(void) //CPU
{
  int i, j; 
  for (i = 0; i < 100; i++){
    for (i = 0; i < 1000000; i++){} 
  } 
}

void
f1(void){ //S-CPU

  int i, j; 
  for (i = 0; i < 100; i++){
    for (i = 0; i < 1000000; i++){} 
    yield();
  } 
}

void
f2(void){ //IO
  int i, j; 
  for (i = 0; i < 100; i++){
    sleep(1);
  } 
}

int
main(int argc, char *argv[])
{
  if (argc != 1)
    printf("No number was entered");
  else {
    int num = 3 * atoi(argv[1]);
    int i;
    int pid;

    for(i = 0; i < num; i++){
      if ((pid = fork()) == 0) { // son
          
          pid = getpid();
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

          // print statistics 
          exit(0);
      }
      else if (pid > 0) {       // parent 
          int retime;
          int rutime;
          int stime;

          wait2(&retime, &rutime, &stime);
      }
      else {                    // fork failed
          printf("fork() failed!\n");
          
      }
      printf(1, "%s%s", argv[i], i+1 < argc ? " " : "\n");
    }
  }
  exit();
}
