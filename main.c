#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>


#define NUM_THREADS 3

// Funkcja dla testu 1 (dodawanie i usuwanie elementów)
void* testAddAndRemove(void* arg) {
    TList* list = (TList*)arg;
    for (int i = 0; i < 10; ++i) {
        char* item = (char*)malloc(20);
        snprintf(item, 20, "Item %d", i);
        putItem(list, item);
        printf("Added: %s\n", item);

        usleep(100000);  // Symulacja opóźnienia

        void* removedItem = getItem(list);
        printf("Removed: %s\n", (char*)removedItem);
        free(removedItem);
    }
    return NULL;
}

// Funkcja dla testu 2 (testowanie pełnej listy)
void* testFullList(void* arg) {
    TList* list = (TList*)arg;
    for (int i = 0; i < 10; ++i) {
        char* item = (char*)malloc(20);
        snprintf(item, 20, "Item %d", i);
        putItem(list, item);
        printf("Added: %s\n", item);
    }
    return NULL;
}

// Funkcja dla testu 3 (usuwanie elementu)
// void* testRemoveItem(void* arg) {
//     TList* list = (TList*)arg;
//     for (int i = 0; i < 10; ++i) {
//         char* item = (char*)malloc(20);
//         snprintf(item, 20, "Item %d", i);
//         putItem(list, item);
//     }

//     // Usuwamy elementy
//     for (int i = 0; i < 5; ++i) {
//         printf("Removing Item %d\n", i);
//         char* itemToRemove = NULL;
// for (struct Node* curr = list->head; curr != NULL; curr = curr->next) {
//     if (strcmp((char*)curr->data, "Item 1") == 0) {
//         itemToRemove = curr->data;
//         break;
//     }
// }
// if (itemToRemove && removeItem(list, itemToRemove) == 1) {
//             printf("Removed Item 1\n");
//         } else {
//             printf("Item not found\n");
//         }
//     }
//     return NULL;
// }
void* testRemoveItem(void* arg) {
    TList* list = (TList*)arg;
    
    // Najpierw opróżniamy listę
    while (list->numElem > 0) {
        void* item = getItem(list);
        free(item);
    }
    
    // Teraz dodajemy własne elementy
    char* items[5];  // zmniejszamy liczbę elementów
    for (int i = 0; i < 5; ++i) {
        items[i] = (char*)malloc(20);
        snprintf(items[i], 20, "Item %d", i);
        putItem(list, items[i]);
        printf("Added for removal: %s\n", items[i]);
    }

    // Próbujemy usunąć konkretny element
    for (int i = 0; i < 3; ++i) {  // zmniejszamy liczbę prób
        printf("Trying to remove Item 1\n");
        struct Node* curr = list->head;
        void* itemToRemove = NULL;
        
        // Szukamy elementu "Item 1"
        while (curr != NULL) {
            if (strcmp((char*)curr->data, "Item 1") == 0) {
                itemToRemove = curr->data;
                break;
            }
            curr = curr->next;
        }
        
        if (itemToRemove && removeItem(list, itemToRemove) == 1) {
            printf("Successfully removed Item 1\n");
        } else {
            printf("Item 1 not found\n");
        }
        usleep(100000);  // krótka przerwa między próbami
    }
    return NULL;
}

int main() {
    TList* list = createList(10);
    pthread_t threads[NUM_THREADS];
    
    // Test 1
    printf("\n=== Test 1: Add and Remove ===\n");
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_create(&threads[i], NULL, testAddAndRemove, list);
    }
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }
    
    // Czyszczenie listy przed testem 2
    while (list->numElem > 0) {
        void* item = getItem(list);
        free(item);
    }
    
    // Test 2
    printf("\n=== Test 2: Full List ===\n");
    pthread_t fullListThread;
    pthread_create(&fullListThread, NULL, testFullList, list);
    pthread_join(fullListThread, NULL);
    
    // Czyszczenie listy przed testem 3
    while (list->numElem > 0) {
        void* item = getItem(list);
        free(item);
    }
    
    // Test 3
    printf("\n=== Test 3: Remove Item ===\n");
    pthread_t removeItemThread;
    pthread_create(&removeItemThread, NULL, testRemoveItem, list);
    pthread_join(removeItemThread, NULL);

    printf("\nFinal list contents:\n");
    showList(list);
    destroyList(list);
    return 0;
}
