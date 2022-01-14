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


    //obtem sintomas
    fprintf(stdout,"Indique os seus sintomas: ");
    fgets(u1.sintomas,MAX_STRING_SIZE-1,stdin);
    u1.atendido = 0;

    fdServer = open(SERVER_FIFO,O_WRONLY );
    if(fdServer==-1) {
        printf("\nErro ao abrir o NamedPipe do Balcao\n");
        exit(1);
    }

    int size = write(fdServer,&u1,sizeof(u1));
    if(size<0){
        printf("\nErro ao escrever a estrutura para o pipe \n");
        exit(1);
    }

    fdCliente = open(CLIENT_FIFO_FINAL,O_RDONLY );

    int size2 = read(fdCliente,&u1,sizeof(u1));
    if(size2<0){
        printf("\nErro ao ler a estrutura para o pipe \n");
    }
    if(strcmp(u1.nomeUtente,"SERVERFULL")==0){
        fprintf(stdout,"O servidor esta cheio");
        exit(0);
    }
    if(strcmp(u1.nomeUtente,"ESPECIALIDADEFULL")==0){
        fprintf(stdout,"Foi lhe atribuido a especialidade %s, mas esta ja se encontra cheia",u1.especialidadeAtribuida);
        exit(0);
    }


    MSG msgStatus;
    read(fdCliente,&msgStatus,sizeof(MSG));
    fprintf(stdout,"%s\n",msgStatus.msg);
    fflush(stdout);
    close(fdCliente);

    fprintf(stdout,"\nA espera de medico...");
    fflush(stdout);


    MSG msg;
    fdCliente = open(CLIENT_FIFO_FINAL,O_RDONLY );
    int sizeReadMassage = read(fdCliente,&msg, sizeof(MSG));
    close(fdCliente);
    char MEDICO_FIFO_FINAL[MAX_STRING_SIZE];
    char input[MAX_STRING_SIZE];

    if(sizeReadMassage>0){
        sprintf(MEDICO_FIFO_FINAL,MEDICO_FIFO,atoi(msg.msg));
    }
    printf("\nConectado ao medico\n");
    fflush(stdout);

    fdCliente = open(CLIENT_FIFO_FINAL,O_RDONLY );
    do{

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(0, &read_fds);
        FD_SET(fdCliente, &read_fds);

        select(fdCliente+1,&read_fds,NULL,NULL,NULL);

        if(FD_ISSET(0,&read_fds)){
            //enviar texto para medico
            fgets(input,99,stdin);
            strcpy(msg.msg,input);
            msg.sender = getpid();
            int fdMedico = open(MEDICO_FIFO_FINAL,O_WRONLY );
            write(fdMedico,&msg,sizeof(msg));
            close(fdMedico);
        }
        if(FD_ISSET(fdCliente,&read_fds)){
            //recebeu texto do medico
            int readSize = read(fdCliente,&msg, sizeof(MSG));
            if(readSize > 0){
                fprintf(stdout,"Medico: %s",msg.msg);
            }
            if(strcmp(msg.msg,"adeus\n")==0){
                printf("O médico terminou a consulta !!");

            }
        }
    } while (strcmp(msg.msg,"adeus\n")!=0 || strcmp(input,"adeus\n")!=0);

    close(fdServer);
    unlink(CLIENT_FIFO_FINAL);

return 0;
}
