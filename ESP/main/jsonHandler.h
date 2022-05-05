#ifndef JSON_READER_H
#define JSON_READER_H

#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"

typedef struct dispositivo{
    char *topico;
    int saida;
    int dimerizavel;
    int num;
} dispositivo;

char *jParseString(cJSON *json, char *attribute);
int jParseInt(cJSON *json, char *attribute);
char *cadastroJson(int energia);
char *readType(char *jsonString);
dispositivo readCadastro(char *jsonString);
int readGPIO(char *jsonString);
char *mudancaEstado(int estado, int num);
char *dhtDataToJson(int data, char *name, int num);
char *solicitarDescadastro(int num);
#endif
