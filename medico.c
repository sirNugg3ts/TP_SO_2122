#include "stdio.h"
#include "string.h"
#include <stdlib.h>
#include <unistd.h>
#include "balcao.h"
#include "utils.h"
#include <signal.h>

void heartbeat(int signal, siginfo_t *i,void *v){
    //TODO: notificar balcao
}



int main(){

    struct sigaction heartbeat_signal;

    heartbeat_signal.sa_handler = heartbeat;
    heartbeat_signal.sa_flags = SA_RESTART|SA_SIGINFO;

    //SA_RESTART - para retomar scanf
    //SA_SIGINFO - ???

    sigaction(SIGALRM,&heartbeat_signal,NULL);


}