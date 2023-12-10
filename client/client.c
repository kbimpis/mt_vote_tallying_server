#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include "common.h"
#include "vote.h"
#include "thread.h"

#define CMDLINE_ARGS 4
#define NUMBER_THREADS 16

/*I hate that I redefine this, but it's a small function that will likely never need to change*/
void perror_exit(char* string){
    perror(string);
    exit(1);
}

int isSendName(char* msg){
    int cmp = strcmp(SEND_NAME_MSG,msg);
    if (cmp) return 0;
    return 1;
}

int isSendVote(char* msg){
    int cmp = strcmp(SEND_VOTE_MSG,msg);
    if (cmp) return 0;
    return 1;
}

int alreadyVoted(char* msg){
    int cmp = strcmp(ALREADY_VOTED_MSG,msg);
    if (cmp) return 0;
    return 1;
}

int isVoteRecorded(char* msg, char* party){
    char expected[BUFSIZE];
    strcpy(expected,"VOTE FOR ");
    strcat(expected,party);
    strcat(expected," RECORDED");

    int cmp = strcmp(expected,msg);
    if (cmp) return 0;
    return 1;
}


/*The argument passed to client_thread*/
typedef struct client_info{
    char serverName[256];
    int portNum;
    Vote singleVote;
} ClientInfo;

/*Initialize new ClientInfo instance*/
ClientInfo* new_info(char* serverName, int portNum, Vote v){
    ClientInfo* info = malloc(sizeof(*info));
    strcpy(info->serverName,serverName);
    info->portNum = portNum;
    info->singleVote = v;

    return info;
}

/*Takes a vote, and communicates it to the server*/
void* client_thread(void* argp){
    ClientInfo* info = (ClientInfo*) argp;

    int sock;
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*)&server;
    struct hostent *rem;

    ASSERT((sock = socket(AF_INET, SOCK_STREAM, 0)));

    if ((rem = gethostbyname(info->serverName)) == NULL) {	
	   herror("gethostbyname"); exit(1);
    }

    memset(&server,0,sizeof(server));
    server.sin_family = AF_INET;
    memcpy(&server.sin_addr, rem->h_addr_list[0], rem->h_length);
    server.sin_port = htons(info->portNum);


    ASSERT(connect(sock,serverptr,sizeof(server)));

    char buf[BUFSIZE];

    ASSERT(read(sock,buf,BUFSIZE));

    if (!isSendName(buf)) perror_exit("expected \"SEND NAME\"");

    ASSERT(write(sock,info->singleVote.voter,BUFSIZE));
    memset(buf,0,BUFSIZE);
    ASSERT(read(sock,buf,BUFSIZE));

    if(isSendVote(buf)){
        ASSERT(write(sock,info->singleVote.party,BUFSIZE));

        ASSERT(read(sock,buf,BUFSIZE));
    }
    else if (!alreadyVoted(buf)){
        perror_exit("expected \"SEND VOTE\" or \"ALREADY VOTED\"");
    }

    shutdown(sock,SHUT_WR);
    close(sock);
    free(info);
}

int main(int argc, char* argv[]){

    char buf[BUFSIZE];
    if (argc != CMDLINE_ARGS){
        perror_exit("Usage: pollSwayer [serverName] [portNum] [inputFile]");
    }
    const int PORT = atoi(argv[2]);


    FILE* fp = fopen(argv[3],"r");
    if (fp == NULL) perror_exit("fopen");

    ClientInfo* info;
    char SERVER_NAME[BUFSIZE];
    strcpy(SERVER_NAME,argv[1]);


    ThreadArray* t = array_init(NUMBER_THREADS);
    while (fgets(buf,BUFSIZE,fp)){
        Vote newVote = vote_init(buf);

        info = new_info(SERVER_NAME,PORT,newVote);

        pthread_t* newThread = array_get_next(t);

        ASSERT(pthread_create(newThread,NULL,client_thread,info));
    }
    fclose(fp);

    array_joinAll(t);
    array_free(t);

}