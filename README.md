# This repository contains the implementation of a vote-tallying program in C. It is implemented in three parts: A multi-threaded server, a multi-threaded client, and some bash scripts for testing purposes. It was completed in the context of a university assignment and awarded full credit.

## Summary
The server is launched, then a client process. The client process opens the input file, containing individual votes, then creates a new thread for each line. Each thread connects to the server (TCP connection) and communicates with it. The communcation follows the protocol:
	- [server]: "SEND NAME"
	- [client]:  <[name]_[surname]>
	- [server]: "SEND VOTE"
	- [client]: <[party_name]>
	- [server]: "VOTE FOR [party_name] RECORDED"

name, surname, and party_name are contained in the line taken from the input file.
Alternatively, if after receiving the voter's full name, the server determines that there is already a vote recorded by that person, instead of sending the "SEND VOTE" message, it will send "ALREADY VOTED" and close the connection.


## Usage

In general, the server process is run by executing
	./poller [portNum] [numWorkerThreads] [bufferSize] [poll-log-file] [poll-stats-file]

The client server is run by executing
	./pollSwayer [serverName] [portNum] [inputFile]

In the above commands,
	- portnum indicates the port-number that the server will listen to.
	- numWorkerThreads indicates the number of worker-threads the server will produce. Those threads will execute the responses to the client requests. Must be >0.
	- bufferSize indicates the size of the buffer that stores connections from clients waiting to be serviced. Must be >0.
	- poll-log-file. The name of the file where each individual vote ( including username and party of choice ) is recorded. No such file need already exist, and if one exists, it is overwritten.
	- poll-stats-file: The name of the file where the results ( number of votes per party ) are recorded.  No such file need already exist, and if one exists, it is overwritten.
	- serverName: The name of the server to connect to.
	- inputFile: A file containing votes. A line corresponds to one vote and has the format:
		<name> (space) <surname> (space) <party_name> (newline character)

For example, if this command is executed:
	 poller 5634 8 16 pollLog.txt pollStats.txt
then the server will listen to port 5634, it will create 8 worker-threads,, the buffer will hold up to 16 connections, each vote will be recorded to pollLog.txt, and the results in pollStats.txt

Then, if the above command is run in a server named "example.server.com", then
	pollSwayer example.server.com 5634 <inputFile> must be run (on a different terminal, of course).


## Testing:
To test the accuracy of the server-client interface, three bash scripts were made to automate
	1. The creation of different input files (create_input.sh)
	2. The expected (proper) results of said input files (tallyVotes.sh)
	3. The accumulated results from the log-file (processLogFile.sh)
Usage:
	- make server
	- (change terminal)
	- make run-client-test
	- (change back to server terminal)
	- CTRL + C
	- test-results

To test with the normal (standard, non-changing) input, run:
	- make server
	- (change terminal)
	- make client
	- (change back to server terminal)
	- CTRL + C
	- normal-results

The last rule's (test-results / normal-results) last command (diff ... ) must indicate no difference between the two files.
	


## Implementation details:
The server merely creates the master thread, and waits for it to finish. That happens when a SIGINT (CTRL + C) signal is propagated to the thread. The master thread initially creates the worker threads. Then, while active, it listens on the specified port number, accepts connections and stores them in the buffer, which is implemented as a queue. Each worker thread extracts a connection from the buffer and handles it, communicating with the client and recording the vote, if it is valid.
When the master thread is interrupted by a SIGINT signal, it sends (broadcasts) that information to all worker threads (through pthread_cond_broadcast), waits for them to finish, frees unnecessary memory, creates the stats file, and exits.

##Folder structure:
	- include: All header files for both server and client
	- server: server source files
	- client: client source file
	- shared: structures that both the server and the client utilize, namely TheadArray and Vote.
	- scripts: Bash scripts for testing
All binary and text files are saved at the top level. All object files (.o) are created and stored in the same folder as the corresponding source file.
		
	

	
					
