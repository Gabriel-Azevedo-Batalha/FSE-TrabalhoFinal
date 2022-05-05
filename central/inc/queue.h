#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>

typedef struct fila{
	int V;
	struct fila *next;
} fila;

fila *filaInit();
int filaVazia(fila *f);
void insereFila(fila *f, int e);
int removeFila(fila *f);

#endif