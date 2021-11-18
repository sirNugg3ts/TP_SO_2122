#ifndef MEDICO_H
#define MEDICO_H

#include "utils.h"


typedef struct especialista{
    char nomeMedico[MAX_STRING_SIZE];
    char especialidade[MAX_STRING_SIZE];
    int isAlive;

}Especialista, *pEspecialista;


#endif
