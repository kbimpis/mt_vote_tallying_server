#include "thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

ThreadArray* array_init(int size){
    ThreadArray* tArray = malloc(sizeof(*tArray));
    tArray->size = size;
    tArray->count = 0;
    tArray->threadIDs = malloc(tArray->size * sizeof(*tArray->threadIDs));

    return tArray;
}

/*Return the pointer to where the next thread_id will be stored.
If the array is full, double its size*/
pthread_t* array_get_next(ThreadArray* t){
    if (t->count >= (t->size - 1)){
        t->size *= 2;
        pthread_t* temp = realloc(t->threadIDs,t->size*sizeof(*temp));
        int tries=0;
        while (!temp && tries<10){
            temp = realloc(t->threadIDs,t->size*sizeof(*temp));
            tries++;
        }
        if (!temp) perror_exit("realloc");
        t->threadIDs = temp;
    }
    pthread_t* next = &t->threadIDs[t->count++];
    t->count++;
    return next;
}

void array_joinAll(ThreadArray* t){
    for (int i=0; i<t->count; i++){
        ASSERT(pthread_join(t->threadIDs[i++],NULL));
    }
}

void array_free(ThreadArray* t){
    free(t->threadIDs);
    free(t);
}