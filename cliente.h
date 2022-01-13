#ifndef CLIENTE
#define CLIENTE

#include "utils.h"

typedef struct utente{
    pid_t pid;

    char nomeUtente[MAX_STRING_SIZE];
    char sintomas[MAX_STRING_SIZE];
    char especialidadeAtribuida[MAX_STRING_SIZE];
    struct utente *next;
    int prioridadeAtribuida;
    int atendido;
    char nomeMedico[MAX_STRING_SIZE];
} Utente, *pUtente;


#endif