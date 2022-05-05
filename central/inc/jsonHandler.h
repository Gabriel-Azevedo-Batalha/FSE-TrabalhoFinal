#ifndef JSON_READER_H
#define JSON_READER_H

#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"

typedef struct dispositivo{
    int num;
    int ativo;
    char *id;
    char *comodo;
    char *topico;
    int energia;
    int entrada;
    char *nomeEntrada;
    int acionaAlarme;
    int saida;
    char *nomeSaida;
    int dimerizavel;
    int temperatura;
    int umidade;
} dispositivo;

char *jParseString(cJSON *json, char *attribute);
int jParseInt(cJSON *json, char *attribute);
char *cadastroJson(dispositivo d);
char *mudaSaidaJson(int saida);
char *readType(char *jsonString);
int readEnergy(char *jsonString);
char *descadastroJson();

#endif
