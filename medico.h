#ifndef MEDICO_H
#define MEDICO_H

#define MAX_NAME_SIZE 50
#define HEARTBEAT 20

typedef struct especialista{
    char nomeMedico[MAX_NAME_SIZE];
    char especialidade[MAX_NAME_SIZE];

}Especialista, *pEspecialista;


#endif
