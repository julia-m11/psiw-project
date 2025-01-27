#ifndef LCL_LIST_H
#define LCL_LIST_H
#include <pthread.h>

// ==============================================
//
//  Version 1.0, 2025-01-15
//
// ==============================================

struct Node {
    void* data;
    struct Node* next;
};

struct TList {
    struct Node* head;
    struct Node* tail;
    int numElem;
    int maxSize;
    pthread_mutex_t listMutex;
    pthread_cond_t notFull;      
    pthread_cond_t notEmpty;
};

typedef struct TList TList;

TList* createList(int size);
void putItem(TList* list, void* el);
void* getItem(TList* list);
int removeItem(TList* list, void* el);
int getCount(TList* list);
void setMaxSize(TList* list, int n);
void appendItems(TList* list, TList* list2);
void destroyList(TList* list);
void showList(TList* list);
void* popItem(TList* list);


#endif //LCL_LIST_H