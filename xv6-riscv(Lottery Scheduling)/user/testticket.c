#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/pstat.h"
#include "user/user.h"

int main(int argc, char *argv[]){

    int ticket;

    if(argc == 1){
        ticket = 10;
    }
    else if(argc > 1 && (argv[1][0] < '0' || argv[1][0] > '9')){
        fprintf(2, "Usage: %s ticket_no\n", argv[0]);
        exit(1);
    }
    else{
        ticket = atoi(argv[1]);
    }
    settickets(ticket);
    if(fork() == 0){
        settickets(ticket);
    }
    else{
        settickets(ticket * 2);
    }
    while(1);
    return 0;
}