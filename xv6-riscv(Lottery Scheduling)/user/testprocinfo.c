#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/pstat.h"
#include "user/user.h"

int main(int argc, char *argv[]){
    settickets(10);
    struct pstat stat;
    getpinfo(&stat);
    printf("PID | In Use | Original Tickets | Current Tickets | Time Slices\n");
    for(int i = 0; i < NPROC; i++){
        if(stat.inuse[i] != 0){
            printf("%d\t%d\t\t%d\t\t%d\t\t%d\n", stat.pid[i], stat.inuse[i], stat.tickets_original[i], stat.tickets_current[i], stat.time_slices[i]);
        }
    }
    return 0;
}