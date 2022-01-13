#ifndef BALCAO
#define BALCAO

//valores constantes
#define MAXUTENTESESPECIALIDADE 5
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "medico.h"
#include "cliente.h"

struct Balcao{
    int N; //valor maximo de clientes EM SIMULTANEO
    int M; //valor maximo de medicos EM SIMULTANEO

    int nClienteLigados;
    int nMedicosLigados;

    int nUtentesEspecialidade[MAXUTENTESESPECIALIDADE];
    //oftalmologia neurologia estomatologia ortopedia geral
};

typedef struct utenteContainer {
    pUtente first;
    pthread_mutex_t list_mutex;
} *pUtenteContainer;

struct dadosManager{
    pUtenteContainer listaUtentes;
    pEspecialista listaEspecialistas;
    int *nUtentesEspecialidade;
    int stop;
};

struct dadosHeartbeat{
    pEspecialista listaEspecialistas;
    int stop;
    int fdHeartbeat;
};

typedef struct {
    int *fd_balcao_classificador;
    int *fd_classificador_balcao;
    int fdServer;
    int stopReceiving;
    int *nUtentesLigados;
    int nMaxClientes;
    pUtenteContainer listaUtentes;
    int *nUtentesEspecialidade;
    pEspecialista listaEspecialistas;
    pthread_mutex_t *mutexPrints;
} DADOS_REG_UTENTES;

typedef struct{
    int fdServer;
    int stopReceiving;
    int *nMedicosLigados;
    int nMaxMedicos;
    pthread_mutex_t  *mutexPrints;
    pEspecialista lista;

} DADOS_REG_MEDICOS;



#endif