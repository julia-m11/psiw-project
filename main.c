#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

void* removeThreadFunction(void* list) {
    char* searchData = "data";
    int result = removeItem((TList*)list, searchData);
    printf("Wątek usuwający: próba usunięcia, wynik: %d\n", result);
    return NULL;
}

void* putThreadFunction(void* list) {
    char* data = strdup("data");
    putItem((TList*)list, data);
    printf("Wątek dodający: dodano element '%s'\n", data);
    return NULL;
}

void* getThreadFunction(void* list) {
    printf("Wątek pobierający - czekanie na element...\n");
    void* item = getItem((TList*)list);
    printf("Wątek pobierający: pobrano element '%s'\n", (char*)item);
    free(item);  
    return NULL;
}

int main() {

    //-------------Test 1 użycie removeItem()------------------

    printf("Test 1 - użycie removeItem()\n");
    printf("\n");

    TList* list = createList(0);
    pthread_t thread1, thread2, thread3;

    printf("Lista utworzona, początkowa liczba elementów: %d\n", getCount(list));
    printf("Uruchomienie dwóch wątków usuwających.\n");
    pthread_create(&thread1, NULL, removeThreadFunction, list);
    pthread_create(&thread2, NULL, removeThreadFunction, list);

    sleep(2);

    printf("Dodanie elementu do listy w innym wątku.\n");
    pthread_create(&thread3, NULL, putThreadFunction, list);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    int numElem = getCount(list);
    printf("Ilość elementów na liście: %d \n", numElem);
    printf("Zawartość końcowa:\n");
    showList(list);
    destroyList(list);

    //-------------- Test 2 użycie funkcji blokujacej getItem() -----------------

    printf("\n");
    printf("Test 2 - użycie funkcji blokujacej getItem() z pustą listą.\n");
    printf("\n");

    TList* list1 = createList(0);
    pthread_t thread4, thread5, thread6, thread7;

    printf("Lista utworzona, początkowa liczba elementów: %d\n", getCount(list1));
    printf("Uruchomienie dwóch wątków pobierających.\n");
    pthread_create(&thread4, NULL, getThreadFunction, list1);
    pthread_create(&thread5, NULL, getThreadFunction, list1);

    sleep(2);

    printf("Dodanie elementu do listy w innym wątku.\n");
    pthread_create(&thread6, NULL, putThreadFunction, list1);
    pthread_create(&thread7, NULL, putThreadFunction, list1);

    pthread_join(thread4, NULL);
    pthread_join(thread5, NULL);
    pthread_join(thread6, NULL);
    pthread_join(thread7, NULL);

    int numElem1 = getCount(list1);
    printf("Ilość elementów na liście: %d \n", numElem1);
    
    destroyList(list1);

    //---------------- Test 3 uzycie funkcji setMaxSize() ---------------------

    printf("\n");
    printf("Test 3 - użycie funkcji setMaxSize()");
    printf("\n");

    TList* list2 = createList(0);
    setMaxSize(list2, 3);
    pthread_t thread8, thread9, thread10, thread11, thread12, thread13;

    pthread_create(&thread8, NULL, putThreadFunction, list2);
    pthread_create(&thread9, NULL, putThreadFunction, list2);
    pthread_create(&thread10, NULL, putThreadFunction, list2);

    sleep(2);
    printf("Lista po dodaniu 3 elementów:\n");
    showList(list2);

    printf("Zmniejszenie maksymalnego rozmiaru listy do 2.\n");
    setMaxSize(list2, 2);

    printf("Próba dodania elemntu po zmniejszeniu maksymalnej wielkości - blokada.\n");
    pthread_create(&thread11, NULL, putThreadFunction, list2);
    printf("Oczekiwanie na usunięcie dwóch elementów...\n");
    sleep(2);

    pthread_create(&thread12, NULL, removeThreadFunction, list2);
    pthread_create(&thread13, NULL, removeThreadFunction, list2);

    pthread_join(thread8, NULL);
    pthread_join(thread9, NULL);
    pthread_join(thread10, NULL);
    pthread_join(thread11, NULL);
    pthread_join(thread12, NULL);
    pthread_join(thread13, NULL);

    printf("Stan końcowy listy:\n");
    showList(list2);
    
    destroyList(list2);

    return 0;
}