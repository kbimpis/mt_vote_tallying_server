#include "vote.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#define DEFAULT_SIZE 100

void record_init(){
    rec.count = 0;
    rec.size = DEFAULT_SIZE; 
    rec.votes = malloc(rec.size * sizeof(Vote));
}

Vote vote_init(char* string){
    Vote newVote;
    char temp[BUFSIZE];

    strcpy(temp,string);
    strcpy(newVote.voter,strtok(temp," "));
    strcat(newVote.voter,"_");
    strcat(newVote.voter,strtok(NULL," "));
    strcpy(newVote.party,strtok(NULL,"\0"));

    return newVote;
}

int already_voted(const char* voter){
    for (int i=0; i<rec.count; i++){
        if (strcmp(rec.votes[i].voter,voter) == 0) return 1;
    }
    return 0;
}

void record_vote(Vote v){
    if (rec.count >= rec.size){
        rec.size *= 2;
        Vote* temp = (Vote*) realloc(rec.votes,rec.size*sizeof(*rec.votes));
        int tries = 0;
        while (!temp && tries<10){
            temp = (Vote*) realloc(rec.votes,rec.size*sizeof(*rec.votes));
            tries++;
        }
        if (!temp){
            perror("realloc");
            exit(1);
        }
        rec.votes = temp;

    }
    rec.votes[rec.count++] = v;
}

/*Record the number of votes each party got*/
void stats(char* filename){
    FILE* fp = fopen(filename,"w");

    /*Assume at least 2 choices*/
    int pv_size = 2;

    int pv_count = 0;
    PartyVotes* pv = malloc(pv_size*sizeof(*pv));


    char party[SMALL_STRING_SIZE];

    for (int i=0; i<rec.count; i++){
        int found = 0;
        strcpy(party,rec.votes[i].party);

        for (int i=0; i<pv_count; i++){
            if (strcmp(pv[i].party_name,party) == 0){
                pv[i].votes++;
                found = 1;
                break;
            }
        }
        if (!found){
            if (pv_count >= (pv_size - 1)){
                pv_size *= 2;
                PartyVotes* temp = realloc(pv,pv_size*sizeof(*pv));
                
                int tries=0;
                while (!temp && tries<10){
                    temp = realloc(pv,pv_size*sizeof(*pv));
                    tries++;
                }
                if (!temp){
                    perror("realloc");
                    exit(1);
                }
                pv = temp;
            }
            strcpy(pv[pv_count].party_name,party);
            pv[pv_count++].votes = 1;
        }
    }
    free(rec.votes);

    for (int i=0; i<pv_count; i++){
        fprintf(fp,"%s %d\n",pv[i].party_name,pv[i].votes);
    }
    
    free(pv);
    fclose(fp);

}