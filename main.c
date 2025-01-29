#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    TList* list;
    void* data;
    int threadId;
} ThreadArgs;

void* putThreadFunction(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    putItem(args->list, args->data);
    printf("Wątek %d: dodano element %s\n", args->threadId, (char*)args->data);
    return NULL;
}

void* removeThreadFunction(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    int result = removeItem(args->list, args->data);
    printf("Wątek %d: próba usunięcia elementu %s, wynik: %d\n", 
           args->threadId, (char*)args->data, result);
    return NULL;
}

void* getThreadFunction(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    printf("Wątek %d: czekanie na element...\n", args->threadId);
    void* item = getItem(args->list);
    if(item) {
        printf("Wątek %d: pobrano element %s\n", args->threadId, (char*)item);
        free(item);
    }
    return NULL;
}

int main() {
    printf("\n=== Test 1: Dodawanie i usuwanie elementów ===\n");
    TList* list = createList(5);
    pthread_t threads[6];
    ThreadArgs args[6];
    
    char* data1 = strdup("Item1");
    char* data2 = strdup("Item2");
    char* data3 = strdup("Item3");

    // Dodawanie elementów
    args[0].list = list;
    args[0].data = data1;
    args[0].threadId = 1;
    pthread_create(&threads[0], NULL, putThreadFunction, &args[0]);

    args[1].list = list;
    args[1].data = data2;
    args[1].threadId = 2;
    pthread_create(&threads[1], NULL, putThreadFunction, &args[1]);

    args[2].list = list;
    args[2].data = data3;
    args[2].threadId = 3;
    pthread_create(&threads[2], NULL, putThreadFunction, &args[2]);

    // Czekamy na dodanie elementów
    for(int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\nStan listy po dodaniu elementów:\n");
    showList(list);

    // Usuwanie elementów
    args[3].list = list;
    args[3].data = data1;
    args[3].threadId = 4;
    pthread_create(&threads[3], NULL, removeThreadFunction, &args[3]);

    args[4].list = list;
    args[4].data = data2;
    args[4].threadId = 5;
    pthread_create(&threads[4], NULL, removeThreadFunction, &args[4]);

    args[5].list = list;
    args[5].data = data2; 
    args[5].threadId = 6;
    pthread_create(&threads[5], NULL, removeThreadFunction, &args[5]);

    
    for(int i = 3; i < 6; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\nStan końcowy listy:\n");
    showList(list);
    destroyList(list);

    printf("\n=== Test 2: Zmiana rozmiaru listy ===\n");
    list = createList(3);
    
    // Dodajemy 3 elementy
    char* item1 = strdup("First");
    char* item2 = strdup("Second");
    char* item3 = strdup("Third");
    
    args[0].list = list;
    args[0].data = item1;
    args[0].threadId = 1;
    putThreadFunction(&args[0]);
    
    args[1].list = list;
    args[1].data = item2;
    args[1].threadId = 2;
    putThreadFunction(&args[1]);
    
    args[2].list = list;
    args[2].data = item3;
    args[2].threadId = 3;
    putThreadFunction(&args[2]);

    printf("\nStan listy przed zmianą rozmiaru:\n");
    showList(list);
    
    printf("\nZmniejszanie rozmiaru listy do 2...\n");
    setMaxSize(list, 2);

    // Próba usunięcia elementu
    args[3].list = list;
    args[3].data = item1;
    args[3].threadId = 4;
    removeThreadFunction(&args[3]);

    printf("\nStan końcowy listy:\n");
    showList(list);
    destroyList(list);

    return 0;
}