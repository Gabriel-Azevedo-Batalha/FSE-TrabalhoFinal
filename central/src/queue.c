#include "queue.h"

fila *filaInit(){
	fila *new = malloc(sizeof(fila));
	new->next = NULL;
    return new;
}

int filaVazia(fila *f){
	return f->next == NULL;
}

void insereFila(fila *f, int e){
	fila *new = malloc(sizeof(fila));
	new->V = e;
	new->next = NULL;
	if (filaVazia(f))
		f->next = new;
	else{
		fila *tmp = f->next;
		while(tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = new;
	}
}

int removeFila(fila *f){

	fila *tmp = f->next;
	f->next = tmp->next;
	int e = tmp->V;
	free(tmp);

	return e;
}
