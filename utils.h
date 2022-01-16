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
#include <signal.h>
#include <pthread.h>

#define MAX_STRING_SIZE 256

#define SERVER_FIFO "/tmp/balc_fifo"
#define SERVER_FIFO_FOR_MEDICS "/tmp/balc_fifo_medics"
#define CLIENT_FIFO "/tmp/client_fifo_%d"
#define MEDICO_FIFO "/tmp/medico_fifo_%d"
#define BALCAO_COMMANDS "/tmp/balcao_commands"

typedef struct{
    pid_t sender;
    char msg[MAX_STRING_SIZE];
} MSG;

#endif