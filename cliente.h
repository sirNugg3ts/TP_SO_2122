#ifndef CLIENTE
#define CLIENTE

#define MAX_NAME_SIZE 100

typedef struct utente{
    char nomeUtente[MAX_NAME_SIZE];
    int desistiu; // morreu ou desistiu do tratamento
    int fila; // se estiver na fila - 1 ; se estiver em tratamento - 0
} Utente, *pUtente;


#endif