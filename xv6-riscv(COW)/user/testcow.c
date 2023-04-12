#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"
#include "kernel/memlayout.h"


int fds[2];
char buf[4096];

void
testcopyout()
{
  
  buf[0] = 99;

  for(int i = 0; i < 4; i++){
    if(pipe(fds) != 0){
      printf("pipe() failed\n");
      exit(-1);
    }
    int pid = fork();
    if(pid == 0){
      sleep(1);
      if(read(fds[0], buf, sizeof(i)) != sizeof(i)){
        printf("error: read failed\n");
        exit(1);
      }
      sleep(1);
      int j = *(int*)buf;
      if(j != i){
        printf("error: read the wrong value\n");
        exit(1);
      }
      exit(0);
    }
    if(write(fds[1], &i, sizeof(i)) != sizeof(i)){
      printf("error: write failed\n");
      exit(-1);
    }
  }

  int xstatus = 0;
  for(int i = 0; i < 4; i++) {
    wait(&xstatus);
    if(xstatus != 0) {
      exit(1);
    }
  }

  if(buf[0] != 99){
    printf("error: child overwrote parent\n");
    exit(1);
  }

  printf("ok\n");
}


int
main(int argc, char *argv[]){
  printf("Start\n");
  meminfo();
  uint64 a = (uint64)sbrk(PGSIZE * 10);
  int *b = (int *) a;
  printf("After sbrk\n");
  meminfo();
  int pid = fork();
  if(pid == 0){
    printf("Child\n");
    meminfo();
    printf("%d\n", *b);
    printf("After read\n");
    meminfo();
    *b = 1;
    printf("%d\n", *b);
    printf("After write\n");
    meminfo();
    a += PGSIZE * 2;
    b = (int *) a;
    printf("%d\n", *b);
    printf("After another read\n");
    meminfo();
    *b = 2;
    printf("%d\n", *b);
    printf("After another write\n");
    meminfo();
    exit(0);
  }
  else{
    wait(0);
    printf("%d\n", *b);
    printf("After parent read, not same as child\n");
    meminfo();
    *b = 3;
    printf("%d\n", *b);
    printf("After parent write, should not decrease\n");
    meminfo();
  }
  testcopyout();
  return 0;
}