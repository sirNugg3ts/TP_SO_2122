#ifndef UTILS
#define UTILS

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#define MAX_STRING_SIZE 256
#define HEARTBEAT 20

#define SERVER_FIFO "MEDICALso_server"
#define CLIENT_FIFO "MEDICALso_client_%d"
#define MEDICO_FIFO "MEDICALso_medico_%d"

typedef struct{
    pid_t sender;
    char msg[MAX_STRING_SIZE]; 

} MSG;


#endif