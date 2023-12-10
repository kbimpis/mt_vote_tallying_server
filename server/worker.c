#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "buffer.h"
#include "vote.h"

#define READSIZE 1024

/*Takes in a vote, and creates a string representation of it. Caller takes ownership of the string*/
char* create_logString(Vote v){
    /*Size is length of strings + (space) + (newline) + (null_character)*/
    char* string = malloc((strlen(v.voter) + strlen(v.party) + 3) * sizeof(char));
    strcpy(string,v.voter);

    strcat(string," ");
    strcat(string,v.party);
    strcat(string,"\n");

    return string;
}

/*Reads file descriptors from the buffer, and records the client's vote through a series of read/write calls.
If the vote is successful, the worker logs it into poll-log. Repeats until the master thread receives SIGINT signal*/
void* worker(void* argp){
    /*Block SIGINT signal*/
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset,SIGINT);
    pthread_sigmask(SIG_BLOCK,&sigset,NULL);

    WorkerInfo info = * (WorkerInfo*) argp;
    const int PORT = info.portNumber;

    int worker_socket, clientSocket;

    ASSERT((worker_socket = socket(AF_INET,SOCK_STREAM,0)));

    while (!finished){
        ASSERT(pthread_mutex_lock(&bufMutex));
        while (bufferIsEmpty() && !finished){
            ASSERT(pthread_cond_wait(&nonEmptyCond,&bufMutex));
        }

        clientSocket = buffer_pop();

        ASSERT(pthread_mutex_unlock(&bufMutex));
        ASSERT(pthread_cond_signal(&nonFullCond));
        
        /*If the reason for breaking out of the above while() loop is that the server is shutting down, break before
        executing the remaining read/write calls to a now-closed socket file descriptor*/
        if (finished) break;

        ASSERT(write(clientSocket,SEND_NAME_MSG,strlen(SEND_NAME_MSG)));

        char nameBuf[READSIZE];
        ASSERT(read(clientSocket,nameBuf,READSIZE));
 
        if (already_voted(nameBuf)){
            ASSERT(write(clientSocket,ALREADY_VOTED_MSG,strlen(ALREADY_VOTED_MSG)));
        }
        else{
            Vote newVote;
            strcpy(newVote.voter,nameBuf);
            ASSERT(write(clientSocket,SEND_VOTE_MSG,strlen(SEND_VOTE_MSG)));
            char partyBuf[READSIZE];
            ASSERT(read(clientSocket,partyBuf,READSIZE));
            partyBuf[strcspn(partyBuf, "\n")] = 0; //Remove the newline character
            strcpy(newVote.party,partyBuf);

            record_vote(newVote);

            char voteRecorded[BUFSIZE];
            strcpy(voteRecorded,"VOTE FOR ");
            strcat(voteRecorded,partyBuf);
            strcat(voteRecorded," RECORDED");
            ASSERT(write(clientSocket,voteRecorded,strlen(voteRecorded)));

            /*Record vote in the log file*/
            char* logString = create_logString(newVote);
            ASSERT(pthread_mutex_lock(&logMutex));
            ASSERT(write(info.logFD,logString,strlen(logString)));
            ASSERT(pthread_mutex_unlock(&logMutex));

            free(logString);
        }
        /*Close socket to client*/
        shutdown(clientSocket,SHUT_RDWR);
        close(clientSocket);
        ASSERT(pthread_mutex_unlock(&bufMutex));
    }

    /*Call this instead of pthread_exit(NULL) to avoid memory leaks*/
    return (void*) 0;
}