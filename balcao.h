#ifndef BALCAO
#define BALCAO

//valores constantes
#define MAXUTENTESESPECIALIDADE 5
#define ESPECIALIDADES 5
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
struct Balcao{
    int N; //valor maximo de clientes EM SIMULTANEO
    int M; //valor maximo de medicos EM SIMULTANEO

    int nClienteLigados;
    int nMedicosLigados;

    pid_t filaDeEspera[ESPECIALIDADES]; //o conceito de fila de espera está implementado
                                      //apesar de não sabermos que tipo de dados será usado para representar os clientes

};



#endif