#pragma once
#include "common.h"

/*A single vote. Contains the (conjoined) name of the voter and the party they voted for*/
typedef struct vote{
    char voter[2*SMALL_STRING_SIZE];
    char party[SMALL_STRING_SIZE];
} Vote;

/*Contains the name of a party, and the total number of votes it got*/
typedef struct party_votes{
    char party_name[SMALL_STRING_SIZE];
    int votes;
} PartyVotes;

/*Contains every single vote. Useful for finding out whether someone has already voted, as well as
recording the final statistics, without reading from the poll-log file*/
typedef struct record{
    Vote* votes;
    int size;
    int count;
}Record;

Record rec;


void record_init();
int already_voted(const char* voter);
void record_vote(Vote v);
void stats(char* filename);
Vote vote_init(char* string);

