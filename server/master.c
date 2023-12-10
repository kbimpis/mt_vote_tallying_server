#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include "common.h"
#include "buffer.h"
#include "vote.h"
#include "thread.h"

#define BACKLOG 128

char statsFile[64];
ThreadArray* workerThreads;


void sigint_handler(int signum){

    /*Now all worker-threads can finish*/
    finished = 1;

    /*Wake up all worker-threads that are stuck in a pthread_cond_wait() call*/
    ASSERT(pthread_cond_broadcast(&nonEmptyCond));

    /*Call pthread_join on all worker-threads*/
    array_joinAll(workerThreads);

    array_free(workerThreads);
    buffer_free();

    /*Record the stats for this run*/
    stats(statsFile);

    /*End master thread*/
    pthread_exit(NULL);
}

/*Creates as many worker-threads as instructed via the command-line, accepts requests from clients, 
and puts the client sockets in a buffer. Repeats until it receives a SIGINT signal*/
void* master(void* argp){
    MasterInfo m = * ( (MasterInfo*) argp);

    const int PORT = m.port;
    const int THREADS = m.nThreads;
    strcpy(statsFile,m.statsFile);
 
    int sock, newsock;
    struct sockaddr_in server, client;
    socklen_t clientlen = 0; //To prevent warning later

    /*Create an array of THREADS worker-threads*/
    workerThreads = array_init(THREADS);

    WorkerInfo worker_info;
    worker_info.portNumber = PORT;
    worker_info.logFD = m.logFD;
    
    /*Create the worker-threads*/
    for (int i=0; i<THREADS; i++){
        pthread_t* newWorker = array_get_next(workerThreads);
        ASSERT(pthread_create(newWorker,NULL,worker,&worker_info));
    }

    /*Now that the worker-threads have been created, assign sigint_handler to a potential SIGINT signal*/
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT,&sa,NULL);

    memset(&client,0,sizeof(client));

    struct sockaddr *serverptr=(struct sockaddr *)&server;
    struct sockaddr *clientptr=(struct sockaddr *)&client;

    ASSERT((sock = socket(AF_INET,SOCK_STREAM,0)));


    /*Initialize the server struct*/
    memset(&server,0,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);newsock = accept(sock, clientptr, &clientlen);
    server.sin_port = htons(PORT);

    ASSERT(bind(sock,serverptr,sizeof(server)));

    ASSERT(listen(sock,BACKLOG));

    while (1){
        ASSERT(pthread_mutex_lock(&bufMutex));

        if (bufferIsFull()) ASSERT(pthread_cond_wait(&nonFullCond,&bufMutex));
        
        ASSERT((newsock = accept(sock, clientptr, &clientlen)));

        /*Insert new file descriptor to the buffer*/
        buffer_push(newsock);

        ASSERT(pthread_mutex_unlock(&bufMutex));
        ASSERT(pthread_cond_signal(&nonEmptyCond));
    }
}
