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
    int desistiu; // morreu ou desistiu do tratamento
    int fila; // se estiver na fila - 1 ; se estiver em tratamento - 0
} Utente, *pUtente;


#endif