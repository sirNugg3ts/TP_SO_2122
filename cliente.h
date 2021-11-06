#ifndef CLIENTE
#define CLIENTE

#define MAXCLIENTS 10

typedef struct utente{
    char nomeUtente[50];
    char sintomas[][];
} UTENTE, *pUTENTE;


#endif