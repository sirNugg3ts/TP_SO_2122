#include "stdio.h"
#include "string.h"
#include <stdlib.h>
#include <unistd.h>
#include "balcao.h"

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
        fprintf(stderr, "\nErro - qqq A variavel de ambiente necessaria MAXClIENTES nao se encontra definida");
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
    //TODO
    balcao->nClienteLigados = 0;
    balcao->nMedicosLigados = 0;
    for (int i = 0; i < 5; ++i)
        balcao->filaDeEspera[i]=0;

}


int main(int argc, char *argv[]) {

    struct Balcao balcao; //estrutura que guarda a informação necessária ao balcão
    char comando[10];

    //inicialização
    if (obtemVariaveisAmbiente(&balcao) == -1)
        return -1;
    inicializaStruct(&balcao);


    //File Descriptors para comunicação com o Classificador
    int fd_balcao_classificador[2], fd_classificador_balcao[2];

    if (pipe(fd_balcao_classificador) == -1 || pipe(fd_classificador_balcao) == -1) {
        fprintf(stderr, "\nErro - Nao foi possivel criar pipes\n");
        exit(1);
    }

    //Run Classificador
    switch (fork()) {
        case -1: //error fork
            fprintf(stderr, "\nErro - Nao foi possivel criar fork\n");
            exit(2);
            break;
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

            execl("classificador", "classificador", NULL);
            break;

        }
        default: {//parent

            char input[100];
            char output[100];

            close(fd_balcao_classificador[0]);
            close(fd_classificador_balcao[1]);

            while (1) {
                printf("\nIndique os sintomas: ");
                fgets(input, 99, stdin);

                write(fd_balcao_classificador[1], input, strlen(input));
                read(fd_classificador_balcao[0], output, 99);
                fprintf(stdout, "%s", output);

                if (strcmp(input, "#fim\n") == 0)
                    break;

                //just cleaning memory
                fflush(stdout);
                fflush(stdin);
                memset(output, 0, 99);
                memset(input, 0, 99);
            }
            break;
        }
    }

    while (1) {
        //TODO: VERIFICAÇÕES
        //TODO: implementar todos os comandos
        printf("Insira o comando: ");
        fgets(comando, 9, stdin);
        comando[strcspn(comando, "\n")] = 0;
        if (strcmp(comando, "help") == 0) {
            apresentaMenu();
        } else if (strcmp(comando, "encerra") == 0) {
            break;
        }
    }
}


