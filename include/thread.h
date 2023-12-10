#pragma once

#include <pthread.h>

/*An array of thread_ids. Mostly there to provide a simple api that deals with edge cases*/
typedef struct thread_array{
    pthread_t* threadIDs;
    int size;
    int count;
}ThreadArray;

ThreadArray* array_init(int);
pthread_t* array_get_next(ThreadArray*);
void array_joinAll(ThreadArray*);
void array_free(ThreadArray*);