#pragma once

/*Buffer to which the client-socket file descriptors are stored*/
typedef struct connection_buffer{
    int start;
    int end;
    int bufSize;
    int count;
    int* clientSockets; 
} ConnectionBuffer;


void buffer_init(int);
int bufferIsFull();
int bufferIsEmpty();
void buffer_free();
int buffer_pop();
void buffer_push(int);