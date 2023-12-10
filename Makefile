INCLUDE = ./include
SERVER = ./server
CLIENT = ./client
SHARED = ./shared
SCRIPTS = ./scripts

CC = gcc
CFLAGS = -I $(INCLUDE) -pthread -lpthread

OBJ_SHARED = $(SHARED)/vote.o $(SHARED)/thread.o
OBJ_SERVER = $(SERVER)/server.o $(SERVER)/master.o $(SERVER)/worker.o $(SERVER)/buffer.o
OBJ_CLIENT = $(CLIENT)/client.o

BIN_SERVER = poller
BIN_CLIENT = pollSwayer

SERVERNAME = localhost #Can also be an actual server
PORTNUM = 5634
THREADS = 8
BUFSIZE = 16
LOGFILE = pollLog.txt
STATSFILE = pollStats.txt

SCRIPT_INPUT = $(SCRIPTS)/create_input.sh
SCRIPT_TALLY = $(SCRIPTS)/tallyVotes.sh
SCRIPT_LOG = $(SCRIPTS)/processLogFile.sh
PARTIES_FILE = political_parties.txt #Source for SCRIPT_INPUT

POLLER_RESULTS = pollerResultsFile.txt
TALLY_RESULTS = tallyResultsFile.txt

INPUT_NORMAL = inputFile.txt
INPUT_TEST = input.txt

VALFLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes

all: $(BIN_SERVER) $(BIN_CLIENT)

.PHONY: server
server: $(BIN_SERVER)
	./$(BIN_SERVER) $(PORTNUM) $(THREADS) $(BUFSIZE) $(LOGFILE) $(STATSFILE)

.PHONY: client
client: $(BIN_CLIENT)
	./$(BIN_CLIENT) $(SERVERNAME) $(PORTNUM) $(INPUT_NORMAL)

$(BIN_SERVER): $(OBJ_SERVER) $(OBJ_SHARED)
	$(CC) $(CFLAGS) $(OBJ_SERVER) $(OBJ_SHARED) -o $(BIN_SERVER)

$(BIN_CLIENT): $(OBJ_CLIENT) $(OBJ_SHARED)
	$(CC) $(CFLAGS) $(OBJ_CLIENT) $(OBJ_SHARED) -o $(BIN_CLIENT)

$(OBJ_SERVER): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_CLIENT): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_SHARED): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

input: $(SCRIPT_INPUT)
	$(SCRIPT_INPUT) $(PARTIES_FILE) $(INPUT_TEST) 50

tally: $(SCRIPT_TALLY)
	$(SCRIPT_TALLY) $(INPUT_NORMAL) $(TALLY_RESULTS)

tally-test: $(SCRIPT_TALLY)
	$(SCRIPT_TALLY) $(INPUT_TEST) $(TALLY_RESULTS)

results: $(SCRIPT_LOG)
	$(SCRIPT_LOG) $(LOGFILE)

difference: $(POLLER_RESULTS) $(TALLY_RESULTS) 
	diff $(POLLER_RESULTS) $(TALLY_RESULTS) 
#if diff returns nothing, server worked properly

normal-results: tally results difference #Run tally script, run results script, test difference

run-client-test: input $(BIN_CLIENT) #Create input (SCRIPT_INPUT) and run the client with that input
	./$(BIN_CLIENT) $(SERVERNAME) $(PORTNUM) $(INPUT_TEST)

test-results: tally-test results difference #Run tally script with test input, run results script, test difference
	
.PHONY: valgrind
valgrind: $(BIN_SERVER)
	valgrind $(VALFLAGS) $(BIN_SERVER) $(PORTNUM) $(THREADS) $(BUFSIZE) $(LOGFILE) $(STATSFILE)

.PHONY: clean
clean:
	rm $(SERVER)/*.o $(CLIENT)/*.o $(SHARED)/*.o $(BIN_SERVER) $(BIN_CLIENT) $(STATSFILE) $(TALLY_RESULTS) $(POLLER_RESULTS) $(INPUT_TEST) 
# $(LOGFILE), $(PARTIES_FILE) and $(INPUT_NORMAL) are not removed since they may be needed for testing the scripts / future runs of the server
