#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include "common.h"
#include "buffer.h"
#include "vote.h"

#define CMDLINE_ARGS 6

/*If ASSERT(see common.h) cannot deal with an error, perror_exit is called*/
void perror_exit(char* string){
    perror(string);
    exit(1);
}

/*Where the server starts*/
int main(int argc, char* argv[]){

    /*Make sure command-line parameters are acceptable*/
    if (argc != CMDLINE_ARGS) perror_exit("Usage: poller [portnum] [numWorkerthreads] [bufferSize] [poll-log] [poll-stats]");
    const int portNum = atoi(argv[1]);
    const int numWorkerThreads = atoi(argv[2]);
    if (numWorkerThreads <= 0) perror_exit("numWorkerThreads must be positive");
    const int bufferSize = atoi(argv[3]);
    if (bufferSize <= 0) perror_exit("bufferSize must be positive");

    /*Initialize with 0*/
    finished = 0;

    /*Initialize all mutexes and condition variables*/
    ASSERT(pthread_cond_init(&nonFullCond,NULL));
    ASSERT(pthread_cond_init(&nonEmptyCond,NULL));
    ASSERT(pthread_mutex_init(&bufMutex,NULL));
    ASSERT(pthread_mutex_init(&logMutex,NULL));
    ASSERT(pthread_cond_init(&logCond,NULL));

    /*Open poll-log file*/
    int logFD;
    ASSERT((logFD = open(argv[4],O_CREAT | O_TRUNC | O_RDWR, 0666)));

    /*Initialize the buffer and record structs*/
    buffer_init(bufferSize);    
    record_init();

    /*The parameter that will be passed to the master thread*/
    MasterInfo info;
    info.nThreads = numWorkerThreads;
    info.port = portNum;
    info.logFD = logFD;
    strcpy(info.statsFile,argv[5]);

    /*Create master thread*/
    pthread_t master_thread;
    ASSERT(pthread_create(&master_thread,NULL,master,&info));

    /*Block SIGINT*/
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset,SIGINT);
    pthread_sigmask(SIG_BLOCK,&sigset,NULL);

    /*Wait for master thread to end*/
    ASSERT(pthread_join(master_thread,NULL));
}