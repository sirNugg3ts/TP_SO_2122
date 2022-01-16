#ifndef BALCAO
#define BALCAO

#include "medico.h"
#include "cliente.h"

struct Balcao{
    int N; //valor maximo de clientes EM SIMULTANEO
    int M; //valor maximo de medicos EM SIMULTANEO

    int nClienteLigados;
    int nMedicosLigados;

    int nUtentesEspecialidade[5];
    //oftalmologia neurologia estomatologia ortopedia geral
};

typedef struct utenteContainer {
    pUtente first;
} *pUtenteContainer;

struct dadosManager{
    pUtenteContainer listaUtentes;
    pEspecialista listaEspecialistas;
    int *nUtentesEspecialidade;
    int stop;
    pthread_mutex_t *mutexListaUtentes;
    pthread_mutex_t *mutexListaMedicos;
    pthread_mutex_t *mutexPrints;
};

struct dadosHeartbeat{
    pEspecialista listaEspecialistas;
    int stop;
    pUtenteContainer listaUtentes;
    pthread_mutex_t *mutexPrints;
    pthread_mutex_t *mutexEspecialistas;
};

typedef struct {
    int *fd_balcao_classificador;
    int *fd_classificador_balcao;
    int stopReceiving;
    int *nUtentesLigados;
    int nMaxClientes;
    pUtenteContainer listaUtentes;
    int *nUtentesEspecialidade;
    pEspecialista listaEspecialistas;
    pthread_mutex_t *mutexPrints;
    pthread_mutex_t *mutexListaUtentes;
    pthread_mutex_t *mutexListaMedicos;
    pthread_cond_t *ptrCond;
} DADOS_REG_UTENTES;

typedef struct{
    int stopReceiving;
    int *nMedicosLigados;
    int nMaxMedicos;
    pthread_mutex_t  *mutexPrints;
    pEspecialista lista;
    pthread_mutex_t *mutexListaMedicos;
    pthread_cond_t *ptrCond;
} DADOS_REG_MEDICOS;

struct dadosStatus {
    int *ocupacao;
    int *timeFreq;
    int stopShowing;
};

#endif