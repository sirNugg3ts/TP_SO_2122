#include "stdio.h"
#include "string.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "balcao.h"
#include "medico.h"

/*
void apresentaMenu() {
    printf("\n");
    printf("===== Balcao ====\n");
    printf("= utentes\n= especialistas\n= delut X\n= delesp X\n= freq N\n= encerra\n");
    printf("=================");
}*/

int obtemVariaveisAmbiente(struct Balcao* balcao){
    //FUNCIONA

    //Obtenção da variável de ambiente MAXCLIENTES
    char *env;
    env = getenv("MAXCLIENTS");
    if(env != NULL){
        if(atoi(env) < 0 ){
            perror("\nErro - A variavel de ambiente MAXCLIENTS apresenta um valor invalido");
            return -1;
        }else {
            balcao->N = atoi(env);
        }
    }else{
        perror("\nErro - qqq A variavel de ambiente necessaria MAXClIENTES nao se encontra definida");
        return -1;
    }
    env = NULL;


    //Obtenção da variável de ambiente MAXMEDICOS
    env = getenv("MAXMEDICOS");
    if(env != NULL){
        if(atoi(env) < 0 ){
            perror("\nErro - A variavel de ambiente MAXMEDICOS apresenta um valor invalido");
            return -1;
        }else {
            balcao->M = atoi(env);
        }
    }else{
        perror("\nErro - A variavel de ambiente necessaria MAXMEDICOS nao se encontra definida");
        return -1;
    }

    printf("\n--- Variaveis de Ambiente recebidas ---\n"
           "MAXCLIENTES - %d \n"
           "MAXMEDICOS - %d\n"
                  "---------------------------------------\n\n",balcao->N,balcao->M);

    return 0;
}

int inicializaStruct(struct Balcao* balcao){

    //TODO: por testar

    //inicializar fila de espera de cada especialidade
    for (int i = 0; i < ESPECIALIDADES; ++i) {
        balcao->FilaDeEspera[i] = 0;
    }

}

int main(int argc, char*argv[]) {

    struct Balcao balcao; //estrutura que guarda a informação necessária ao balcão

    //fork stuff
    int id, p;
    int balcao_to_classificador[2];
    int classificador_to_balcao[2];

    pid_t pidChild;

    int rd;
    int rdup;
    int wdup;
    char *eol;

    //comunicação com o Classificador

    char descricaoProblema[50];
    char resposta[50];


    //inicialização
    if( obtemVariaveisAmbiente(&balcao) == -1)
        return -1;
    inicializaStruct(&balcao);


    //criar pipe balcao -> classificador
    if(pipe(balcao_to_classificador) < 0){
        fprintf(stderr,"Erro ao criar pipe balcao -> classificador");
        return -1;
    }

    if(pipe(classificador_to_balcao) < 0){
        fprintf(stderr,"Erro ao cirar pipe classificador -> balcao");
        return -1;
    }

    switch(pidChild = fork()){
        
    }



}


