#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>


/**GLOBALI E STRUTTURE*************************************************************************************************/
typedef enum{
	THINKING,
	HUNGRY,
	EATING
}state;


typedef struct{
	state filosofi[5];
	pthread_cond_t cond[5]; //array di variabili condizione per verificare se i bastoncini sono occupati
	pthread_mutex_t mtx; //mutex per la mutua esclusione
}monitor;

monitor cena_filosofi;
size_t NTIMES = 10;
/*********************************************************************************************************************/



/**PROCEDURE**********************************************************************************************************/
void monitor_filosofi_init(monitor* m);
void monitor_filosofi_destroy(monitor* m);
void pick_up(int filosofo_index);
void pick_down(int filosofo_index);
void test(int filosofo_index);


void monitor_filosofi_init(monitor* m){
	for(size_t i = 0; i < 5; i++)
		m->filosofi[i] = THINKING;

	pthread_mutex_init(&(m->mtx), NULL);

	for(size_t i = 0; i < 5; i++)
		 pthread_cond_init(&(m->cond[i]), NULL);

}

void monitor_filosofi_destroy(monitor* m){
	pthread_mutex_destroy(&(m->mtx));

	for(size_t i = 0; i < 5; i++)
		pthread_cond_destroy(&(m->cond[i]));

}

void pick_up(int filosofo_index){
	if(pthread_mutex_lock(&(cena_filosofi.mtx)) != 0){
        printf("\n ERRORE ACQUISIZIONE DEL MUTEX DA PARTE DEL FIGLIO %d", filosofo_index);
        pthread_exit((void*)-1);
    }

	(cena_filosofi.filosofi)[filosofo_index] = HUNGRY;

	test(filosofo_index);

	if((cena_filosofi.filosofi)[filosofo_index] != EATING)
		pthread_cond_wait(&(cena_filosofi.cond[filosofo_index]), &(cena_filosofi.mtx));


	if(pthread_mutex_unlock(&(cena_filosofi.mtx)) != 0){
        printf("\n ERRORE RILASCIO DEL MUTEX DA PARTE DEL FIGLIO %d\n", filosofo_index);
        pthread_exit((void*)-1);
    }

}

void pick_down(int filosofo_index){
	if(pthread_mutex_lock(&(cena_filosofi.mtx)) != 0){
        printf("\n ERRORE ACQUISIZIONE DEL MUTEX DA PARTE DEL FIGLIO %d\n", filosofo_index);
        pthread_exit((void*)-1);
    }

	(cena_filosofi.filosofi)[filosofo_index] = 	THINKING;
	test((filosofo_index + 4) % 5);
	test((filosofo_index + 1)% 5);

	if(pthread_mutex_unlock(&(cena_filosofi.mtx)) != 0){
        printf("\n ERRORE RILASCIO DEL MUTEX DA PARTE DEL FIGLIO %d\n", filosofo_index);
        pthread_exit((void*)-1);
    }
}

void test(int filosofo_index){	
	if(cena_filosofi.filosofi[(filosofo_index + 4) % 5] != EATING 
		&& cena_filosofi.filosofi[filosofo_index] == HUNGRY 
		&& cena_filosofi.filosofi[(filosofo_index + 1) % 5] != EATING)
	{
		cena_filosofi.filosofi[filosofo_index] = EATING;
		pthread_cond_signal(&(cena_filosofi.cond[(filosofo_index)]));
	}

}
/*********************************************************************************************************************/



void* eseguiFilosofo(void* arg) {
    int id = *(int*)arg;  // Otteniamo l'ID passato come argomento
    int *result = malloc(sizeof(int));  // Allocazione dinamica del risultato
    *result = id;
    int risorsa;
    pthread_t tid = pthread_self();

    for(size_t i = 0; i < NTIMES; i++){
    	pick_up(id);
    	printf("\n SONO IL FILOSOFO %d CON TID: %lu E HO OTTENUTO IL PERMESSO DI MANGIARE PER LA %lu\\a VOLTA\n", id+1, tid, i+1);
    	pick_down(id);
    }
    pthread_exit((void*)result);  // Restituisce il puntatore al risultato
}

int main(int argc, char** argv) {
    int N = 5;
    pthread_t threads[N];   
    int* ids = malloc(N*sizeof(int)); 
    int *result;
    int con = 0;
 

    monitor_filosofi_init(&cena_filosofi);

    for(int i = 0; i < N ; i++)
        ids[i] = i;


    for (int i = 0; i < N; i++) {
        if (pthread_create(&threads[i], NULL, eseguiFilosofo, &ids[i])) {
            fprintf(stderr, "Errore nella creazione del thread %d\n", i);
            return 1;
        }
    }

    for (int i = 0; i < N; i++) {
        if (pthread_join(threads[i], (void**)&result)) {
            fprintf(stderr, "Errore nella terminazione del thread %d\n", i);
            return 2;
        }
        printf("\nIl thread %d restituisce: %d\n", i , *result);
        free(result);  
    }

    monitor_filosofi_destroy(&cena_filosofi);
    free(ids);
    return 0;
}
    