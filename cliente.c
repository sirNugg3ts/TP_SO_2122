#include "utils.h"
#include "cliente.h"

char CLIENT_FIFO_FINAL[MAX_STRING_SIZE];


void sigint_handler(int s, siginfo_t *i,void *v){
    unlink(CLIENT_FIFO_FINAL);
    fprintf(stdout,"\nA terminar cliente...");
    exit(0);
}

int inicializaEstrutura(int argc, char* argv[],pUtente utente){

    if(argc < 2){
        perror("Nao foi indicado qualquer nome para o utente");
        return -1;
    }

    strcpy(utente->nomeUtente,argv[1]);
    utente->pid = getpid();
    return 0;

}

int main(int argc, char*argv[]){
    //TODO: cleaning pipes and stuff

    Utente u1;
    int fdServer, fdCliente;

    if(inicializaEstrutura(argc,argv,&u1)==-1){
        return -1;
    }

    sprintf(CLIENT_FIFO_FINAL,CLIENT_FIFO,getpid());

    //fazer pipe resposta
    if(mkfifo(CLIENT_FIFO_FINAL,0777) == -1){
        if (errno == EEXIST)
            printf("\nFIFO ja existe");
        printf("\nErro ao abrir FIFO");
        exit(1);
    }

    //verificar se o balcao esta a correr
    if(access(SERVER_FIFO,F_OK) != 0){
        fprintf(stderr,"\nO balcao nao se encontra em executamento");
        return 1;
    }

    //sigaction
    struct sigaction sa;
    sa.sa_sigaction = &sigint_handler;
        sa.sa_flags = SA_RESTART|SA_SIGINFO;
    sigaction(SIGINT,&sa,NULL);


    //TODO: Verificar se o balcao esta cheio ou nao

    //obtem sintomas
    fprintf(stdout,"Indique os seus sintomas: ");
    fgets(u1.sintomas,MAX_STRING_SIZE-1,stdin);

    fdServer = open(SERVER_FIFO,O_WRONLY);
    if(fdServer==-1) {
        printf("\nErro ao abrir o NamedPipe do Balcao\n");
        exit(1);
    }
/*
    MSG msg;
    msg.sender = getpid();
    strcpy(msg.msg,"REQUEST isBalcaoFull");

    int size1 = write(fdServer,&msg,sizeof(msg));

    if(size1<1){
        fprintf(stderr,"\nErro ao pedir informacao ao servidor");
        exit(1);
    }else{
        fdCliente = open(CLIENT_FIFO_FINAL,O_RDONLY);
        MSG response;
        int sizeResponse = read(fdCliente,&response,sizeof(response));
        if(strcmp(response.msg,"false")){
            fprintf(stderr,"\nO Balcao encontra-se cheio, a abortar");
            exit(1);

        }
    }

*/
    int size = write(fdServer,&u1,sizeof(u1));
    if(size<0){
        printf("\nErro ao escrever a estrutura para o pipe \n");
        exit(1);
    }

    fdCliente = open(CLIENT_FIFO_FINAL,O_RDONLY);

    int size2 = read(fdCliente,&u1,sizeof(u1));
    if(size2<0){
        printf("\nErro ao ler a estrutura para o pipe \n");
    }
    fprintf(stdout,"\nAtribuido : %s",u1.especialidadeAtribuida);
    fprintf(stdout,"\nPrioridade: %d\n",u1.prioridadeAtribuida);

    close(fdServer);
    unlink(CLIENT_FIFO_FINAL);

return 0;
}
