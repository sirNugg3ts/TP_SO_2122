#include <pthread.h>
#include "balcao.h"
#include "utils.h"
#include "cliente.h"
#include "medico.h"

int terminarPrograma = 1;

void murder(int s){char commando[] = "ENCERRA BALCAO";
   int fd = open(BALCAO_COMMANDS,O_RDWR | O_NONBLOCK);
   if(fd == -1){
       //TODO HANDLE ERROR
   }

   MSG msg;
    strcpy(msg.msg,commando);

   int size = write(fd,&msg, sizeof(MSG));
    if (size < 0) {
        printf("\nErro ao escrever a estrutura para o pipe \n");
        exit(1);
    }
    close(fd);

}

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





void *recebeUtentes(void *Dados) {

    DADOS_REG_UTENTES *dadosRegUtentes;
    dadosRegUtentes = (DADOS_REG_UTENTES *) Dados;

    pthread_mutex_lock(dadosRegUtentes->mutexPrints);
    fprintf(stdout, "\nWaiting for new clients...");
    fflush(stdout);
    pthread_mutex_unlock(dadosRegUtentes->mutexPrints);

    while (dadosRegUtentes->stopReceiving) {
        dadosRegUtentes->fdServer = open(SERVER_FIFO, O_RDONLY);
        if (dadosRegUtentes->fdServer == -1) {
            fprintf(stderr, "\nErro ao abrir FIFO -> MEDICALso_server");
            fflush(stderr);
            unlink(SERVER_FIFO);
            exit(2);
        }
        Utente u1;
        int size = read(dadosRegUtentes->fdServer, &u1, sizeof(u1));

        if (size > 0) {

            char CLIENT_FIFO_FINAL[MAX_STRING_SIZE], bufferClassificador[MAX_STRING_SIZE];
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
                int nUtentesAfrente = 0;
                newUtente->next = NULL;

                pthread_mutex_lock(dadosRegUtentes->mutexListaUtentes);

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

                    nUtentesAfrente = 0;

                    pthread_mutex_lock(dadosRegUtentes->mutexPrints);
                    fprintf(stdout, "Novo Utente:\nNome: %s\nSintomas: %s\nEspecialidade: %s\n", newUtente->nomeUtente,
                            newUtente->sintomas, newUtente->especialidadeAtribuida);
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
                        } else{
                            (dadosRegUtentes->nUtentesEspecialidade[0])++;
                            nUtentesAfrente = (dadosRegUtentes->nUtentesEspecialidade[0])-1;
                        }


                    }

                    if (strcmp(newUtente->especialidadeAtribuida, "neurologia") == 0) {
                        if (dadosRegUtentes->nUtentesEspecialidade[1] == 5) {
                            strcpy(u1.nomeUtente, "ESPECIALIDADEFULL");
                            sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, u1.pid);
                            fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                            write(fdResposta, &u1, sizeof(u1));
                            close(fdResposta);
                        } else{
                            (dadosRegUtentes->nUtentesEspecialidade[1])++;
                            nUtentesAfrente = (dadosRegUtentes->nUtentesEspecialidade[1])-1;
                        }

                    }

                    if (strcmp(newUtente->especialidadeAtribuida, "estomatologia") == 0) {
                        if (dadosRegUtentes->nUtentesEspecialidade[2] == 5) {
                            strcpy(u1.nomeUtente, "ESPECIALIDADEFULL");
                            sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, u1.pid);
                            fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                            write(fdResposta, &u1, sizeof(u1));
                            close(fdResposta);
                        } else{
                            (dadosRegUtentes->nUtentesEspecialidade[2])++;
                            nUtentesAfrente = (dadosRegUtentes->nUtentesEspecialidade[2])-1;
                        }

                    }

                    if (strcmp(newUtente->especialidadeAtribuida, "ortopedia") == 0) {
                        if (dadosRegUtentes->nUtentesEspecialidade[3] == 5) {
                            strcpy(u1.nomeUtente, "ESPECIALIDADEFULL");
                            sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, u1.pid);
                            fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                            write(fdResposta, &u1, sizeof(u1));
                            close(fdResposta);
                        } else{
                            (dadosRegUtentes->nUtentesEspecialidade[3])++;
                            nUtentesAfrente = (dadosRegUtentes->nUtentesEspecialidade[3])-1;
                        }
                    }

                    if (strcmp(newUtente->especialidadeAtribuida, "geral") == 0) {
                        if (dadosRegUtentes->nUtentesEspecialidade[4] == 5) {
                            strcpy(u1.nomeUtente, "ESPECIALIDADEFULL");
                            sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, u1.pid);
                            fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                            write(fdResposta, &u1, sizeof(u1));
                            close(fdResposta);
                        } else{
                            (dadosRegUtentes->nUtentesEspecialidade[4])++;
                            nUtentesAfrente = (dadosRegUtentes->nUtentesEspecialidade[4])-1;
                        }

                    }

                    //já se verificou, não está cheio, contador nUtentesEspecialista aumentado

                    //inserir utente na lista
                    (*dadosRegUtentes->nUtentesLigados)++;

                    pthread_mutex_lock(dadosRegUtentes->mutexPrints);
                    fprintf(stdout, "Novo Utente:\nNome: %s\nSintomas: %sEspecialidade: %s\n\n", newUtente->nomeUtente,
                            newUtente->sintomas, newUtente->especialidadeAtribuida);
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

                pthread_mutex_unlock(dadosRegUtentes->mutexListaUtentes);

                fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                write(fdResposta, &u1, sizeof(u1));
                close(fdResposta);

                int nEspecialistas = 0;

                pthread_mutex_lock(dadosRegUtentes->mutexListaMedicos);
                pEspecialista percorreEspecialistas = dadosRegUtentes->listaEspecialistas->next;
                while (percorreEspecialistas!=NULL){
                    if(strcmp(percorreEspecialistas->especialidade,u1.especialidadeAtribuida)==0)
                        nEspecialistas++;
                    percorreEspecialistas = percorreEspecialistas->next;
                }
                pthread_mutex_unlock(dadosRegUtentes->mutexListaMedicos);
                fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                MSG msg;
                char mensagem[MAX_STRING_SIZE];
                char finalMessage[MAX_STRING_SIZE];

                strcpy(mensagem,"Especialidade: %s\nPrioridade: %d\nUtentes em fila:%d"
                                "\nEspecialistas: %d");
                sprintf(finalMessage, mensagem, newUtente->especialidadeAtribuida, newUtente->prioridadeAtribuida,
                        nUtentesAfrente, nEspecialistas);
                strcpy(msg.msg,finalMessage);
                write(fdResposta,&msg,sizeof(MSG));
                close(fdResposta);
            }
        }
    }
    pthread_exit(NULL);
}

void *recebeMedicos(void *Dados) {

    DADOS_REG_MEDICOS *dados = (DADOS_REG_MEDICOS *) Dados;

    pthread_mutex_lock(dados->mutexPrints);
    fprintf(stdout, "\nWaiting for new especialistas...");
    fflush(stdout);
    pthread_mutex_unlock(dados->mutexPrints);

    while (dados->stopReceiving) {
        Especialista e;
        dados->fdServer = open(SERVER_FIFO_FOR_MEDICS, O_RDONLY);
        if (dados->fdServer == -1) {
            fprintf(stderr, "\nErro ao abrir FIFO -> MEDICALso_server");
            fflush(stderr);
            unlink(SERVER_FIFO);
            exit(2);
        }

        int size = read(dados->fdServer, &e, sizeof(e));
        if (size > 0) {
            char MEDICO_FIFO_FINAL[MAX_STRING_SIZE];
            int fdResposta;

            if ((*dados->nMedicosLigados) == (dados->nMaxMedicos))//server is full
            {
                strcpy(e.nomeMedico, "SERVERFULL");
                sprintf(MEDICO_FIFO_FINAL, MEDICO_FIFO, e.pid);
                fdResposta = open(MEDICO_FIFO_FINAL, O_WRONLY);
                write(fdResposta, &e, sizeof(e));
                close(fdResposta);
            } else //server not full
            {
                pthread_mutex_lock(dados->mutexPrints);
                printf("\nNovo medico:\nNome: %s\nEspecialidade: %s", e.nomeMedico, e.especialidade);
                fflush(stdout);
                pthread_mutex_unlock(dados->mutexPrints);
                pthread_mutex_lock(dados->mutexListaMedicos);
                pEspecialista especialita = malloc(sizeof(Especialista));
                *especialita = e;
                especialita->ocupado = 0;
                especialita->next = NULL;
                if (dados->lista->next == NULL)
                    dados->lista->next = especialita;
                else {
                    pEspecialista percorre = dados->lista->next;
                    while (percorre->next != NULL)
                        percorre = percorre->next;
                    percorre->next = especialita;
                }

                (*dados->nMedicosLigados)++;
                especialita->pidServer = getpid();
                pthread_mutex_unlock(dados->mutexListaMedicos);

                sprintf(MEDICO_FIFO_FINAL, MEDICO_FIFO, e.pid);
                fdResposta = open(MEDICO_FIFO_FINAL, O_WRONLY);
                write(fdResposta, &e, sizeof(e));
                close(fdResposta);
            }
        }
    }
    pthread_exit(NULL);
}

struct dadosStatus{
    int *ocupacao;
    int *timeFreq;
    int stopShowing;
};

void *apresentaStatusEspera(void *Dados){
    struct dadosStatus *dadosStatus = (struct dadosStatus*)Dados;
    sleep(2);

    printf("-> Apresenta status");
    fflush(stdout);


    while(dadosStatus->stopShowing){
        printf("\n###\nStatus:\n");
        fflush(stdout);
        printf("Oftalmologia: %d\n",dadosStatus->ocupacao[0]);
        printf("Neurologia: %d\n",dadosStatus->ocupacao[1]);
        printf("Estomatologia: %d\n",dadosStatus->ocupacao[2]);
        printf("Ortopedia: %d\n",dadosStatus->ocupacao[3]);
        printf("Geral: %d\n",dadosStatus->ocupacao[4]);
        printf("###\n");
        fflush(stdout);
        sleep(*dadosStatus->timeFreq);
    }
    pthread_exit(NULL);
}
void *estabeleceContacto(void *Dados) {
    struct dadosManager *dados = (struct dadosManager *) Dados;



    while (dados->stop) {

        pUtente percorre = dados->listaUtentes->first;

        pEspecialista percorreEspecialista = dados->listaEspecialistas->next;

        pthread_mutex_lock(dados->mutexListaMedicos);
        pthread_mutex_lock(dados->mutexListaUtentes);

        while (percorre != NULL) {
            if(percorre->atendido==0){
                while (percorreEspecialista != NULL) {
                    if (strcmp(percorre->especialidadeAtribuida, percorreEspecialista->especialidade) == 0
                        && percorreEspecialista->ocupado == 0) {

                        //notificar o cliente que vai comunicar com PID medico
                        MSG msgCliente;
                        char pidMedico[6];
                        sprintf(pidMedico, "%d", percorreEspecialista->pid);
                        strcpy(msgCliente.msg, pidMedico);
                        char CLIENT_FIFO_FINAL[MAX_STRING_SIZE];
                        sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, percorre->pid);
                        int fdUtente = open(CLIENT_FIFO_FINAL, O_WRONLY);
                        if (fdUtente == -1) {
                            //TODO HANDLE ERROR
                        }
                        write(fdUtente, &msgCliente, sizeof(MSG));
                        close(fdUtente);


                        //notificar medico que vai comunicar com PID cliente

                        MSG msgMedico;
                        char pidCliente[6];
                        sprintf(pidCliente, "%d", percorre->pid);
                        strcpy(msgMedico.msg, pidCliente);
                        char MEDICO[MAX_STRING_SIZE];
                        sprintf(MEDICO, MEDICO_FIFO, percorreEspecialista->pid);
                        int fdEspecialista = open(MEDICO, O_WRONLY);
                        if (fdEspecialista == -1) {
                            //TODO HANDLE ERROR
                        }
                        write(fdEspecialista, &msgMedico, sizeof(msgMedico));
                        close(fdEspecialista);

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

                        percorre->atendido = 1;
                        percorreEspecialista->ocupado=1;
                        strcpy(percorre->nomeMedico,percorreEspecialista->nomeMedico);

                        printf("\nO utente %s sera atendido pelo medico %s",percorre->nomeUtente,percorreEspecialista->nomeMedico);
                        fflush(stdout);
                    }
                    percorreEspecialista = percorreEspecialista->next;
                }
            }

            percorre = percorre->next;
        }
        pthread_mutex_unlock(dados->mutexListaMedicos);
        pthread_mutex_unlock(dados->mutexListaUtentes);
        sleep(5);
    }
    pthread_exit(NULL);
}


void createFifos() {
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

    if(mkfifo(BALCAO_COMMANDS,0777) == -1){
        if (errno == EEXIST) {
            fprintf(stderr, "\nO pipe ja existe -> MEDICALso_server ");
            exit(1);
        }
        fprintf(stderr, "\nErro ao criar pipe -> MEDICALso_server");
        exit(1);
    }

}

void *handleHeartBeats(void *Dados){
    struct dadosHeartbeat *dadosHeartbeat = (struct dadosHeartbeat*) Dados;

    while (dadosHeartbeat->stop){
        int pidReader;
        int size = read(dadosHeartbeat->fdHeartbeat,&pidReader,sizeof(int));
        if(size>0){
            int flag = 0;
            pEspecialista percorre = dadosHeartbeat->listaEspecialistas->next;
            while (percorre!=NULL){
                if(percorre->pid == pidReader){
                    (percorre->missedHeartbeats)--;
                    flag = 1;
                    break;
                }
                percorre = percorre->next;
            }
            if(flag == 0){
                printf("\nWarning: recebido heartbeat de processo desconhecido");
                fflush(stdout);
            }
        }
    }
}



int main() {

    struct Balcao balcao; //estrutura que guarda a informação necessária ao balcão
    char comando[MAX_STRING_SIZE], comando1[MAX_STRING_SIZE], comando2[MAX_STRING_SIZE];
    int fdServer, fdServerMedics, fd_balcao_classificador[2], fd_classificador_balcao[2];
    pUtenteContainer listaUtentes = NULL;
    pEspecialista listaEspecialistas = NULL;

    signal(SIGINT, murder);

    createFifos();

    if (pipe(fd_balcao_classificador) == -1 || pipe(fd_classificador_balcao) == -1) {
        fprintf(stderr, "\nErro - Nao foi possivel criar pipes para o classificador\n");
        fflush(stderr);
        unlink(SERVER_FIFO);
        exit(4);
    } else {
        //DEBUG
        printf("\n##pipes created\n");
        fflush(stdout);
        //DEBUG
    }

    //inicialização
    if (obtemVariaveisAmbiente(&balcao) == -1) {
        unlink(SERVER_FIFO);
        exit(3);
    }
    inicializaStruct(&balcao);

    //DEBUG
    printf("\n##all ready\n");
    fflush(stdout);
    //DEBUG

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
            //DEBUG
            printf("\n##running classificador\n");
            fflush(stdout);
            //DEBUG
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


            pthread_mutex_t mutexListaUtentes;
            pthread_mutex_t mutexListaMedicos;

            pthread_mutex_init(&mutexListaUtentes, NULL);
            pthread_mutex_init(&mutexListaMedicos, NULL);



            int threadUtentesready = 0,threadMedicosReady = 0;


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
            dados.listaEspecialistas = listaEspecialistas;
            dados.mutexListaUtentes = &mutexListaUtentes;
            dados.mutexListaMedicos = &mutexListaMedicos;




            if (pthread_create(&threadRecebeUtentes, NULL, recebeUtentes, &dados) != 0) {
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
            dadosMedico.mutexListaMedicos = &mutexListaMedicos;


            if (pthread_create(&threadRecebeMedicos, NULL, recebeMedicos, &dadosMedico) != 0) {
                fprintf(stderr, "\nErro ao criar thread para receber utentes\nA terminar...");
                exit(1);
            }

            //thread manage connections

            pthread_t threadGestor;

            struct dadosManager dadosManager;
            dadosManager.listaUtentes = listaUtentes;
            dadosManager.listaEspecialistas = listaEspecialistas;
            dadosManager.nUtentesEspecialidade = balcao.nUtentesEspecialidade;
            dadosManager.mutexListaUtentes = &mutexListaUtentes;
            dadosManager.mutexListaMedicos = &mutexListaMedicos;
            dadosManager.stop = 1;


            if (pthread_create(&threadGestor, NULL, &estabeleceContacto, &dadosManager) != 0) {
                fprintf(stderr, "\nErro ao criar thread para receber utentes\nA terminar...");
                exit(1);
            }

            pthread_t status;
            struct dadosStatus dadosStatus;
            dadosStatus.ocupacao = balcao.nUtentesEspecialidade;
            dadosStatus.stopShowing = 1;
            int timeFreq = 30;
            dadosStatus.timeFreq = &timeFreq;

            if (pthread_create(&status, NULL, &apresentaStatusEspera, &dadosStatus) != 0) {
                fprintf(stderr, "\nErro ao criar thread para receber utentes\nA terminar...");
                exit(1);
            }

            pthread_mutex_lock(&mutexPrints);
            //input teclado
            fprintf(stdout, "\nWrite \"Help\" for a list of commands");
            fflush(stdout);
            pthread_mutex_unlock(&mutexPrints);

            int fdComandos = open(BALCAO_COMMANDS, O_RDWR | O_NONBLOCK);
            fd_set read_fds;
            int stopLoop = 1;

            while(stopLoop) {
                FD_ZERO(&read_fds);
                FD_SET(0, &read_fds);
                FD_SET(fdComandos, &read_fds);

                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 0;

                select(fdComandos + 1, &read_fds, NULL, NULL, &tv);
                if (FD_ISSET(fdComandos, &read_fds)) {
                    printf("\n DING ING\n");
                    fflush(stdout);
                    MSG m;
                    int s = read(fdComandos, &m, sizeof(MSG));

                    printf("%s",m.msg);
                    fflush(stdout);

                    if(strcmp(m.msg,"ENCERRA BALCAO")==0){
                        //TODO HANDLE EXIT
                        dados.stopReceiving = 0; // parar de aceitar utentes
                       dadosManager.stop = 0; //parar de estabelecer conexões
                      pthread_cancel(threadGestor);
                        pthread_cancel(threadRecebeMedicos);
                        pthread_cancel(threadRecebeUtentes);
                       pthread_cancel(status);
                        unlink(SERVER_FIFO);
                        unlink(SERVER_FIFO_FOR_MEDICS);
                        unlink(BALCAO_COMMANDS);
                        stopLoop = 0;
                        printf("\nA terminar, goodbye\n");
                        return EXIT_SUCCESS;
                    }

                    //TODO HANDLE COMMANDS

                }
                else if (FD_ISSET(0, &read_fds)) {
                    fgets(comando, MAX_STRING_SIZE - 1, stdin);
                    strcpy(comando1, strtok(comando, " "));

                    //comandos com 2 argumentos
                    if (strcmp(comando1, "delut") == 0 ||
                        strcmp(comando1, "delesp") == 0 ||
                        strcmp(comando1, "freq") == 0)
                        strcpy(comando2, strtok(NULL, " "));

                    if (strcmp(comando1, "help\n") == 0) {
                        pthread_mutex_lock(&mutexPrints);
                        apresentaMenu();
                        fflush(stdout);
                        pthread_mutex_unlock(&mutexPrints);
                    }

                    if (strcmp(comando1, "utentes\n") == 0) {
                        if (balcao.nClienteLigados == 0) {
                            pthread_mutex_lock(&mutexPrints);
                            fprintf(stdout, "\nNao existem clientes ligados de momento");
                            fflush(stdout);
                            pthread_mutex_unlock(&mutexPrints);
                        } else {
                            pUtente percorre = listaUtentes->first;
                            printf("\n######");
                            fflush(stdout);
                            while (percorre != NULL) {
                                pthread_mutex_lock(&mutexPrints);
                                printf("\nNome do utente: %s\n"
                                       "Especialidade: %s\n"
                                       "Prioridade: %d\n", percorre->nomeUtente, percorre->especialidadeAtribuida,
                                       percorre->prioridadeAtribuida);
                                if (percorre->atendido) {
                                    printf("Atendido por: %s\n", percorre->nomeMedico);
                                } else {
                                    printf("A espera de especialista\n");
                                }
                                fflush(stdout);
                                pthread_mutex_unlock(&mutexPrints);
                                percorre = percorre->next;
                            }
                            printf("\n######");
                            fflush(stdout);
                        }
                    }

                    if (strcmp(comando1, "freq") == 0) {
                        int n = atoi(comando2);
                        if (n <= 0)
                            printf("Valor invalido\n");
                        else
                            timeFreq = n;
                    }

                    if (strcmp(comando1, "especialistas\n") == 0) {
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

                    //TODO
                    if (strcmp(comando1, "delut") == 0) {
                        printf("Segundo comando %s", comando2);
                    } else if (strcmp(comando1, "delesp") == 0) {
                        printf("Segundo comando %s", comando2);
                    }

                    if (strcmp(comando1, "encerra\n") == 0) {

                        //TODO: Notificar que o balcao vai encerrar
                        //TODO: exit gracefully

                    }


                }

            }
        }
    }
}