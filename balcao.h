#ifndef BALCAO
#define BALCAO

//valores constantes
#define MAXUTENTESESPECIALIDADE 5
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
struct Balcao{
    int N; //valor maximo de clientes EM SIMULTANEO
    int M; //valor maximo de medicos EM SIMULTANEO

    int nClienteLigados;
    int nMedicosLigados;

    int nUtentesEspecialidade[MAXUTENTESESPECIALIDADE];
    //oftalmologia neurologia estomatologia ortopedia geral

};



#endif