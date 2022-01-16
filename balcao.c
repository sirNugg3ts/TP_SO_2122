#include "balcao.h"
#include "utils.h"

void unlinkPipes();
void informShutdown(pUtenteContainer pContainer, pEspecialista pEspecialista);
void apresentaMenu();
void murder(int s);
void reportEspecialidadeFull(Utente u1);
void writeMessageToFIFO(char fifo[],char message[]);
int obtemVariaveisAmbiente(struct Balcao *balcao);
void inicializaStruct(struct Balcao *balcao);
void createFifos();

//THREADS
void *recebeUtentes(void *Dados) {

    DADOS_REG_UTENTES *dadosRegUtentes = (DADOS_REG_UTENTES *) Dados;
    Utente u1;

    pthread_mutex_lock(dadosRegUtentes->mutexPrints);
    fprintf(stdout, "\nA espera de novos utentes\n");
    fflush(stdout);
    pthread_mutex_unlock(dadosRegUtentes->mutexPrints);

    while (dadosRegUtentes->stopReceiving) {
        int fdServerFifo = open(SERVER_FIFO, O_RDONLY);
        if (fdServerFifo == -1) {
            fprintf(stderr, "\n[recebeUtentes] Erro ao abrir SERVER_FIFO");
            fflush(stderr);
            unlinkPipes();
            exit(EXIT_FAILURE);
        }

        int size = read(fdServerFifo, &u1, sizeof(u1));

        if (size > 0) { //Novo Utente

            char CLIENT_FIFO_FINAL[MAX_STRING_SIZE], bufferClassificador[MAX_STRING_SIZE];
            char *token;
            int fdResposta;
            int nUtentesAfrente = 0;
            sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, u1.pid);

            if ((*dadosRegUtentes->nUtentesLigados) == (dadosRegUtentes->nMaxClientes)) //Servidor está cheio
            {
                strcpy(u1.nomeUtente, "SERVERFULL");
                fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                write(fdResposta, &u1, sizeof(u1));
                close(fdResposta);
            }
            else //Servidor não se encontra cheio
            {
                pUtente newUtente = malloc(sizeof(Utente));

                //transmitir ao classificador os sintomas
                write(dadosRegUtentes->fd_balcao_classificador[1], u1.sintomas, strlen(u1.sintomas));
                int size1 = read(dadosRegUtentes->fd_classificador_balcao[0], bufferClassificador,sizeof(bufferClassificador));
                bufferClassificador[size1] = '\0';

                //split
                token = strtok(bufferClassificador, " "); //
                strcpy(u1.especialidadeAtribuida, token);

                while (token != NULL) {
                    u1.prioridadeAtribuida = atoi(token);
                    token = strtok(NULL, "\0");
                }

                *newUtente = u1; //Copiar informação da variável u1 para a estrutura alocada em memória
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
                    fprintf(stdout, "[recebeUtentes] Novo Utente:\nNome: %s\nSintomas: %s\nEspecialidade: %s\nPID:%d\n", newUtente->nomeUtente,
                            newUtente->sintomas, newUtente->especialidadeAtribuida,newUtente->pid);
                    fflush(stdout);
                    pthread_mutex_unlock(dadosRegUtentes->mutexPrints);

                } else//já exitem utentes
                {
                    pUtente percorre = dadosRegUtentes->listaUtentes->first;

                    //verificar se a especialidade está cheia
                    if (strcmp(newUtente->especialidadeAtribuida, "oftalmologia") == 0) {
                        if (dadosRegUtentes->nUtentesEspecialidade[0] == 5)
                            reportEspecialidadeFull(u1);
                        else {
                            (dadosRegUtentes->nUtentesEspecialidade[0])++;
                            nUtentesAfrente = (dadosRegUtentes->nUtentesEspecialidade[0]) - 1;
                        }
                    }

                    if (strcmp(newUtente->especialidadeAtribuida, "neurologia") == 0) {
                        if (dadosRegUtentes->nUtentesEspecialidade[1] == 5)
                            reportEspecialidadeFull(u1);
                        else {
                            (dadosRegUtentes->nUtentesEspecialidade[1])++;
                            nUtentesAfrente = (dadosRegUtentes->nUtentesEspecialidade[1]) - 1;
                        }
                    }

                    if (strcmp(newUtente->especialidadeAtribuida, "estomatologia") == 0) {
                        if (dadosRegUtentes->nUtentesEspecialidade[2] == 5)
                            reportEspecialidadeFull(u1);
                        else {
                            (dadosRegUtentes->nUtentesEspecialidade[2])++;
                            nUtentesAfrente = (dadosRegUtentes->nUtentesEspecialidade[2]) - 1;
                        }
                    }

                    if (strcmp(newUtente->especialidadeAtribuida, "ortopedia") == 0) {
                        if (dadosRegUtentes->nUtentesEspecialidade[3] == 5)
                            reportEspecialidadeFull(u1);
                        else {
                            (dadosRegUtentes->nUtentesEspecialidade[3])++;
                            nUtentesAfrente = (dadosRegUtentes->nUtentesEspecialidade[3]) - 1;
                        }
                    }

                    if (strcmp(newUtente->especialidadeAtribuida, "geral") == 0){
                        if(dadosRegUtentes->nUtentesEspecialidade[4] == 5)
                            reportEspecialidadeFull(u1);
                        else {
                            (dadosRegUtentes->nUtentesEspecialidade[4])++;
                            nUtentesAfrente = (dadosRegUtentes->nUtentesEspecialidade[4]) - 1;
                        }
                    }

                    //já se verificou, não está cheio, contador nUtentesEspecialista aumentado

                    //inserir utente na lista
                    (*dadosRegUtentes->nUtentesLigados)++;

                    pthread_mutex_lock(dadosRegUtentes->mutexPrints);
                    fprintf(stdout, "[recebeUtentes] Novo Utente:\nNome: %s\nSintomas: %s\nEspecialidade: %s\nPID:%d\n", newUtente->nomeUtente,
                            newUtente->sintomas, newUtente->especialidadeAtribuida,newUtente->pid);
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
                    }
                    else // existem mais que um
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

                //devolver estrutura atualizada ao utente, confirmando o seu registo no balcão
                fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
                write(fdResposta, &u1, sizeof(u1));
                close(fdResposta);

                int nEspecialistas = 0;
                char mensagem[MAX_STRING_SIZE];
                char finalMessage[MAX_STRING_SIZE];

                //contar numero de especialistas registados no balcão no momento
                pthread_mutex_lock(dadosRegUtentes->mutexListaMedicos);
                pEspecialista percorreEspecialistas = dadosRegUtentes->listaEspecialistas->next;
                while (percorreEspecialistas != NULL) {
                    if (strcmp(percorreEspecialistas->especialidade, u1.especialidadeAtribuida) == 0)
                        nEspecialistas++;
                    percorreEspecialistas = percorreEspecialistas->next;
                }
                pthread_mutex_unlock(dadosRegUtentes->mutexListaMedicos);

                strcpy(mensagem, "Especialidade: %s\nPrioridade: %d\nUtentes em fila:%d"
                                 "\nEspecialistas: %d");
                sprintf(finalMessage, mensagem, newUtente->especialidadeAtribuida, newUtente->prioridadeAtribuida,
                        nUtentesAfrente, nEspecialistas);

                writeMessageToFIFO(CLIENT_FIFO_FINAL,finalMessage);
            }
        }
    }
    pthread_exit(NULL);
}

void *recebeMedicos(void *Dados) {

    DADOS_REG_MEDICOS *dados = (DADOS_REG_MEDICOS *) Dados;

    pthread_mutex_lock(dados->mutexPrints);
    fprintf(stdout, "\nA espera de novos especialistas...");
    fflush(stdout);
    pthread_mutex_unlock(dados->mutexPrints);

    while (dados->stopReceiving) {

        Especialista e;
        int fdServer = open(SERVER_FIFO_FOR_MEDICS, O_RDONLY);
        char MEDICO_FIFO_FINAL[MAX_STRING_SIZE];

        if (fdServer == -1) {
            fprintf(stderr, "\n[recebeMedicos]Erro ao abrir SERVER_FIFO_FOR_MEDICS");
            fflush(stderr);
            unlinkPipes();
            exit(EXIT_FAILURE);
        }

        int size = read(fdServer, &e, sizeof(e));
        if (size > 0) {
            sprintf(MEDICO_FIFO_FINAL, MEDICO_FIFO, e.pid);

            int fdResposta;

            if ((*dados->nMedicosLigados) == (dados->nMaxMedicos))//server is full
            {
                strcpy(e.nomeMedico, "SERVERFULL");
                fdResposta = open(MEDICO_FIFO_FINAL, O_WRONLY);
                write(fdResposta, &e, sizeof(e));
                close(fdResposta);
            } else //server not full
            {
                pthread_mutex_lock(dados->mutexPrints);
                printf("\n[recebeMedicos] Novo medico:\nNome: %s\nEspecialidade: %s\nPID: %d\n", e.nomeMedico, e.especialidade,e.pid);
                fflush(stdout);
                pthread_mutex_unlock(dados->mutexPrints);

                pEspecialista especialita = malloc(sizeof(Especialista));

                *especialita = e;

                especialita->ocupado = 0;
                especialita->next = NULL;
                especialita->pidServer = getpid();

                pthread_mutex_lock(dados->mutexListaMedicos);
                if (dados->lista->next == NULL)
                    dados->lista->next = especialita;
                else {
                    pEspecialista percorre = dados->lista->next;
                    while (percorre->next != NULL)
                        percorre = percorre->next;
                    percorre->next = especialita;
                }

                (*dados->nMedicosLigados)++;
                pthread_mutex_unlock(dados->mutexListaMedicos);

                sprintf(MEDICO_FIFO_FINAL, MEDICO_FIFO, e.pid);
                fdResposta = open(MEDICO_FIFO_FINAL, O_WRONLY);
                write(fdResposta, especialita, sizeof(Especialista));
                close(fdResposta);
            }
        }
    }
    pthread_exit(NULL);
}

void *apresentaStatusEspera(void *Dados) {
    struct dadosStatus *dadosStatus = (struct dadosStatus *) Dados;

    while (dadosStatus->stopShowing) {
        pthread_mutex_lock(dadosStatus->mutexPrints);
        printf("\n###\nStatus:\n");
        fflush(stdout);
        printf("Oftalmologia: %d\n", dadosStatus->ocupacao[0]);
        printf("Neurologia: %d\n", dadosStatus->ocupacao[1]);
        printf("Estomatologia: %d\n", dadosStatus->ocupacao[2]);
        printf("Ortopedia: %d\n", dadosStatus->ocupacao[3]);
        printf("Geral: %d\n", dadosStatus->ocupacao[4]);
        printf("###\n");
        fflush(stdout);
        pthread_mutex_unlock(dadosStatus->mutexPrints);
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
            if (percorre->atendido == 0) {
                while (percorreEspecialista != NULL) {
                    if (strcmp(percorre->especialidadeAtribuida, percorreEspecialista->especialidade) == 0 && percorreEspecialista->ocupado == 0) {

                        //notificar o cliente que vai comunicar com PID medico
                        char CLIENT_FIFO_FINAL[MAX_STRING_SIZE];
                        sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, percorre->pid);
                        char pidMedico[6];
                        sprintf(pidMedico, "%d", percorreEspecialista->pid);

                        writeMessageToFIFO(CLIENT_FIFO_FINAL,pidMedico);

                        //notificar medico que vai comunicar com PID cliente
                        char MEDICO[MAX_STRING_SIZE];
                        char pidCliente[6];

                        sprintf(pidCliente, "%d", percorre->pid);
                        sprintf(MEDICO, MEDICO_FIFO, percorreEspecialista->pid);
                        writeMessageToFIFO(MEDICO,pidCliente);

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
                        percorreEspecialista->ocupado = 1;
                        percorreEspecialista->pidUtenteAtribuido = percorre->pid;
                        strcpy(percorre->nomeMedico, percorreEspecialista->nomeMedico);
                        strcpy(percorreEspecialista->nomeUtente, percorre->nomeUtente);

                        pthread_mutex_lock(dados->mutexPrints);
                        printf("\n[Gestor] O utente %s sera atendido pelo medico %s\n", percorre->nomeUtente, percorreEspecialista->nomeMedico);
                        fflush(stdout);
                        pthread_mutex_unlock(dados->mutexPrints);
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

void *handleHeartBeats(void *Dados) {
    struct dadosHeartbeat *dadosHeartbeat = (struct dadosHeartbeat *) Dados;

    while (dadosHeartbeat->stop) {
        pEspecialista percorre = dadosHeartbeat->listaEspecialistas->next;
        while (percorre != NULL) {
            (percorre->missedHeartbeats)++;
            if (percorre->missedHeartbeats == 2) {

                pthread_mutex_lock(dadosHeartbeat->mutexPrints);
                printf("\n[HeartBeat] O medico %s com pid %d deixou de dar sinal de vida\n", percorre->nomeMedico,percorre->pid);
                fflush(stdout);
                pthread_mutex_unlock(dadosHeartbeat->mutexPrints);

                //remover especialista
                if (percorre->ocupado) { //estava a atender um utente
                    int found = 0;
                    pUtente percorreUtente = dadosHeartbeat->listaUtentes->first; //procurar o utente para informar que o médico morreu
                    while (percorreUtente->pid != percorre->pidUtenteAtribuido) {
                        if (percorreUtente->pid == percorre->pidUtenteAtribuido) {
                            found = 1;
                            break;
                        } else
                            percorreUtente = percorreUtente->next;
                    }
                    if (!found) {
                        pthread_mutex_lock(dadosHeartbeat->mutexPrints);
                        printf("\n[HeartBeat] O utente que estava a ser atendido por este especialista"
                               "ja abandonou o sistema\n");
                        fflush(stdout);
                        pthread_mutex_unlock(dadosHeartbeat->mutexPrints);
                    } else {
                        char clienteFIFO[MAX_STRING_SIZE];
                        sprintf(clienteFIFO, CLIENT_FIFO, percorreUtente->pid);
                        writeMessageToFIFO(clienteFIFO,"DELUT");

                        pthread_mutex_lock(dadosHeartbeat->mutexPrints);
                        printf("\n[HeartBeat] O Utente que estava a ser atendido por este especialista foi removido\n");
                        fflush(stdout);
                        pthread_mutex_unlock(dadosHeartbeat->mutexPrints);
                    }
                }

                //remover especialista da lista
                pthread_mutex_lock(dadosHeartbeat->mutexEspecialistas);
                pEspecialista beforeRemove = dadosHeartbeat->listaEspecialistas->next;
                if (beforeRemove == percorre) {
                    dadosHeartbeat->listaEspecialistas->next = beforeRemove->next;
                } else {
                    while (beforeRemove->next != percorre)
                        beforeRemove = beforeRemove->next;
                    beforeRemove->next = percorre->next;
                    free(percorre);
                }
                pthread_mutex_unlock(dadosHeartbeat->mutexEspecialistas);
            }
            percorre = percorre->next;
        }
        sleep(20);
    }
    pthread_exit(NULL);
}

int main() {

    struct Balcao balcao; //estrutura que guarda a informação necessária ao balcão
    char comando[MAX_STRING_SIZE], comando1[MAX_STRING_SIZE], comando2[MAX_STRING_SIZE]; //chars para processar input do teclado e dos pipes
    int fdServer, fdServerMedics, fd_balcao_classificador[2], fd_classificador_balcao[2],timeFreq = 30;
    pUtenteContainer listaUtentes = NULL;
    pEspecialista listaEspecialistas = NULL;

    //iniciar signal listener para SIGINT aka ctrl+c
    signal(SIGINT, murder);

    createFifos();

    if (pipe(fd_balcao_classificador) == -1 || pipe(fd_classificador_balcao) == -1) {
        fprintf(stderr, "\nErro - Nao foi possivel criar pipes para o classificador\n");
        fflush(stderr);
        unlinkPipes();
        exit(EXIT_FAILURE);
    }

    //inicialização
    if (obtemVariaveisAmbiente(&balcao) == -1) {
        unlinkPipes();
        exit(EXIT_FAILURE);
    }
    inicializaStruct(&balcao);

    //Run Classificador
    switch (fork()) {
        case -1: {
            //error fork
            fprintf(stderr, "\nErro - Nao foi possivel criar fork\n");
            fflush(stderr);

            //clean pipes
            unlinkPipes();
            exit(EXIT_FAILURE);
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
                exit(EXIT_FAILURE);
            }
            fprintf(stderr, "\nErro ao correr classificador");
            fflush(stderr);
            //clean pipes
            unlinkPipes();
            exit(EXIT_FAILURE);
        }
        default: {//parent

            close(fd_balcao_classificador[0]);
            close(fd_classificador_balcao[1]);

            pthread_t threadRecebeUtentes; //Thread que irá receber os novos utentes
            pthread_t threadRecebeMedicos; //Thread que irá receber os novos especialistas
            pthread_t threadGestor; //Thread que irá emparelhar Utentes e Especialistas
            pthread_t status; //Thread que informa o estado da lista de espera

            pthread_mutex_t mutexListaUtentes;
            pthread_mutex_t mutexListaMedicos;
            pthread_mutex_t mutexPrints;

            //inicializar mutexes
            pthread_mutex_init(&mutexPrints, NULL);
            pthread_mutex_init(&mutexListaUtentes, NULL);
            pthread_mutex_init(&mutexListaMedicos, NULL);

            listaUtentes = malloc(sizeof(pUtenteContainer)); //inicializar lista de utentes
            listaUtentes->first = NULL;

            listaEspecialistas = malloc(sizeof(pEspecialista)); //inicializar lista de especialistas
            listaEspecialistas->next = NULL;

            //thread recebeUtentes
            DADOS_REG_UTENTES dados;
            dados.fd_classificador_balcao = fd_classificador_balcao;
            dados.fd_balcao_classificador = fd_balcao_classificador;
            dados.listaEspecialistas = listaEspecialistas;
            dados.listaUtentes = listaUtentes;
            dados.nUtentesLigados = &balcao.nClienteLigados;
            dados.nUtentesEspecialidade = balcao.nUtentesEspecialidade;
            dados.nMaxClientes = balcao.N;
            dados.mutexPrints = &mutexPrints;
            dados.mutexListaUtentes = &mutexListaUtentes;
            dados.mutexListaMedicos = &mutexListaMedicos;
            dados.stopReceiving = 1;

            if (pthread_create(&threadRecebeUtentes, NULL, recebeUtentes, &dados) != 0) {
                fprintf(stderr, "\nErro ao criar thread [recebeUtentes]\nA terminar...");
                fflush(stderr);
                unlinkPipes();
                exit(EXIT_FAILURE);
            }

            //thread recebeMedicos
            DADOS_REG_MEDICOS dadosMedico;
            dadosMedico.lista = listaEspecialistas;
            dadosMedico.nMaxMedicos = balcao.M;
            dadosMedico.nMedicosLigados = &balcao.nMedicosLigados;
            dadosMedico.stopReceiving = 1;
            dadosMedico.mutexPrints = &mutexPrints;
            dadosMedico.mutexListaMedicos = &mutexListaMedicos;

            if (pthread_create(&threadRecebeMedicos, NULL, recebeMedicos, &dadosMedico) != 0) {
                fprintf(stderr, "\nErro ao criar thread [recebeMedicos]\nA terminar...");
                fflush(stderr);
                unlinkPipes();
                exit(EXIT_FAILURE);
            }

            //thread manage connections
            struct dadosManager dadosManager;
            dadosManager.listaUtentes = listaUtentes;
            dadosManager.listaEspecialistas = listaEspecialistas;
            dadosManager.nUtentesEspecialidade = balcao.nUtentesEspecialidade;
            dadosManager.mutexListaUtentes = &mutexListaUtentes;
            dadosManager.mutexListaMedicos = &mutexListaMedicos;
            dadosManager.mutexPrints = &mutexPrints;
            dadosManager.stop = 1;

            if (pthread_create(&threadGestor, NULL, &estabeleceContacto, &dadosManager) != 0) {
                fprintf(stderr, "\nErro ao criar thread [Gestor]\nA terminar...");
                fflush(stderr);
                unlinkPipes();
                exit(EXIT_FAILURE);
            }

            //thread Estado das filas de espera
            struct dadosStatus dadosStatus;
            dadosStatus.ocupacao = balcao.nUtentesEspecialidade;
            dadosStatus.stopShowing = 1;
            dadosStatus.timeFreq = &timeFreq;
            dadosStatus.mutexPrints = &mutexPrints;

            if (pthread_create(&status, NULL, &apresentaStatusEspera, &dadosStatus) != 0) {
                fprintf(stderr, "\nErro ao criar thread [Status]\nA terminar...");
                fflush(stderr);
                unlinkPipes();
                exit(EXIT_FAILURE);
            }

            //thread handleHeartBeats
            pthread_t heartbeats;
            struct dadosHeartbeat dadosHeartbeat;
            dadosHeartbeat.listaEspecialistas = listaEspecialistas;
            dadosHeartbeat.mutexEspecialistas = &mutexListaMedicos;
            dadosHeartbeat.listaUtentes = listaUtentes;
            dadosHeartbeat.mutexPrints = &mutexPrints;
            dadosHeartbeat.stop = 1;

            if (pthread_create(&heartbeats, NULL, &handleHeartBeats, &dadosHeartbeat) != 0) {
                fprintf(stderr, "\nErro ao criar thread [HeartBeat]\nA terminar...");
                fflush(stderr);
                unlinkPipes();
                exit(EXIT_FAILURE);
            }

            pthread_mutex_lock(&mutexPrints);
            //input teclado
            fprintf(stdout, "\nWrite \"Help\" for a list of commands\n");
            fflush(stdout);
            pthread_mutex_unlock(&mutexPrints);

            int fdComandos = open(BALCAO_COMMANDS, O_RDWR | O_NONBLOCK);
            fd_set read_fds;
            int stopLoop = 1;

            while (stopLoop) {

                FD_ZERO(&read_fds);
                FD_SET(0, &read_fds);
                FD_SET(fdComandos, &read_fds);

                int ndf = select(fdComandos + 1, &read_fds, NULL, NULL, NULL);
                if(ndf == -1){
                    fprintf(stderr, "\n[MAIN]Erro no Select\nA terminar...");
                    fflush(stderr);
                    unlinkPipes();
                    exit(EXIT_FAILURE);
                }
                if (FD_ISSET(fdComandos, &read_fds))
                //comandos para o pipe BALCAO_COMMANDS
                {
                    MSG m;
                    int s = read(fdComandos, &m, sizeof(MSG));

                    if (strcmp(m.msg, "ENCERRA BALCAO") == 0) {
                        //indicar as threads para terminar
                        dados.stopReceiving = 0;
                        dadosMedico.stopReceiving = 0;
                        dadosStatus.stopShowing = 0;
                        dadosManager.stop = 0;
                        dadosHeartbeat.stop = 0;
                        pthread_cancel(threadGestor);
                        pthread_cancel(threadRecebeMedicos);
                        pthread_cancel(threadRecebeUtentes);
                        pthread_cancel(status);
                        informShutdown(listaUtentes,listaEspecialistas);
                        unlinkPipes();
                        printf("\nA terminar, goodbye\n");
                        return EXIT_SUCCESS;
                    } else
                    {
                        strcpy(comando1, strtok(m.msg, " "));

                        if (strcmp(comando1, "ENCERRA") == 0) {
                            strcpy(comando2, strtok(NULL, " "));
                            pEspecialista percorreE = listaEspecialistas->next;
                            int pidTerminar = atoi(comando2);

                            pthread_mutex_lock(&mutexPrints);
                            printf("\n[Commands Balcao]O especialista %s - %s com pid %d vai encerrar\n",percorreE->nomeMedico,percorreE->especialidade,percorreE->pid);
                            fflush(stdout);
                            pthread_mutex_unlock(&mutexPrints);

                            //remover médico da lista de especialistas
                            pthread_mutex_lock(&mutexListaMedicos);

                            if (percorreE->pid == pidTerminar) {
                                listaEspecialistas->next = percorreE->next;
                                free(percorreE);
                            }else{
                                while (percorreE->pid != pidTerminar)
                                    percorreE = percorreE->next;

                                pEspecialista p2 = listaEspecialistas->next;
                                while (p2->next != percorreE)
                                    p2 = p2->next;

                                p2->next = percorreE->next;
                                free(percorreE);
                            }

                            pthread_mutex_unlock(&mutexListaMedicos);
                        }else if (strcmp(comando1, "ADEUS") == 0)
                        {
                            strcpy(comando2, strtok(NULL, " "));
                            int pidTerminar = atoi(comando2);
                            int removed = 0;
                            int wasOccupied = 0;

                            //libertar médico para aceitar novos utentes
                            pthread_mutex_lock(&mutexListaMedicos);
                            pEspecialista percorreE = listaEspecialistas->next;
                            if (percorreE != NULL) {
                                while (percorreE->pidUtenteAtribuido != pidTerminar && percorreE->ocupado == 1 && percorreE != NULL)
                                    percorreE = percorreE->next;
                                if(percorreE->pidUtenteAtribuido == pidTerminar){
                                    percorreE->ocupado = 0;
                                    wasOccupied = 1;
                                }
                            }
                            pthread_mutex_unlock(&mutexListaMedicos);

                            //libertar utente
                            pthread_mutex_lock(&mutexListaUtentes);
                            pUtente percorre = listaUtentes->first;

                            if (percorre->pid == pidTerminar) {
                                if(!wasOccupied){
                                    printf("\n[Commands Balcao]O utente %s com pid %d desistiu\n",percorre->nomeUtente,percorre->pid);
                                    fflush(stdout);
                                }else{
                                    printf("\n[Commands Balcao]Terminada consulta do utente %s com pid %d\n",percorre->nomeUtente,percorre->pid);
                                    fflush(stdout);
                                }
                                listaUtentes->first = percorre->next;
                                free(percorre);
                                removed = 1;
                            } else {
                                while (percorre != NULL && removed==0) {
                                    if (percorre->pid == pidTerminar) {
                                        pUtente delete = listaUtentes->first;
                                        while (delete->next != percorre)
                                            delete = delete->next;
                                        delete->next = percorre->next;
                                        if(!wasOccupied){
                                            printf("\n[Commands Balcao]O utente %s com pid %d desistiu",percorre->nomeUtente,percorre->pid);
                                            fflush(stdout);
                                        }else{
                                            printf("\n[Commands Balcao]Terminada consulta do utente %s com pid %d",percorre->nomeUtente,percorre->pid);
                                            fflush(stdout);
                                        }
                                        free(percorre);
                                        removed = 1;
                                        break;
                                    } else
                                        percorre = percorre->next;
                                }
                            }
                            pthread_mutex_unlock(&mutexListaUtentes);

                        }else if (strcmp(comando1, "HEARTBEAT") == 0) {
                            pthread_mutex_lock(&mutexListaMedicos);
                            pEspecialista percorreE = listaEspecialistas->next;
                            while (percorreE != NULL) {
                                if (percorreE->pid == m.sender) {
                                    percorreE->missedHeartbeats--;
                                    break;
                                } else {
                                    percorreE = percorreE->next;
                                }
                            }
                            pthread_mutex_unlock(&mutexListaMedicos);
                        }
                    }

                } //FIM COMANDOS BALCAO_COMMANDS

                else
                    if (FD_ISSET(0, &read_fds))
                    {
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
                                fprintf(stdout, "\n[MAIN]Nao existem clientes ligados de momento\n");
                                fflush(stdout);
                                pthread_mutex_unlock(&mutexPrints);
                            } else {
                                pUtente percorre = listaUtentes->first;
                                printf("\n###### Utentes ######\n");
                                fflush(stdout);
                                while (percorre != NULL) {
                                    pthread_mutex_lock(&mutexPrints);
                                    printf("\nNome do utente: %s\n"
                                           "PID: %d\n"
                                           "Especialidade: %s\n"
                                           "Prioridade: %d\n", percorre->nomeUtente, percorre->pid,
                                           percorre->especialidadeAtribuida,
                                           percorre->prioridadeAtribuida);
                                    if (percorre->atendido) {
                                        printf("Atendido por: %s\n", percorre->nomeMedico);
                                    } else {
                                        printf("A espera de especialista\n");
                                    }
                                    printf("------");
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
                                printf("[MAIN]Valor invalido\n");
                            else
                                timeFreq = n;
                        }

                        if (strcmp(comando1, "especialistas\n") == 0) {
                            if (balcao.nMedicosLigados == 0) {
                                pthread_mutex_lock(&mutexPrints);
                                fprintf(stdout, "\n[MAIN]Nao existem medicos ligados de momento");
                                fflush(stdout);
                                pthread_mutex_unlock(&mutexPrints);
                            } else {
                                pEspecialista percorre = listaEspecialistas->next;
                                printf("\n###### Especialistas ######\n");
                                while (percorre != NULL) {
                                    pthread_mutex_lock(&mutexPrints);
                                    printf("\nNome do especialista: %s\n"
                                           "PID: %d\n"
                                           "Especialidade: %s\n", percorre->nomeMedico, percorre->pid,
                                           percorre->especialidade);
                                    if (percorre->ocupado)
                                        printf("Encontra-se em consulta com o Utente %s que tem o pid %d\n",
                                               percorre->nomeUtente, percorre->pidUtenteAtribuido);
                                    printf("-----");
                                    fflush(stdout);
                                    pthread_mutex_unlock(&mutexPrints);
                                    percorre = percorre->next;
                                }
                                printf("\n######");
                            }
                        }

                        if (strcmp(comando1, "delut") == 0) {
                            pUtente percorre = listaUtentes->first;
                            int removed = 0;
                            while (percorre != NULL && removed == 0) {

                                if (percorre->pid == atoi(comando2)) {
                                    if(percorre->atendido == 1){
                                        printf("[MAIN] O utente ja se encontra atendido\n"
                                               "Nao pode remover um utente em consulta");
                                        fflush(stdout);
                                        removed = 1;
                                    }else{
                                        //notificar cliente
                                        char clienteFIFO[MAX_STRING_SIZE];
                                        sprintf(clienteFIFO, CLIENT_FIFO, atoi(comando2));
                                        writeMessageToFIFO(clienteFIFO,"DELUT");
                                        removed = 1;
                                    }
                                }
                                percorre = percorre->next;
                            }
                            if (!removed) {
                                printf("\n[MAIN]Utente nao encontrado\n");
                                fflush(stdout);
                            }
                        } else if (strcmp(comando1, "delesp") == 0) {
                            pEspecialista percorre = listaEspecialistas->next;
                            int removed = 0;
                            while (percorre != NULL && removed == 0) {
                                if (percorre->pid == atoi(comando2)) {
                                    if(percorre->ocupado == 1){
                                        printf("[MAIN]O especialista esta em consulta\n"
                                               "Nao pode remover um especialista em consulta");
                                        fflush(stdout);
                                        removed = 1;
                                    }else{
                                        //notificar cliente
                                        char especialistaFiFO[MAX_STRING_SIZE];
                                        sprintf(especialistaFiFO, MEDICO_FIFO, atoi(comando2));
                                        writeMessageToFIFO(especialistaFiFO,"DELUT");
                                        removed = 1;
                                    }
                                }
                                percorre = percorre->next;
                            }
                            if (!removed) {
                                printf("\n[MAIN]Especialista nao encontrado\n");
                                fflush(stdout);
                            }
                        }
                        if (strcmp(comando1, "encerra\n") == 0) {
                            kill(getpid(),SIGINT);
                        }
                }
            }
        }
    }
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

void informShutdown(pUtenteContainer pContainer, pEspecialista pEspecialista) {
    pUtente percorre = pContainer->first;
    Especialista *p = pEspecialista->next;
    MSG msg;
    while (percorre!=NULL){
        char acess[MAX_STRING_SIZE];
        sprintf(acess,CLIENT_FIFO,percorre->pid);
        writeMessageToFIFO(acess,"SHUTDOWN");
        percorre = percorre->next;
    }
    while (p!= NULL){
        char acess[MAX_STRING_SIZE];
        sprintf(acess,MEDICO_FIFO,p->pid);
        writeMessageToFIFO(acess,"SHUTDOWN");
        p = p->next;
    }
}

void writeMessageToFIFO(char fifo[],char message[]){
    MSG msg;
    strcpy(msg.msg,message);
    msg.sender = getpid();
    int fd = open(fifo,O_WRONLY);
    write(fd,&msg,sizeof(MSG));
    close(fd);
}

void reportEspecialidadeFull(Utente u1){
    char CLIENT_FIFO_FINAL[MAX_STRING_SIZE];
    strcpy(u1.nomeUtente, "ESPECIALIDADEFULL");
    sprintf(CLIENT_FIFO_FINAL, CLIENT_FIFO, u1.pid);
    int fdResposta = open(CLIENT_FIFO_FINAL, O_WRONLY);
    write(fdResposta, &u1, sizeof(u1));
    close(fdResposta);
}

void murder(int s) {
    writeMessageToFIFO(BALCAO_COMMANDS,"ENCERRA BALCAO");
}

void apresentaMenu() {
    printf("\n");
    printf("===== Balcao ====\n");
    printf("= utentes\n= especialistas\n= delut X\n= delesp X\n= freq N\n= encerra\n");
    printf("=================\n");
}

void createFifos() {
    //apagar pipes caso já existam
    remove(SERVER_FIFO);
    remove(SERVER_FIFO_FOR_MEDICS);
    remove(BALCAO_COMMANDS);

    //criar os pipes
    if (mkfifo(SERVER_FIFO, 0777) == -1) {
        if (errno == EEXIST) {
            fprintf(stderr, "\nPipe SERVER_FIFO ja exsite");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "\nErro ao criar pipe SERVER_FIFO");
        exit(EXIT_FAILURE);
    }

    if (mkfifo(SERVER_FIFO_FOR_MEDICS, 0777) == -1) {
        if (errno == EEXIST) {
            fprintf(stderr, "\nO pipe SERVER_FIFO_FOR_MEDICS ja existe");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "\nErro ao criar pipe SERVER_FIFO_FOR_MEDICS");
        exit(EXIT_FAILURE);
    }

    if (mkfifo(BALCAO_COMMANDS, 0777) == -1) {
        if (errno == EEXIST) {
            fprintf(stderr, "\nO pipe BALCAO_COMMANDS ja existe");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "\nErro ao criar pipe BALCAO_COMMANDS");
        exit(EXIT_FAILURE);
    }
}

void unlinkPipes(){
    unlink(SERVER_FIFO);
    unlink(SERVER_FIFO_FOR_MEDICS);
    unlink(BALCAO_COMMANDS);
}