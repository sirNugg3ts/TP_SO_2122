#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#pragma ide diagnostic ignored "cert-err34-c"

#include <pthread.h>
#include "balcao.h"
#include "utils.h"
#include "cliente.h"
#include "medico.h"

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
            fprintf(stderr, "\nErro - A variavel de ambiente MAXMEDICOS apresenta um valor invalido\n");
            return -1;
        } else {
            balcao->M = atoi(env);
        }
    } else {
        fprintf(stderr, "\nErro - A variavel de ambiente necessaria MAXMEDICOS nao se encontra definida\n");
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

void *estabeleceContacto(void *Dados){
    struct dadosManager *dados = (struct dadosManager*)Dados;

    while(dados->stop){

        pUtente percorre = dados->listaUtentes->first;
        pEspecialista percorreEspecialista = dados->listaEspecialistas->next;

        while(percorre!=NULL){
            while (percorreEspecialista!=NULL){
                printf("\n## %s | %s\n",percorre->especialidadeAtribuida,percorreEspecialista->especialidade);
                printf("-- %d\n",strcmp(percorre->especialidadeAtribuida,percorreEspecialista->especialidade));
                fflush(stdout);
                if(strcmp(percorre->especialidadeAtribuida,percorreEspecialista->especialidade) == 0){
                    //TODO COMECAR COMUNICACAO

                    //notificar o cliente que vai comunicar com PID medico
                    MSG msgCliente;
                    char pidMedico[6];
                    sprintf(pidMedico,"%d",percorreEspecialista->pid);
                    strcpy(msgCliente.msg,pidMedico);
                    char CLIENT_FIFO_FINAL[MAX_STRING_SIZE];
                    sprintf(CLIENT_FIFO_FINAL,CLIENT_FIFO,percorre->pid);
                    int fdUtente = open(CLIENT_FIFO_FINAL,O_WRONLY);
                    if(fdUtente == -1){
                        //TODO HANDLE ERROR
                    }
                    write(fdUtente,&msgCliente, sizeof(MSG));
                    close(fdUtente);


                    //notificar medico que vai comunicar com PID cliente

                    MSG msgMedico;
                    char pidCliente[6];
                    sprintf(pidCliente,"%d",percorre->pid);
                    strcpy(msgMedico.msg,pidCliente);
                    char MEDICO[MAX_STRING_SIZE];
                    sprintf(MEDICO,MEDICO_FIFO,percorreEspecialista->pid);
                    int fdEspecialista = open(MEDICO,O_WRONLY);
                    if(fdEspecialista == -1){
                        //TODO HANDLE ERROR
                    }
                    write(fdEspecialista,&msgMedico, sizeof(msgMedico));
                    close(fdEspecialista);
/*
                    //tirar cliente da fila de espera
                    if (strcmp(percorre->especialidadeAtribuida, "oftalmologia") == 0)
                        (dados->nUtentesEspecialidade[0])--;
                    if (strcmp(percorre->especialidadeAtribuida, "neurologia") == 0)
                        (dados->nUtentesEspecialidade[1])--;
                    if (strcmp(percorre->especialidadeAtribuida, "estomatologia") == 0)
                        (dados->nUtentesEspecialidade[2])--;
                    if (strcmp(percorre->especialidadeAtribuida, "ortopedia") == 0)
                        (dados->nUtentesEspecialidade[3])--;
                    if (strcmp(percorre->especialidadeAtribuida, "geral") == 0)
                        (dados->nUtentesEspecialidade[4])--;
*/

                    pUtente percorreRemover = dados->listaUtentes->first;
                    if(percorreRemover==percorre){
                        dados->listaUtentes->first = percorre->next;
                    }else{
                        while(percorreRemover->next!=percorre)
                            percorreRemover = percorreRemover->next;
                        percorreRemover->next = percorre->next;
                        free(percorre);
                    }
                    break;
                }
                percorreEspecialista = percorreEspecialista->next;
            }
            percorre = percorre->next;
        }
        sleep(2);
    }
    pthread_exit(NULL);
}

void *recebeUtentes(void *Dados) {
    DADOS_REG_UTENTES *dadosRegUtentes;
    dadosRegUtentes = (DADOS_REG_UTENTES *) Dados;

    pthread_mutex_lock(dadosRegUtentes->mutexPrints);
    fprintf(stdout, "\nWaiting for new clients...");
    fflush(stdout);
    pthread_mutex_unlock(dadosRegUtentes->mutexPrints);

    while (dadosRegUtentes->stopReceiving) {
        //read pipe
        Utente u1;
        int size = read(dadosRegUtentes->fdServer, &u1, sizeof(u1));

        if (size > 0) {

            char CLIENT_FIFO_FINAL[MAX_STRING_SIZE],bufferClassificador[MAX_STRING_SIZE];
            char *token;
            int fdResposta;


            if ((*dadosRegUtentes->nUtentesLigados) == (dadosRegUtentes->nMaxClientes))//server is full
            {
                strcpy(u1.nomeUtente, "SERVERFULL");
                sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, u1.pid);
                fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                write(fdResposta, &u1, sizeof(u1));
                close(fdResposta);
            } else //server is not full
            {
                pUtente newUtente = malloc(sizeof(Utente));

                //transmitir ao classificador os sintomas
                write(dadosRegUtentes->fd_balcao_classificador[1], u1.sintomas, strlen(u1.sintomas));

                //ler resposta do classificador
                int size1 = read(dadosRegUtentes->fd_classificador_balcao[0], bufferClassificador,
                                 sizeof(bufferClassificador));


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
                newUtente->next = NULL;

                if (dadosRegUtentes->listaUtentes->first == NULL) //primeiro utente
                {
                    dadosRegUtentes->listaUtentes->first = newUtente;
                    (*dadosRegUtentes->nUtentesLigados)++;

                    if (strcmp(newUtente->especialidadeAtribuida, "oftalmologia") == 0)
                        (dadosRegUtentes->nUtentesEspecialidade[0])++;
                    if (strcmp(newUtente->especialidadeAtribuida, "neurologia") == 0)
                        (dadosRegUtentes->nUtentesEspecialidade[1])++;
                    if (strcmp(newUtente->especialidadeAtribuida, "estomatologia") == 0)
                        (dadosRegUtentes->nUtentesEspecialidade[2])++;
                    if (strcmp(newUtente->especialidadeAtribuida, "ortopedia") == 0)
                        (dadosRegUtentes->nUtentesEspecialidade[3])++;
                    if (strcmp(newUtente->especialidadeAtribuida, "geral") == 0)
                        (dadosRegUtentes->nUtentesEspecialidade[4])++;

                    pthread_mutex_lock(dadosRegUtentes->mutexPrints);
                    fprintf(stdout,"Novo Utente:\nNome: %s\nSintomas: %s\nEspecialidade: %s\n",newUtente->nomeUtente,
                            newUtente->sintomas,newUtente->especialidadeAtribuida);
                    fflush(stdout);
                    pthread_mutex_unlock(dadosRegUtentes->mutexPrints);
                } else//já exitem utentes
                {
                    pUtente percorre = dadosRegUtentes->listaUtentes->first;

                    //verificar se a especialidade está cheia
                    if (strcmp(newUtente->especialidadeAtribuida, "oftalmologia") == 0) {
                        if (dadosRegUtentes->nUtentesEspecialidade[0] == 5) {
                            strcpy(u1.nomeUtente, "ESPECIALIDADEFULL");
                            sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, u1.pid);
                            fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                            write(fdResposta, &u1, sizeof(u1));
                            close(fdResposta);
                        } else
                            (dadosRegUtentes->nUtentesEspecialidade[0])++;

                    }

                    if (strcmp(newUtente->especialidadeAtribuida, "neurologia") == 0) {
                        if (dadosRegUtentes->nUtentesEspecialidade[1] == 5) {
                            strcpy(u1.nomeUtente, "ESPECIALIDADEFULL");
                            sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, u1.pid);
                            fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                            write(fdResposta, &u1, sizeof(u1));
                            close(fdResposta);
                        } else
                            (dadosRegUtentes->nUtentesEspecialidade[1])++;

                    }

                    if (strcmp(newUtente->especialidadeAtribuida, "estomatologia") == 0) {
                        if (dadosRegUtentes->nUtentesEspecialidade[2] == 5) {
                            strcpy(u1.nomeUtente, "ESPECIALIDADEFULL");
                            sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, u1.pid);
                            fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                            write(fdResposta, &u1, sizeof(u1));
                            close(fdResposta);
                        } else
                            (dadosRegUtentes->nUtentesEspecialidade[2])++;

                    }

                    if (strcmp(newUtente->especialidadeAtribuida, "ortopedia") == 0) {
                        if (dadosRegUtentes->nUtentesEspecialidade[3] == 5) {
                            strcpy(u1.nomeUtente, "ESPECIALIDADEFULL");
                            sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, u1.pid);
                            fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                            write(fdResposta, &u1, sizeof(u1));
                            close(fdResposta);
                        } else
                            (dadosRegUtentes->nUtentesEspecialidade[3])++;
                    }

                    if (strcmp(newUtente->especialidadeAtribuida, "geral") == 0) {
                        if (dadosRegUtentes->nUtentesEspecialidade[4] == 5) {
                            strcpy(u1.nomeUtente, "ESPECIALIDADEFULL");
                            sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, u1.pid);
                            fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                            write(fdResposta, &u1, sizeof(u1));
                            close(fdResposta);
                        } else
                            (dadosRegUtentes->nUtentesEspecialidade[4])++;

                    }

                    //já se verificou, não está cheio, contador nUtentesEspecialista aumentado

                    //inserir utente na lista
                    (*dadosRegUtentes->nUtentesLigados)++;

                   pthread_mutex_lock(dadosRegUtentes->mutexPrints);
                   fprintf(stdout,"Novo Utente:\nNome: %s\nSintomas: %s\nEspecialidade: %s\n",newUtente->nomeUtente,
                           newUtente->sintomas,newUtente->especialidadeAtribuida);
                   fflush(stdout);
                   pthread_mutex_unlock(dadosRegUtentes->mutexPrints);

                    if (percorre->next == NULL) //existe apenas 1 utente
                    {
                        if (percorre->prioridadeAtribuida > newUtente->prioridadeAtribuida) {
                            newUtente->next = percorre;
                            dadosRegUtentes->listaUtentes->first = newUtente;
                        } else {
                            percorre->next = newUtente;
                            newUtente->next = NULL;
                        }
                    } else // existem mais que um
                    {
                        if (percorre->prioridadeAtribuida > newUtente->prioridadeAtribuida) {
                            newUtente->next = percorre;
                            dadosRegUtentes->listaUtentes->first = newUtente;
                        } else {
                            while (percorre->next != NULL) {
                                if (percorre->next->prioridadeAtribuida > newUtente->prioridadeAtribuida) {
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

void *recebeMedicos(void *Dados){

    DADOS_REG_MEDICOS *dados = (DADOS_REG_MEDICOS *)Dados;

    pthread_mutex_lock(dados->mutexPrints);
    fprintf(stdout,"\nWaiting for new especialistas...");
    fflush(stdout);
    pthread_mutex_unlock(dados->mutexPrints);

    while(dados->stopReceiving){
        Especialista e;
        int size = read(dados->fdServer,&e,sizeof(e));

        if(size > 0){
            char MEDICO_FIFO_FINAL[MAX_STRING_SIZE];
            int fdResposta;

            pthread_mutex_lock(dados->mutexPrints);
            printf("\nNovo medico");
            fflush(stdout);
            pthread_mutex_unlock(dados->mutexPrints);

            if((*dados->nMedicosLigados) == (dados->nMaxMedicos))//server is full
            {
                strcpy(e.nomeMedico,"SERVERFULL");
                sprintf(MEDICO_FIFO_FINAL,MEDICO_FIFO,e.pid);
                fdResposta = open(MEDICO_FIFO_FINAL,O_WRONLY);
                write(fdResposta,&e,sizeof(e));
                close(fdResposta);
            }
            else //server not full
            {
                pEspecialista especialita = malloc(sizeof(Especialista));
                *especialita = e;
                especialita->next = NULL;
                if(dados->lista->next == NULL)
                    dados->lista->next= especialita;
                else{
                    pEspecialista percorre = dados->lista->next;
                    while(percorre->next!=NULL)
                        percorre=percorre->next;
                    percorre->next = especialita;
                }

                (*dados->nMedicosLigados)++;
                sprintf(MEDICO_FIFO_FINAL,MEDICO_FIFO,e.pid);
                fdResposta = open(MEDICO_FIFO_FINAL,O_WRONLY);
                write(fdResposta,&e,sizeof(e));
                close(fdResposta);
            }
        }
    }
    pthread_exit(NULL);

}

void createFifos(){
    //iniciar pipe balcao
    if (mkfifo(SERVER_FIFO, 0777) == -1) {
        if (errno == EEXIST) {
            fprintf(stderr, "\nO pipe ja existe -> MEDICALso_server ");
            exit(1);
        }
        fprintf(stderr, "\nErro ao criar pipe -> MEDICALso_server");
        exit(1);
    }

    if (mkfifo(SERVER_FIFO_FOR_MEDICS, 0777) == -1) {
        if (errno == EEXIST) {
            fprintf(stderr, "\nO pipe ja existe -> MEDICALso_server ");
            exit(1);
        }
        fprintf(stderr, "\nErro ao criar pipe -> MEDICALso_server");
        exit(1);
    }
}

void openPipes(int *fdServer, int *fdServerMedics, int fd_balcao_classificador[],int fd_classificador_balcao[]){
    //FD servidor
    *fdServer = open(SERVER_FIFO, O_RDONLY | O_NONBLOCK);
    if (*fdServer == -1) {
        fprintf(stderr, "\nErro ao abrir FIFO -> MEDICALso_server");
        fflush(stderr);
        unlink(SERVER_FIFO);
        exit(2);
    }

    *fdServerMedics = open(SERVER_FIFO_FOR_MEDICS, O_RDONLY | O_NONBLOCK);
    if (*fdServerMedics == -1) {
        fprintf(stderr, "\nErro ao abrir FIFO -> MEDICALso_server");
        fflush(stderr);
        unlink(SERVER_FIFO);
        exit(2);
    }

    //File Descriptors para comunicação com o Classificador

    if (pipe(fd_balcao_classificador) == -1 || pipe(fd_classificador_balcao) == -1) {
        fprintf(stderr, "\nErro - Nao foi possivel criar pipes para o classificador\n");
        fflush(stderr);

        unlink(SERVER_FIFO);
        exit(4);
    }
}

int main() {

    struct Balcao balcao; //estrutura que guarda a informação necessária ao balcão
    char comando[MAX_STRING_SIZE],comando1[MAX_STRING_SIZE],comando2[MAX_STRING_SIZE];
    int fdServer,fdServerMedics, fd_balcao_classificador[2], fd_classificador_balcao[2],comandoN;
    pUtenteContainer listaUtentes = NULL;
    pEspecialista listaEspecialistas = NULL;

    createFifos();
    openPipes(&fdServer,&fdServerMedics,fd_balcao_classificador,fd_classificador_balcao);

    //inicialização
    if (obtemVariaveisAmbiente(&balcao) == -1) {
        unlink(SERVER_FIFO);
        exit(3);
    }
    inicializaStruct(&balcao);

    //Run Classificador
    switch (fork()) {
        case -1: {
            //error fork
            fprintf(stderr, "\nErro - Nao foi possivel criar fork\n");
            fflush(stderr);

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
            pthread_t threadRecebeMedicos;

            pthread_mutex_t mutexPrints;
            pthread_mutex_init(&mutexPrints, NULL);

            listaUtentes = malloc(sizeof(pUtenteContainer));
            listaUtentes->first = NULL;

            listaEspecialistas = malloc(sizeof(pEspecialista));
            listaEspecialistas->next = NULL;




            DADOS_REG_UTENTES dados;
            dados.fdServer = fdServer;
            dados.fd_classificador_balcao = fd_classificador_balcao;
            dados.fd_balcao_classificador = fd_balcao_classificador;
            dados.stopReceiving = 1;
            dados.listaUtentes = listaUtentes;
            dados.nUtentesLigados = &balcao.nClienteLigados;
            dados.nUtentesEspecialidade = balcao.nUtentesEspecialidade;
            dados.mutexPrints = &mutexPrints;
            dados.nMaxClientes = balcao.N;

            if (pthread_create(&threadRecebeUtentes, NULL, &recebeUtentes, &dados) != 0) {
                fprintf(stderr, "\nErro ao criar thread para receber utentes\nA terminar...");
                exit(1);
            }

            DADOS_REG_MEDICOS dadosMedico;
            dadosMedico.lista = listaEspecialistas;
            dadosMedico.mutexPrints = &mutexPrints;
            dadosMedico.nMaxMedicos = balcao.M;
            dadosMedico.nMedicosLigados = &balcao.nMedicosLigados;
            dadosMedico.fdServer = fdServerMedics;
            dadosMedico.stopReceiving = 1;

            if (pthread_create(&threadRecebeMedicos, NULL, &recebeMedicos, &dadosMedico) != 0) {
                fprintf(stderr, "\nErro ao criar thread para receber utentes\nA terminar...");
                exit(1);
            }

            //thread manage connections

            pthread_t threadGestor;


            struct dadosManager dadosManager;
            dadosManager.listaUtentes = listaUtentes;
            dadosManager.listaEspecialistas = listaEspecialistas;
            dadosManager.stop = 1;

            if (pthread_create(&threadGestor, NULL, &estabeleceContacto, &dadosManager) != 0) {
                fprintf(stderr, "\nErro ao criar thread para receber utentes\nA terminar...");
                exit(1);
            }

            pthread_mutex_lock(&mutexPrints);
            //input teclado
            fprintf(stdout, "\nWrite \"Help\" for a list of commands");
            fflush(stdout);
            pthread_mutex_unlock(&mutexPrints);
            do {

                fgets(comando, MAX_STRING_SIZE - 1, stdin);
                strcpy(comando1, strtok(comando, " "));

                //comandos com 2 argumentos

                if (strcmp(comando1, "delut") == 0 || strcmp(comando1, "delesp") == 0) {
                    strcpy(comando2, strtok(NULL, " "));
                } else if (strcmp(comando1, "freq") == 0) {
                    strcpy(comando2, strtok(NULL, " "));
                    comandoN = atoi(comando2);
                }


                //DEBUG
                if(strcmp(comando1,"medicos\n") == 0) {
                    if (balcao.nMedicosLigados == 0) {
                        pthread_mutex_lock(&mutexPrints);
                        fprintf(stdout, "\nNao existem medicos ligados de momento");
                        fflush(stdout);
                        pthread_mutex_unlock(&mutexPrints);
                    } else {

                        pEspecialista percorre = listaEspecialistas->next;
                        while (percorre != NULL) {
                            pthread_mutex_lock(&mutexPrints);
                            printf("\nNome do Medico: %s\nEspecialidade: %s\n", percorre->nomeMedico,
                                   percorre->especialidade);
                            fflush(stdout);
                            pthread_mutex_unlock(&mutexPrints);
                            percorre = percorre->next;
                        }
                    }
                }
                //DEBUG
                if (strcmp(comando1, "help\n") == 0) {
                    pthread_mutex_lock(&mutexPrints);
                    apresentaMenu();
                    fflush(stdout);
                    pthread_mutex_unlock(&mutexPrints);
                } else if (strcmp(comando1, "utentes\n") == 0) {//DEBUG
                    //DEBUG
                    if (balcao.nClienteLigados == 0) {
                        pthread_mutex_lock(&mutexPrints);
                        fprintf(stdout, "\nNao existem clientes ligados de momento");
                        fflush(stdout);
                        pthread_mutex_unlock(&mutexPrints);
                    } else {
                        pUtente percorre = listaUtentes->first;
                        while (percorre != NULL) {
                            //TODO: inserir informacao dos clientes
                            pthread_mutex_lock(&mutexPrints);
                            printf("\nNome do utente: %s", percorre->nomeUtente);
                            fflush(stdout);
                            pthread_mutex_unlock(&mutexPrints);
                            percorre = percorre->next;
                        }
                    }
                } else if (strcmp(comando1, "delut") == 0) {
                    printf("Segundo comando %s", comando2);
                } else if (strcmp(comando1, "delesp") == 0) {
                    printf("Segundo comando %s", comando2);
                } else if (strcmp(comando1, "freq") == 0) {
                    printf("Segundo comando %d", comandoN);
                } else if (strcmp(comando1, "encerra\n") == 0) {
                    dados.stopReceiving = 0; // parar de aceitar utentes
                  //  dadosManager.stop = 0; //parar de estabelecer conexões
                    pthread_cancel(threadGestor);
                    //TODO: Notificar que o balcao vai encerrar
                    //TODO: exit gracefully
                    break;
                }
            } while (1);
            pthread_join(threadRecebeUtentes, NULL);
            break;
        }
    }
}

#pragma clang diagnostic pop