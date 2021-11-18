//
// Created by diogo on 11/11/2021.
//
#include "cliente.h"
#include "stdio.h"
#include "string.h"

int inicializaEstrutura(int argc, char* argv[],pUtente utente){

    //TODO: testar inicialização

    if(argc < 2){
        perror("Nao foi indicado qualquer nome para o utente");
        return -1;
    }

    strcpy(utente->nomeUtente,argv[1]);
    return 0;

}

int main(int argc, char*argv[]){

    Utente u1;

    if(inicializaEstrutura(argc,argv,&u1)==-1){
        return -1;
    }

return 0;
}
