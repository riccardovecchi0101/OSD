#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#define DIM 5
#define NTIMES 10;


typedef struct{
    char dato[5]; /* stringa che rappresenta il vero messaggio */
    int iter; /* valore della iterazione corrente */
} Messaggio;


Messaggio buffer[5];

sem_t spazio_disponibile;
sem_t messaggio_disponibile;

pthread_mutex_t mtx_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx_2 = PTHREAD_MUTEX_INITIALIZER;

size_t punt1;
size_t punt2;

void* eseguiProduttore(void* arg) {

    int id = *(int*)arg;  // Otteniamo l'ID passato come argomento
    int *result = malloc(sizeof(int));  // Allocazione dinamica del risultato
    int risorsa;
    Messaggio m;
    char contenuto[5];
    size_t con = NTIMES;
    pthread_t tid = pthread_self();
    
    contenuto[0] = id + 48;
    contenuto[1] = 0;

    for(size_t i = 0; i < con; i++){
        
        sem_wait(&spazio_disponibile);
        if(pthread_mutex_lock(&mtx_1) != 0){
            printf("\n ERRORE ACQUISIZIONE DEL MUTEX 1 DA PARTE DEL FIGLIO %d CON TID %lu\n", id, (unsigned long)tid);
            pthread_exit((void*)-1);
        }

        strcpy(m.dato, contenuto);
        m.iter = i;
        buffer[punt1] = m;
        printf("\n Sono IL FIGLIO CON TID %lu DI INDICE %d E HO DEPOSITATO IL SEGUENTE MESSAGGIO: contenuto: %s, iter: %d", (unsigned long)tid, id, m.dato, m.iter);
        punt1 = (punt1 + 1) % DIM;

        if(pthread_mutex_unlock(&mtx_1) != 0){
            printf("\n ERRORE RILASCIO DEL MUTEX DA PARTE DEL FIGLIO %d CON TID %lu\n", id, (unsigned long)tid);
            pthread_exit((void*)-1);
        }

        sem_post(&messaggio_disponibile);
    }

    *result = id;
    pthread_exit((void*)result);  // Restituisce il puntatore al risultato
}



void* eseguiConsumatore(void* arg) {

    int id = *(int*)arg;  // Otteniamo l'ID passato come argomento
    int *result = malloc(sizeof(int));  // Allocazione dinamica del risultato
    int risorsa;
    Messaggio m;
    size_t con = NTIMES;
    pthread_t tid = pthread_self();

    for(size_t i = 0; i < con; i++){

        sem_wait(&messaggio_disponibile);
        if(pthread_mutex_lock(&mtx_2) != 0){
            printf("\n ERRORE ACQUISIZIONE DEL MUTEX 2 DA PARTE DEL FIGLIO %d CON TID %lu\n", id, (unsigned long)tid);
            pthread_exit((void*)-1);
        }
   
        m = buffer[punt2];
        printf("\n Sono IL FIGLIO CON TID %lu DI INDICE %d E HO PRELEVATO IL SEGUENTE MESSAGGIO: contenuto: %s, iter: %d", (unsigned long)tid, id, m.dato, m.iter);

        punt2 = (punt2 + 1) % DIM;

        if(pthread_mutex_unlock(&mtx_2) != 0){
            printf("\n ERRORE RILASCIO DEL MUTEX DA PARTE DEL FIGLIO %d CON TID %lu\n", id, (unsigned long)tid);
            pthread_exit((void*)-1);
        }

        sem_post(&spazio_disponibile);

        sleep(2);
    }        

    *result = id;
    pthread_exit((void*)result);  // Restituisce il puntatore al risultato
}





int main(int argc, char** argv) {

	if(argc != 2){
		printf("\n %s TAKES ONE ARGUMENT \n", argv[0]);
		exit(-1);
	}


    int N = atoi(argv[1]);
    pthread_t threads[N];   
    int* ids = malloc(N*sizeof(int)); 
    int *result;
    int con = 0;

    sem_init(&spazio_disponibile, 0, DIM);
    sem_init(&messaggio_disponibile, 0, 0);
   
    punt1 = 0;
    punt2 = 0;

    for(int i = 0; i < N ; i++)
        ids[i] = i;


    for (int i = 0; i < N; i++) {
        if(i < N/2){
            if (pthread_create(&threads[i], NULL, eseguiProduttore, &ids[i])) {
                fprintf(stderr, "Errore nella creazione del thread %d\n", i);
                return 1;
            }
        }

        else{
            if (pthread_create(&threads[i], NULL, eseguiConsumatore, &ids[i])) {
                fprintf(stderr, "Errore nella creazione del thread %d\n", i);
                return 1;
            }
        }
    }

  
    for (int i = 0; i < N; i++) {
        if (pthread_join(threads[i], (void**)&result)) {
            fprintf(stderr, "Errore nella terminazione del thread %d\n", i);
            return 2;
        }
        printf("\nIl thread %d restituisce: %d\n", i , *result);
    }

    free(result);  
    free(ids);
    pthread_mutex_destroy(&mtx_1);
    pthread_mutex_destroy(&mtx_2);
    sem_destroy(&spazio_disponibile);
    sem_destroy(&messaggio_disponibile);

    return 0;
}
    