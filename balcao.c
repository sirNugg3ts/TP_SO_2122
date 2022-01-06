#include <pthread.h>
#include "balcao.h"
#include "utils.h"
#include "cliente.h"

typedef struct {
    int *fd_balcao_classificador;
    int *fd_classificador_balcao;
    int fdServer;
    int stopReceiving;
} DADOS_REG_UTENTES;

void apresentaMenu() {
    printf("\n");
    printf("===== Balcao ====\n");
    printf("= utentes\n= especialistas\n= delut X\n= delesp X\n= freq N\n= encerra\n");
    printf("=================\n");
}

int obtemVariaveisAmbiente(struct Balcao *balcao) {

    //Obtenção da variável de ambiente MAXCLIENTES

    char *env;
    env = getenv("MAXCLIENTES");
    if (env != NULL) {
        if (atoi(env) < 0) {
            fprintf(stderr, "\nErro - A variavel de ambiente MAXCLIENTS apresenta um valor invalido");
            return -1;
        } else {
            balcao->N = atoi(env);
        }
    } else {
        fprintf(stderr, "\nErro - A variavel de ambiente necessaria MAXClIENTES nao se encontra definida");
        return -1;
    }

    env = NULL;

    //Obtenção da variável de ambiente MAXMEDICOS
    env = getenv("MAXMEDICOS");
    if (env != NULL) {
        if (atoi(env) < 0) {
            fprintf(stderr, "\nErro - A variavel de ambiente MAXMEDICOS apresenta um valor invalido");
            return -1;
        } else {
            balcao->M = atoi(env);
        }
    } else {
        fprintf(stderr, "\nErro - A variavel de ambiente necessaria MAXMEDICOS nao se encontra definida");
        return -1;
    }

    printf("\n--- Variaveis de Ambiente recebidas ---\n"
           "MAXCLIENTES - %d \n"
           "MAXMEDICOS - %d\n"
           "---------------------------------------\n\n", balcao->N, balcao->M);

    return 0;
}

void inicializaStruct(struct Balcao *balcao) {
    balcao->nClienteLigados = 0;
    balcao->nMedicosLigados = 0;
    for (int i = 0; i < 5; ++i)
        balcao->filaDeEspera[i] = 0;
}

void *recebeUtentes(void *Dados) {
    DADOS_REG_UTENTES *dadosRegUtentes = (DADOS_REG_UTENTES *) Dados;
    //TODO: processamento de vários utentes, de momento apenas lê um
    //TODO: Verificar se o balcao ta cheio

    while (dadosRegUtentes->stopReceiving) {
        //read pipe
        Utente u1;
        int size = read(dadosRegUtentes->fdServer, &u1, sizeof(u1));
        if (size > 0) {
            printf("\nNovo Utente");
            char CLIENT_FIFO_FINAL[MAX_STRING_SIZE];
            char *token;

            //recebeu com sucesso
            write(dadosRegUtentes->fd_balcao_classificador[1], u1.sintomas, strlen(u1.sintomas));
            read(dadosRegUtentes->fd_classificador_balcao[0], u1.especialidadeAtribuida,
                 sizeof(u1.especialidadeAtribuida));
            sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, u1.pid);

            //split

            token = strtok(u1.especialidadeAtribuida, " "); //
            strcpy(u1.especialidadeAtribuida, token);

            while (token != NULL) {
                printf(" %s\n", token); //debug
                token = strtok(NULL, "\0");
            }

            u1.prioridadeAtribuida = atoi(u1.especialidadeAtribuida);

            int fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
            int size2 = write(fdResposta, &u1, sizeof(u1));
            close(fdResposta);
        }
    }
}


int main(int argc, char *argv[]) {
    //TODO: 1 - Correctly clean all pipes

    struct Balcao balcao; //estrutura que guarda a informação necessária ao balcão
    char comando[MAX_STRING_SIZE];
    int fdServer, fd_balcao_classificador[2], fd_classificador_balcao[2];

    fflush(stdout);
    fflush(stdin);

    //iniciar pipe balcao
    if (mkfifo(SERVER_FIFO, 0777) == -1) {
        if (errno == EEXIST) {
            fprintf(stderr, "\nO pipe ja existe -> MEDICALso_server ");
            exit(1);
        }
        fprintf(stderr, "\nErro ao criar pipe -> MEDICALso_server");
        exit(1);
    }

    //FD servidor
    fdServer = open(SERVER_FIFO, O_RDONLY);
    if (fdServer == -1) {
        fprintf(stderr, "\nErro ao abrir FIFO -> MEDICALso_server");
        unlink(SERVER_FIFO);
        exit(2);
    }

    //inicialização
    if (obtemVariaveisAmbiente(&balcao) == -1) {
        unlink(SERVER_FIFO);
        exit(3);
    }
    inicializaStruct(&balcao);


    //File Descriptors para comunicação com o Classificador

    if (pipe(fd_balcao_classificador) == -1 || pipe(fd_classificador_balcao) == -1) {
        fprintf(stderr, "\nErro - Nao foi possivel criar pipes para o classificador\n");
        unlink(SERVER_FIFO);
        exit(4);
    }

    //Run Classificador
    switch (fork()) {
        case -1:{
            //error fork
            fprintf(stderr, "\nErro - Nao foi possivel criar fork\n");

            //clean pipes
            unlink(SERVER_FIFO);
            exit(5);
        }
        case 0: { // child - run classificador

            // write 1 -> 0 read
            close(STDIN_FILENO);
            dup(fd_balcao_classificador[0]);
            close(fd_balcao_classificador[0]);
            close(fd_balcao_classificador[1]);


            close(STDOUT_FILENO);
            dup(fd_classificador_balcao[1]);
            close(fd_classificador_balcao[1]);
            close(fd_classificador_balcao[0]);

            if (execl("classificador", "classificador", NULL) == -1) {
                fprintf(stderr, "\nNao foi possivel iniciar o classificador");
                //clean pipes
                unlink(SERVER_FIFO);
                exit(6);
            }
            fprintf(stderr, "\nErro ao correr classificador");
            //clean pipes
            unlink(SERVER_FIFO);
            exit(7);
        }
        default: {//parent

            close(fd_balcao_classificador[0]);
            close(fd_classificador_balcao[1]);

            pthread_t threadRecebeUtentes;
            DADOS_REG_UTENTES dados;
            dados.fdServer = fdServer;
            dados.fd_classificador_balcao = fd_classificador_balcao;
            dados.fd_balcao_classificador = fd_balcao_classificador;
            dados.stopReceiving = 1;

            if (pthread_create(&threadRecebeUtentes, NULL, &recebeUtentes, &dados) != 0) {
                fprintf(stderr, "\nErro ao criar thread para receber utentes\nA terminar...");
                exit(1);
            }

            //input teclado
            do {
                fgets(comando, MAX_STRING_SIZE - 1, stdin);
                comando[strcspn(comando, "\n")] = 0;
                //TODO: Inserir o resto dos comandos
                if (strcmp(comando, "help") == 0) {
                    apresentaMenu();
                } else if (strcmp(comando, "encerra") == 0) {
                    dados.stopReceiving = 0; // parar de aceitar utentes
                    //TODO: Notificar que o balcao vai encerrar
                    //TODO: exit gracefully
                    break;
                }
            } while (1);
            pthread_join(threadRecebeUtentes,NULL);
            break;
        }
    }
}