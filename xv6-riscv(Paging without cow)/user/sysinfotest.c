#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"


void
sinfo() {
  if (sysinfo() < 0) {
    printf("FAIL: sysinfo failed");
    exit(1);
  }
}


void
testmem() {
  sinfo();

  uint64 a = (uint64)sbrk(PGSIZE * 11);

  sinfo();

  if(a == 0xffffffffffffffff){
    printf("sbrk failed");
    exit(1);
  }

  int pid = fork();

  if(pid == 0){
    printf("Child process\n");
    sinfo();
    exit(0);
  }

  wait(0);


  printf("%p\n", a);

  int *b = (int *)a;
  *b = 10;

  printf("%d\n", *b);

  sinfo();

  
  if((uint64)sbrk(-PGSIZE * 11) == 0xffffffffffffffff){
    printf("sbrk failed");
    exit(1);
  }

  sinfo();
}


int
main(int argc, char *argv[])
{
  testmem();
  exit(0);
}
