#ifndef MEDICO_H
#define MEDICO_H

#include "utils.h"


typedef struct especialista{
    pid_t pid;
    char nomeMedico[MAX_STRING_SIZE];
    char especialidade[MAX_STRING_SIZE];
    int isAlive;
    struct especialista* next;

}Especialista, *pEspecialista;


#endif
