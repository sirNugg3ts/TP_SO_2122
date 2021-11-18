#include "cliente.h"
#include "stdio.h"
#include "string.h"

int inicializaEstrutura(int argc, char* argv[],pUtente utente){

    if(argc < 2){
        perror("Nao foi indicado qualquer nome para o utente");
        return -1;
    }

    strcpy(utente->nomeUtente,argv[1]);
    return 0;

}

int main(int argc, char*argv[]){

    Utente u1;
    char sintomas[100];

    if(inicializaEstrutura(argc,argv,&u1)==-1){
        return -1;
    }

    //TODO: Verificar se o balcao esta cheio ou nao

    //obtem sintomas
    fprintf(stdout,"Indique os seus sintomas: ");
    fgets(sintomas,99,stdin);

    //TODO: enviar informação para o balcão - namedpipe

return 0;
}
