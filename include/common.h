#pragma once
#include <pthread.h>

/*Macro to avoid if statements*/
#define ASSERT(result){ \
    if (result < 0){  \
        perror("error"); \
        exit(1); \
    } \
};

void* master(void* argp);
void* worker(void* argp);
void perror_exit(char*);

#define SEND_NAME_MSG "SEND NAME PLEASE"
#define ALREADY_VOTED_MSG "ALREADY VOTED"
#define SEND_VOTE_MSG "SEND VOTE PLEASE"

#define BUFSIZE 128
#define SMALL_STRING_SIZE 24

/*Stores information for the master thread*/
typedef struct master_info{
    int nThreads;
    int port;
    int logFD;
    char statsFile[64];
} MasterInfo;

/*Stores information for worker threads*/
typedef struct worker_struct{
    int portNumber;
    int logFD;
} WorkerInfo;

/*Whether SIGINT has been sent to master thread*/
int finished;

pthread_cond_t nonEmptyCond;
pthread_cond_t nonFullCond;

pthread_mutex_t logMutex;
pthread_cond_t logCond;

pthread_mutex_t bufMutex;