#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#pragma ide diagnostic ignored "cert-err34-c"

#include <pthread.h>
#include "balcao.h"
#include "utils.h"
#include "cliente.h"
typedef struct utenteContainer{
    pUtente first;
   pthread_mutex_t list_mutex;
}*pUtenteContainer;

typedef struct {
    int *fd_balcao_classificador;
    int *fd_classificador_balcao;
    int fdServer;
    int stopReceiving;
    int *nUtentesLigados;
    int nMaxClientes;
    pUtenteContainer listaUtentes;
    int *nUtentesEspecialidade;
    pthread_mutex_t mutexPrints;
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
        balcao->nUtentesEspecialidade[i] = 0;
}

void *recebeUtentes(void *Dados) {
    DADOS_REG_UTENTES *dadosRegUtentes;
    dadosRegUtentes = (DADOS_REG_UTENTES *) Dados;

    fprintf(stdout,"\nWaiting for new clients...");
    fflush(stdout);

    while (dadosRegUtentes->stopReceiving) {
        //read pipe
        Utente u1;
        int size = read(dadosRegUtentes->fdServer, &u1, sizeof(u1));

        if (size > 0) {

            char CLIENT_FIFO_FINAL[MAX_STRING_SIZE];
            int fdResposta;

            //DEBUG
            printf("\n/// nligados = %d ; nMax = %d",*dadosRegUtentes->nUtentesLigados,
                   dadosRegUtentes->nMaxClientes);
            fflush(stdout);
            //DEBUG

            if((*dadosRegUtentes->nUtentesLigados) == (dadosRegUtentes->nMaxClientes))//server is full
            {


                strcpy(u1.nomeUtente,"SERVERFULL");
                sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, u1.pid);
                fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                write(fdResposta, &u1, sizeof(u1));
                close(fdResposta);

            }
            else //server is not full
            {
                char bufferClassificador[MAX_STRING_SIZE];
                char *token;

                pUtente newUtente = malloc(sizeof(Utente));

                //DEBUG
                printf("\nNovo Utente");
                printf("\nSintomas recebidos: %s",u1.sintomas);
                fflush(stdout);
                //DEBUG

                //transmitir ao classificador os sintomas
                write(dadosRegUtentes->fd_balcao_classificador[1], u1.sintomas, strlen(u1.sintomas));

                //ler resposta do classificador
                int size1 = read(dadosRegUtentes->fd_classificador_balcao[0],bufferClassificador,sizeof(bufferClassificador));

                //DEBUG
                printf("\nInput classificador: %s",bufferClassificador);
                //DEBUG

                bufferClassificador[size1] = '\0';
                sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, u1.pid);

                //split
                token = strtok(bufferClassificador, " "); //
                strcpy(u1.especialidadeAtribuida, token);

                while (token != NULL) {
                    u1.prioridadeAtribuida = atoi(token);
                    token = strtok(NULL, "\0");
                }


                *newUtente = u1;
                newUtente->next=NULL;

                if(dadosRegUtentes->listaUtentes->first==NULL) //primeiro utente
                {
                    //DEBUG
                    printf("\n -- Primeiro utente ");
                    fflush(stdout);
                    //DEBUG


                    dadosRegUtentes->listaUtentes->first = newUtente;
                    newUtente->next = NULL;
                    (*dadosRegUtentes->nUtentesLigados)++;
                }
                else//já existem utentes
                {
                    pUtente percorre = dadosRegUtentes->listaUtentes->first;

                    //DEBUG
                    printf("\n  --Ja existem utentes na lista");
                    fflush(stdout);
                    //DEBUG

                    //verificar se a especialidade está cheia
                    //TODO validar que o array esta a aser atualizado como deve ser; this shit sus
                    if(strcmp(newUtente->especialidadeAtribuida,"oftalmologia")==0){
                        if(dadosRegUtentes->nUtentesEspecialidade[0]==5){
                            strcpy(u1.nomeUtente,"ESPECIALIDADEFULL");
                            sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, u1.pid);
                            fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                            write(fdResposta, &u1, sizeof(u1));
                            close(fdResposta);

                        }else{
                            (dadosRegUtentes->nUtentesEspecialidade[0])++;
                        }
                    }
                    if(strcmp(newUtente->especialidadeAtribuida,"neurologia")==0){
                        if(dadosRegUtentes->nUtentesEspecialidade[1]==5){
                            strcpy(u1.nomeUtente,"ESPECIALIDADEFULL");
                            sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, u1.pid);
                            fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                            write(fdResposta, &u1, sizeof(u1));
                            close(fdResposta);
                        }else{
                            (dadosRegUtentes->nUtentesEspecialidade[1])++;
                        }
                    }
                    if(strcmp(newUtente->especialidadeAtribuida,"estomatologia")==0){
                        if(dadosRegUtentes->nUtentesEspecialidade[2]==5){
                            strcpy(u1.nomeUtente,"ESPECIALIDADEFULL");
                            sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, u1.pid);
                            fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                            write(fdResposta, &u1, sizeof(u1));
                            close(fdResposta);
                        }else{
                            (dadosRegUtentes->nUtentesEspecialidade[2])++;
                        }
                    }
                    if(strcmp(newUtente->especialidadeAtribuida,"ortopedia")==0){

                        if(dadosRegUtentes->nUtentesEspecialidade[3]==5){

                            strcpy(u1.nomeUtente,"ESPECIALIDADEFULL");
                            sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, u1.pid);
                            fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                            write(fdResposta, &u1, sizeof(u1));
                            close(fdResposta);
                        }else{
                            (dadosRegUtentes->nUtentesEspecialidade[3])++;
                        }
                    }
                    if(strcmp(newUtente->especialidadeAtribuida,"geral")==0){
                        if(dadosRegUtentes->nUtentesEspecialidade[4]==5){
                            strcpy(u1.nomeUtente,"ESPECIALIDADEFULL");
                            sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, u1.pid);
                            fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                            write(fdResposta, &u1, sizeof(u1));
                            close(fdResposta);
                        }else{
                            (dadosRegUtentes->nUtentesEspecialidade[4])++;
                        }
                    }

                    //já se verificou, não está cheio, contador nUtentesEspecialista aumentado

                    //DEBUG
                    printf("\n Nao esta cheio ");
                    fflush(stdout);
                    //DEBUG

                    //inserir utente na lista

                    if(percorre->next == NULL) //existe apenas 1 utente
                    {
                        if(percorre->prioridadeAtribuida > newUtente->prioridadeAtribuida){
                            newUtente->next = percorre;
                            dadosRegUtentes->listaUtentes->first = newUtente;
                        }else{
                            percorre->next = newUtente;
                            newUtente->next = NULL;
                        }
                    }

                    else // existem mais que um
                    {
                        if(percorre->prioridadeAtribuida > newUtente->prioridadeAtribuida){
                            newUtente->next = percorre;
                            dadosRegUtentes->listaUtentes->first = newUtente;
                        }else{
                            while (percorre->next!=NULL){
                                if(percorre->next->prioridadeAtribuida > newUtente->prioridadeAtribuida){
                                    newUtente->next = percorre->next;
                                    percorre->next = newUtente;
                                    break;
                                }
                                percorre = percorre->next;
                            }
                            percorre->next = newUtente;
                        }
                    }
                }

                fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                write(fdResposta, &u1, sizeof(u1));
                close(fdResposta);
            }

        }
    }
    pthread_exit(NULL);
}


int main() {


    struct Balcao balcao; //estrutura que guarda a informação necessária ao balcão
    char comando[MAX_STRING_SIZE];
    char comando1[MAX_STRING_SIZE];
    char comando2[MAX_STRING_SIZE];
    int comandoN;
    int fdServer, fd_balcao_classificador[2], fd_classificador_balcao[2];
    char *token;

    pUtenteContainer listaUtentes = NULL;


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
    fdServer = open(SERVER_FIFO, O_RDONLY | O_NONBLOCK);
    if (fdServer == -1) {
        fprintf(stderr, "\nErro ao abrir FIFO -> MEDICALso_server");
        fflush(stderr);
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
        fflush(stderr);

        unlink(SERVER_FIFO);
        exit(4);
    }

    printf("\nforking");
    fflush(stdout);

    //Run Classificador
    switch (fork()) {
        case -1:{
            //error fork
            fprintf(stderr, "\nErro - Nao foi possivel criar fork\n");
            fflush(stderr);

            //clean pipes
            unlink(SERVER_FIFO);
            exit(5);
        }
        case 0: { // child - run classificador

            fprintf(stdout,"\nVou iniciar o classificador");
            fflush(stdout);

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
                fflush(stderr);
                //clean pipes
                unlink(SERVER_FIFO);
                exit(6);
            }
            fprintf(stderr, "\nErro ao correr classificador");
            fflush(stderr);
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
            listaUtentes = malloc(sizeof(pUtenteContainer));
            dados.listaUtentes = listaUtentes;
            listaUtentes->first=NULL;
            dados.nUtentesLigados = &balcao.nClienteLigados;
            dados.nUtentesEspecialidade = balcao.nUtentesEspecialidade;

            dados.nMaxClientes = balcao.N;
            //dados.nMaxClientes = 0;

            printf("\nVou iniciar a thread");
            fflush(stdout);

            if (pthread_create(&threadRecebeUtentes, NULL, &recebeUtentes, &dados) != 0) {
                fprintf(stderr, "\nErro ao criar thread para receber utentes\nA terminar...");
                exit(1);
            }

            printf("\nThread Iniciada");
            fflush(stdout);

            //input teclado
            fprintf(stdout,"\nWrite \"Help\" for a list of commands");
            fflush(stdout);
            do {

                fgets(comando, MAX_STRING_SIZE - 1, stdin);
                strcpy(comando1,strtok(comando, " "));

                //comandos com 2 argumentos

                if(strcmp(comando1, "delut") == 0 || strcmp(comando1, "delesp") == 0){
                    strcpy(comando2,strtok(NULL, " "));
                }else if(strcmp(comando1, "freq") == 0){
                    strcpy(comando2,strtok(NULL, " "));
                    comandoN=atoi(comando2);
                }

                //TODO: Inserir o resto dos comandos
                if (strcmp(comando1, "help\n") == 0) {
                    apresentaMenu();
                    fflush(stdout);
                }else if(strcmp(comando1, "utentes\n") == 0){
                    if(balcao.nClienteLigados == 0){
                        fprintf(stdout,"\nNao existem clientes ligados de momento");
                        fflush(stdout);
                    }else{
                        pUtente percorre = listaUtentes->first;
                        while(percorre!=NULL) {
                            //TODO: inserir informacao dos clientes
                            printf("\nNome do utente: %s",percorre->nomeUtente);
                            fflush(stdout);
                            percorre=percorre->next;
                        }
                    }
                }else if(strcmp(comando1, "delut") == 0){
                    printf("Segundo comando %s", comando2);
                }else if(strcmp(comando1, "delesp")==0){
                    printf("Segundo comando %s", comando2);
                }else if(strcmp(comando1, "freq")==0){
                    printf("Segundo comando %d", comandoN);
                }
                else if (strcmp(comando1, "encerra\n") == 0) {
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
#pragma clang diagnostic pop