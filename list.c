#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "list.h"

TList* createList(int size) { //tworzenie listy o zadanym max rozmiarze

	struct TList* list = (struct TList*)malloc(sizeof(struct TList));
	// obsluga bledu alokowania pamieci
	if (list == NULL) {
		perror("Błąd alokacji pamięci dla listy.");
		return NULL;
	}

    //----------------mutexy----------------
    if (pthread_mutex_init(&list->listMutex, NULL) != 0) {  //inicjalizacja mutexow
        perror("Błąd inicjalizacji listMutex.");
        free(list);
        return NULL;
    }
	//------------zmienne warunkowe----------
    if (pthread_cond_init(&list->notFull, NULL) != 0) {
        perror("Błąd inicjalizacji zmiennej warunkowej notFull.");
        pthread_mutex_destroy(&list->listMutex);
        free(list);
        return NULL;
    }
    if (pthread_cond_init(&list->notEmpty, NULL) != 0) {
        perror("Błąd inicjalizacji zmiennej warunkowej notEmpty.");
        pthread_mutex_destroy(&list->listMutex);
        pthread_cond_destroy(&list->notFull);
        free(list);
        return NULL;
    }
  
	list->maxSize = size;
	list->numElem = 0;
	list->head = NULL;
	list->tail = NULL;
	list->isDestroyed = 0; //lista jest aktywna
	return list;
}

void putItem(TList* list, void *el) { //operacja dodawania elementu el (reprezentowanego wskaźnikiem) na koniec
                                      //listy. Operacja może być blokująca, jeżeli lista zawiera już N lub więcej elementów.
	
    pthread_mutex_lock(&list->listMutex); //zajecie mutexa listy

    while (list->numElem >= list->maxSize) { //lista ma za duzo elementow, wiec watek musi czekac
		if (list->isDestroyed) {
			pthread_mutex_unlock(&list->listMutex); //lista zostala w miedzyczasie usunieta
            return;
		}
        pthread_cond_wait(&list->notFull, &list->listMutex);
    }

	//jesli mozna dodac element
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    if (newNode == NULL) {
        perror("Błąd alokacji pamięci dla węzła.");
		pthread_mutex_unlock(&list->listMutex);
        return;
    }

    newNode->data = el;
    newNode->next = NULL;

    if (list->numElem == 0) { //jesli lista bedzie pusta
        list->head = newNode;
        list->tail = newNode;
    }
    else { // jesli ma juz elementy
        list->tail->next = newNode;
        list->tail = newNode;
    }
    list->numElem++;
	
    pthread_cond_signal(&list->notEmpty); //sygnal dla getItem itp.
    pthread_mutex_unlock(&list->listMutex);
}

void* getItem(TList* list) { //operacja usuwania pierwszego (najstarszego) elementu z listy
    pthread_mutex_lock(&list->listMutex);

    while (list->numElem == 0) { //pusta lista - funkcja moze byc blokujaca
		if (list->isDestroyed){
			pthread_mutex_unlock(&list->listMutex); // lista zostala w miedzyczasie usunieta
            return NULL;
		}
        pthread_cond_wait(&list->notEmpty, &list->listMutex);
    }

	if (list->numElem > 1) { //wiecej niz 1 element
		struct Node* oldHead = list->head;
		struct Node* newHead = list->head->next;
		list->head = newHead;
		void* oldData = oldHead->data;
		free(oldHead);
		list->numElem--;

		if (list->numElem < list->maxSize){ //moze byc sytuaacja ze po usunieciu nadal elementów jest więcej niz maxSize
        	pthread_cond_signal(&list->notFull);
		}
        pthread_mutex_unlock(&list->listMutex);

		return oldData;
	}
	else { // 1 element
		struct Node* oldHead = list->head;
		void* oldData = oldHead->data;
		free(oldHead);
		list->head = NULL;
		list->tail = NULL;
		list->numElem--;

		if (list->numElem < list->maxSize){ //moze byc sytuaacja ze po usunieciu nadal elementów jest więcej niz maxSize
        	pthread_cond_signal(&list->notFull);
		}
        pthread_mutex_unlock(&list->listMutex);

		return oldData;
	}
}

int removeItem(TList* list, void* el) { //operacja usuwania z listy elementu el
	
    pthread_mutex_lock(&list->listMutex); //co jesli dwa watki na raz beda chcialy usunac el z tej samej listy

	if (list->numElem == 0) {
        pthread_mutex_unlock(&list->listMutex);
		return 0; //lista pusta, nie ma takiego elementu
	}

	struct Node* currNode = list->head;
    struct Node* prevNode = NULL;

	while (currNode != NULL) {
        if (currNode->data == el) { // Znaleziono element
            break;
        }
        prevNode = currNode;
        currNode = currNode->next;
    }

	if (currNode == NULL) { // Element nie został znaleziony
        pthread_mutex_unlock(&list->listMutex);
        return -1;
    }

	// Usuwanie elementu
    if (prevNode == NULL) { // Usuwamy pierwszy element (głowę)
        list->head = currNode->next;
        if (list->head == NULL) { // Jeśli lista miała jeden element
            list->tail = NULL;
        }
    } else if (currNode->next == NULL) { // Usuwamy ostatni element (ogon)
        list->tail = prevNode;
        prevNode->next = NULL;
    } else { // Usuwamy element ze środka
        prevNode->next = currNode->next;
    }

    free(currNode->data);
    free(currNode);
    list->numElem--;

    if (list->numElem < list->maxSize) {
        pthread_cond_signal(&list->notFull);
    }

    pthread_mutex_unlock(&list->listMutex); 
    return 1; 
}

int getCount(TList* list) {

	pthread_mutex_lock(&list->listMutex); 
	int count = list->numElem;
	pthread_mutex_unlock(&list->listMutex); 

    return count;
}

void setMaxSize(TList* list, int n) {

	pthread_mutex_lock(&list->listMutex); 
	list->maxSize = n;
	pthread_mutex_unlock(&list->listMutex);
}

void appendItems(TList* list, TList* list2) {

    pthread_mutex_lock(&list->listMutex);
    pthread_mutex_lock(&list2->listMutex);

	if (list2->numElem == 0) { //nie ma co dodawac
        pthread_mutex_unlock(&list2->listMutex);
        pthread_mutex_unlock(&list->listMutex);
		return;
	}
	
	if (list->numElem == 0) { //lista1 jest pusta
		list->head = list2->head;
		list->tail = list2->tail;

		list->numElem += list2->numElem;
		list2->head = NULL;
		list2->tail = NULL;
		list2->numElem = 0;
		if (list2->numElem < list2->maxSize){
			pthread_cond_signal(&list2->notFull);
		}

		if (list->numElem > 0){
			pthread_cond_signal(&list->notEmpty); //np probujemy usunac element z pustej listy czekamy na sygnal
		}                                    //potem laczymy listy wiec juz nie jest pusta- trzeba wyslac sygnal
	}                                  

	else { //wykoanie operacji append
		list->tail->next = list2->head;
		list->tail = list2->tail;

		list->numElem += list2->numElem;
		list2->head = NULL;  //zwolnic pamiec z nodow
		list2->tail = NULL;
		list2->numElem = 0;
		if (list2->numElem < list2->maxSize){ //jesli przed dodaniem byla pelna a watek czeka to juz nie jest
			pthread_cond_signal(&list2->notFull);
		}
	}

    pthread_mutex_unlock(&list2->listMutex);
    pthread_mutex_unlock(&list->listMutex);
}

void destroyList(TList* list) {  //nie bedzie przeszkadzane

	pthread_mutex_lock(&list->listMutex);
	list->isDestroyed =1;

	pthread_cond_broadcast(&list->notFull);  // Budzimy wszystkie wątki oczekujace na ta liste
    pthread_cond_broadcast(&list->notEmpty);

	struct Node* temp = list->head;
	while (temp != NULL) {
		struct Node* next = temp->next;
		free(temp->data);
		free(temp);
		temp = next;
	}

	pthread_mutex_unlock(&list->listMutex);

	pthread_mutex_destroy(&list->listMutex);
    pthread_cond_destroy(&list->notFull);
    pthread_cond_destroy(&list->notEmpty);
	free(list);
}


void showList(TList* list) {  //zakladajac ze lista przetrzymuje lancuchy tekstowe

	pthread_mutex_lock(&list->listMutex); 

	struct Node* currNode = list->head;
	while (currNode != NULL) {
		printf("%s\n", (char*)currNode->data);
		currNode = currNode->next;
		//printf("wywolanie show list\n");
	}

	pthread_mutex_unlock(&list->listMutex);
}

void* popItem(TList* list) {
    pthread_mutex_lock(&list->listMutex);

    while (list->numElem == 0) { 
        if (list->isDestroyed) {
            pthread_mutex_unlock(&list->listMutex);
            return NULL;
        }
        pthread_cond_wait(&list->notEmpty, &list->listMutex);
    }

    void* oldData = NULL;

    if (list->numElem == 1) { // jeden element
        oldData = list->tail->data;
        free(list->tail);
        list->head = NULL;
        list->tail = NULL;
    } else { // więcej niż 1 element
        struct Node* currNode = list->head;
        
        // Znajdujemy przedostatni węzeł
        while (currNode->next != list->tail) {
            currNode = currNode->next;
        }
        
        oldData = list->tail->data;
        free(list->tail);
        list->tail = currNode;
        list->tail->next = NULL;
    }

    list->numElem--;

    if (list->numElem < list->maxSize) {
        pthread_cond_signal(&list->notFull);
    }
    
    pthread_mutex_unlock(&list->listMutex);
    return oldData;
}
