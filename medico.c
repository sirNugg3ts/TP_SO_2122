#include "stdio.h"
#include "string.h"
#include <stdlib.h>
#include <unistd.h>
#include "balcao.h"
#include "utils.h"
#include <signal.h>
#include <pthread.h>
#include "medico.h"

char MEDICO_FIFO_FINAL[MAX_STRING_SIZE];

void sigint(int s)
{
    int fd = open(BALCAO_COMMANDS, O_WRONLY);
    if (fd == -1) {
        fprintf(stderr, "\nNao foi possivel avisar o balcao");
        exit(EXIT_FAILURE);
    } else {
        MSG msg;
        char comando[MAX_STRING_SIZE], final[MAX_STRING_SIZE];
        strcpy(comando, "ENCERRA %d");
        sprintf(final, comando, getpid());
        strcpy(msg.msg, final);
        int size = write(fd, &msg, sizeof(MSG));
        if (size < 0) {
            printf("\nErro ao informar balcao \n");
            exit(EXIT_FAILURE);
        }
        close(fd);
    }
    unlink(MEDICO_FIFO_FINAL);
    exit(EXIT_SUCCESS);
}
void inicializaEstrutura(int argc, char *argv[], pEspecialista especialista) {
    if (argc < 3) {
        fprintf(stderr, "Nao foi indicado nome ou especialidade");
        exit(-1);
    }

    if (strcmp(argv[2], "oftalmologia") != 0 &&
        strcmp(argv[2], "neurologia") != 0 &&
        strcmp(argv[2], "estomatologia") != 0 &&
        strcmp(argv[2], "ortopedia") != 0 &&
        strcmp(argv[2], "geral") != 0) {
        fprintf(stderr, "A especialidade indicada nao existe\n");
        exit(-1);
    }

    strcpy(especialista->nomeMedico, argv[1]);
    strcpy(especialista->especialidade, argv[2]);
    especialista->pid = getpid();
}


void *heartbeat(void *Dados) {
    do{
        int fd = open(BALCAO_COMMANDS,O_WRONLY);
        if(fd == -1){
            fprintf(stderr,"\nNao foi possivel avisar o balcao");
            exit(EXIT_FAILURE);
        }else{
            MSG msg;
            char comando[MAX_STRING_SIZE],final[MAX_STRING_SIZE];
            strcpy(comando,"HEARTBEAT");
            sprintf(final,comando,getpid());
            strcpy(msg.msg,final);
            msg.sender = getpid();
            int size = write(fd,&msg, sizeof(MSG));
            if (size < 0) {
                printf("\nErro ao informar balcao \n");
                exit(EXIT_FAILURE);
            }
            close(fd);
        }
        sleep(20);
    }while(1);



}

int main(int argc, char *argv[]) {
    Especialista e1;
    int fdServer, fdEspecialista;

    inicializaEstrutura(argc, argv, &e1);

    signal(SIGINT,sigint);



    sprintf(MEDICO_FIFO_FINAL, MEDICO_FIFO, getpid());

    //fazer pipe resposta
    if (mkfifo(MEDICO_FIFO_FINAL, 0777) == -1) {
        if (errno == EEXIST)
            fprintf(stderr, "\nFIFO ja existe");
        fprintf(stderr, "\nErro ao abrir FIFO");
        exit(-1);
    }

    //verificar se o balcao esta a correr
    if (access(SERVER_FIFO_FOR_MEDICS, F_OK) != 0) {
        fprintf(stderr, "\nO balcao nao se encontra em executamento");
        return 1;
    }


    fdServer = open(SERVER_FIFO_FOR_MEDICS, O_WRONLY);
    if (fdServer == -1) {
        fprintf(stderr, "\nErro ao abrir pipe servidor");
        exit(-1);
    }
    int size = write(fdServer, &e1, sizeof(e1));
    if (size < 0) {
        printf("\nErro ao escrever a estrutura para o pipe \n");
        exit(1);
    }

    fdEspecialista = open(MEDICO_FIFO_FINAL, O_RDONLY);
    if (fdEspecialista == -1) {
        fprintf(stderr, "\nErro ao abrir pipe servidor");
        exit(-1);
    }

    int size1 = read(fdEspecialista, &e1, sizeof(e1));

    if (size1 < 0) {
        printf("\nErro ao escrever a estrutura para o pipe \n");
        exit(1);
    }
    if (strcmp(e1.nomeMedico, "SERVERFULL") == 0) {
        fprintf(stderr, "\nServidor cheio");
        fflush(stderr);
        close(fdServer);
        unlink(MEDICO_FIFO_FINAL);
        exit(1);
    }
    close(fdServer);


    fd_set read_fds;
    pthread_t threadHeartbeat;

    int pidServer;
    pidServer = e1.pidServer;


    if (pthread_create(&threadHeartbeat, NULL, &heartbeat, NULL) != 0) {
        fprintf(stderr, "\nErro ao criar thread para receber utentes\nA terminar...");
        exit(1);
    }

    char input[MAX_STRING_SIZE];

    MSG msgServer;
    fprintf(stdout, "A espera de utentes\n");
    fflush(stdout);

    char UTENTE_FIFO[MAX_STRING_SIZE];

    fdEspecialista = open(MEDICO_FIFO_FINAL, O_RDWR | O_NONBLOCK);


    MSG msg;
    int flag = 0;
    do {

        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        if (flag == 0) {
            fprintf(stdout, "A espera de utentes\n");
            fflush(stdout);
        }

        FD_ZERO(&read_fds);
        FD_SET(0, &read_fds);
        FD_SET(fdEspecialista, &read_fds);

        select(fdEspecialista + 1, &read_fds, NULL, NULL, NULL);

        if (FD_ISSET(0, &read_fds)) //enviar mensagem para o pipe do cliente
        {
            if (flag) //conectado
            {
                //TODO VERIFICAR ADEUS
                fgets(input, 99, stdin);
                strcpy(msg.msg, input);
                //enviar texto para cliente
                msg.sender = getpid();
                int fdCliente = open(UTENTE_FIFO, O_WRONLY);
                write(fdCliente, &msg, sizeof(msg));
                close(fdCliente);
                if (strcmp(input, "adeus\n") == 0) {
                    flag = 0;
                    int fd = open(BALCAO_COMMANDS, O_RDWR | O_NONBLOCK);
                    if (fd == -1) {
                        fprintf(stderr, "\nNao foi possivel avisar o balcao");
                        exit(EXIT_FAILURE);
                    }
                        MSG msg;
                        char comando[MAX_STRING_SIZE], final[MAX_STRING_SIZE];
                        strcpy(comando, "FREE %d");
                        sprintf(final, comando, getpid());
                        strcpy(msg.msg, final);
                        int size = write(fd, &msg, sizeof(MSG));
                        if (size < 0) {
                            printf("\nErro ao informar balcao \n");
                            exit(EXIT_FAILURE);
                        }
                        close(fd);
                    }
                } else {
                    fgets(input, 99, stdin);
                    if (strcmp(input, "encerra\n") == 0) {
                        kill(getpid(),SIGINT);
                    }
                }
            }
            if (FD_ISSET(fdEspecialista, &read_fds))//recebeu uma mensagem
            {
                int sizeReadMessage = read(fdEspecialista, &msgServer, sizeof(MSG));
                if(strcmp(msgServer.msg,"DELUT") == 0 && msgServer.sender == pidServer){
                    printf("\n[SERVER]O servidor removeu-o do sistema");
                    strcpy(msg.msg,"adeus\n");
                    msg.sender = getpid();
                        int fdCliente = open(UTENTE_FIFO, O_WRONLY);
                        write(fdCliente, &msg, sizeof(msg));
                        close(fdCliente);
                    kill(getpid(),SIGINT);
                }else {
                    if (flag == 0) {
                        if (sizeReadMessage > 0) {
                            sprintf(UTENTE_FIFO, CLIENT_FIFO, atoi(msgServer.msg));
                        }
                        printf("\nConectado ao utente:\n");
                        fflush(stdout);
                        flag = 1;
                    }else {
                        //recebeu texto do medico
                        if (sizeReadMessage > 0) {
                            fprintf(stdout, "Cliente: %s \n", msgServer.msg);
                            if (strcmp(msgServer.msg, "adeus\n") == 0) {
                                printf("O utente terminou a consulta !!");
                                flag = 0;
                            }
                        }
                    }
                }
            }
        }while (strcmp(input, "sair\n") != 0);
    kill(getpid(),SIGINT);
    }
