#ifndef MEDICO_H
#define MEDICO_H

#include "utils.h"


typedef struct especialista{
    pid_t pid;
    char nomeMedico[MAX_STRING_SIZE];
    char especialidade[MAX_STRING_SIZE];
    int ocupado;
    struct especialista* next;
    pid_t pidServer;
    int missedHeartbeats;

}Especialista, *pEspecialista;


#endif
