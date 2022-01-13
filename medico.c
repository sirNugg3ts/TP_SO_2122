#include "stdio.h"
#include "string.h"
#include <stdlib.h>
#include <unistd.h>
#include "balcao.h"
#include "utils.h"
#include <signal.h>
#include "medico.h"

void inicializaEstrutura(int argc, char* argv[],pEspecialista especialista){
    if(argc < 3){
        fprintf(stderr,"Nao foi indicado nome ou especialidade");
        exit(-1);
    }

    if(strcmp(argv[2],"oftalmologia")!=0 &&
        strcmp(argv[2],"neurologia")!=0 &&
        strcmp(argv[2],"estomatologia")!=0 &&
        strcmp(argv[2],"ortopedia")!=0 &&
        strcmp(argv[2],"geral")!=0){
        fprintf(stderr,"A especialidade indicada nao existe\n");
        exit(-1);
    }

    strcpy(especialista->nomeMedico,argv[1]);
    strcpy(especialista->especialidade,argv[2]);
    especialista->pid = getpid();
}

pid_t pidServidor;

void handleAlarm(int s,siginfo_t *info,void *c){

}

int main(int argc, char* argv[]){
    Especialista e1;
    int fdServer, fdEspecialista;

    inicializaEstrutura(argc,argv,&e1);

    char MEDICO_FIFO_FINAL[MAX_STRING_SIZE];

    sprintf(MEDICO_FIFO_FINAL,MEDICO_FIFO,getpid());

    //fazer pipe resposta
    if(mkfifo(MEDICO_FIFO_FINAL,0777) == -1){
        if(errno == EEXIST)
            fprintf(stderr,"\nFIFO ja existe");
        fprintf(stderr,"\nErro ao abrir FIFO");
        exit(-1);
    }

    //verificar se o balcao esta a correr
    if(access(SERVER_FIFO_FOR_MEDICS,F_OK) != 0){
        fprintf(stderr,"\nO balcao nao se encontra em executamento");
        return 1;
    }


    fdServer = open(SERVER_FIFO_FOR_MEDICS,O_WRONLY );
    if(fdServer == -1){
        fprintf(stderr,"\nErro ao abrir pipe servidor");
        exit(-1);
    }
    int size = write(fdServer,&e1,sizeof(e1));
    if(size<0){
        printf("\nErro ao escrever a estrutura para o pipe \n");
        exit(1);
    }

    fdEspecialista = open(MEDICO_FIFO_FINAL,O_RDONLY );
    if(fdEspecialista == -1){
        fprintf(stderr,"\nErro ao abrir pipe servidor");
        exit(-1);
    }

    int size1 = read(fdEspecialista,&e1, sizeof(e1));

    if(size1 < 0){
        printf("\nErro ao escrever a estrutura para o pipe \n");
        exit(1);
    }
    if(strcmp(e1.nomeMedico,"SERVERFULL") == 0){
        fprintf(stderr,"\nServidor cheio");
        fflush(stderr);
        close(fdServer);
        unlink(MEDICO_FIFO_FINAL);
        exit(1);
    }
    close(fdServer);


    fd_set read_fds;

    MSG msgServer;

    fprintf(stdout,"A espera de utentes\n");
    fflush(stdout);

    char UTENTE_FIFO[MAX_STRING_SIZE];

    fdEspecialista = open(MEDICO_FIFO_FINAL,O_RDONLY );
    int sizeReadMessage = read(fdEspecialista,&msgServer, sizeof(MSG));
    if(sizeReadMessage > 0){
        sprintf(UTENTE_FIFO,CLIENT_FIFO,atoi(msgServer.msg));
    }

    printf("\nConectado ao utente:\n");
    fflush(stdout);


    do{

        char input[MAX_STRING_SIZE];
        MSG msg;
        FD_ZERO(&read_fds);
        FD_SET(0,&read_fds);
        FD_SET(fdEspecialista,&read_fds);

        int nfd = select(fdEspecialista+1,&read_fds,NULL,NULL,NULL);

        if(FD_ISSET(0,&read_fds)) //enviar mensagem para o pipe do cliente
        {
            fgets(input,99,stdin);
            strcpy(msg.msg,input);
            //enviar texto para cliente
            msg.sender = getpid();
            int fdCliente = open(UTENTE_FIFO,O_WRONLY);
            write(fdCliente,&msg,sizeof(msg));
            close(fdCliente);
        }
        if(FD_ISSET(fdEspecialista,&read_fds))//recebeu uma mensagem
        {
            //recebeu texto do medico
            int readSize = read(fdEspecialista,&msg, sizeof(MSG));
            if(readSize > 0){
                fprintf(stdout,"Cliente: %s \n",msg.msg);
            }
        }
    }while(1);

    //à espera de contacto





}