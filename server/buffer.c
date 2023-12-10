#include "buffer.h"
#include <stdlib.h>
#include <stdio.h>

ConnectionBuffer buffer;

void buffer_init(int bufSize){
    buffer.start = 0;
    buffer.end = -1;
    buffer.count = 0;
    buffer.bufSize = bufSize;
    buffer.clientSockets = malloc(buffer.bufSize * sizeof(int));
}

void buffer_free(){
    free(buffer.clientSockets);
}

int bufferIsFull(){
    return buffer.count >= buffer.bufSize;
}

int bufferIsEmpty(){
    return buffer.count <= 0;
}

int buffer_pop(){
    if (buffer.count <= 0) return -1;
    int oldStart = buffer.start;
    buffer.start = (buffer.start + 1) % buffer.bufSize;
    buffer.count--;
    return buffer.clientSockets[oldStart];
}

void buffer_push(int fd){
    buffer.end = (buffer.end + 1) % buffer.bufSize;
    buffer.count++;
    buffer.clientSockets[buffer.end] = fd;
}