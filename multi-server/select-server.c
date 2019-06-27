#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define FAIL -1           // define a friendly flag
#define BUF_SIZE 1 << 10  // server I/O buffer size
#define Q 10              // max online clients

// Plz add the -C99 flag to compile
int main(int argc, char *argv[]) {
	// read the port from cli command
	short unsigned int server_port = 12000;
	if (argc < 2 || sscanf(argv[1], "%i", &server_port) != 1) {
		// check param list length & read the port num
		printf("command line param should be the listening port\n");
		return 1;
	}

	int listen_sockfd;  // listening socket
	// get socket, IPv4 & TCP
	if ((listen_sockfd = socket(PF_INET, SOCK_STREAM, 0)) == FAIL) {
		perror("getting socket from the OS");  // describe the error
		return 1;
	}

	// init the server (local) address
	struct sockaddr_in local_addr = {0};  // server's local address, init to zero
	local_addr.sin_family = AF_INET;      // IPv4
	local_addr.sin_addr.s_addr = INADDR_ANY;   // bind IP to any inbound IP
	local_addr.sin_port = htons(server_port);  // port

	// bind the socket to the listening address
	if (bind(listen_sockfd, (struct sockaddr *)&local_addr,
					 sizeof(struct sockaddr_in)) == FAIL) {
		perror("binding local address");
		return 1;
	}

	// listening to the connection request with queue length: 10
	if (listen(listen_sockfd, 5) == FAIL) {
		perror("starting listening");
		return 1;
	}
	printf("listening on :%i\n", server_port);

	struct sockaddr_in remote_addr = {0};                // client address
	unsigned int addr_len = sizeof(struct sockaddr_in);  // address length
	char buffer[BUF_SIZE];                               // server I/O buffer
	fd_set all_set;                   // the sets to trace all fds
	FD_ZERO(&all_set);                // clear
	FD_SET(listen_sockfd, &all_set);  // register the listening socket
	FD_SET(0, &all_set);              // register the stdin socket
	int max_fd = listen_sockfd;       // maximum file describer

	// a simple connection queue, with the listenr at the head
	int connfd_queue[Q] = {0};

	fd_set select_set = all_set;  // a copy of all_set to select
	for (; select(max_fd + 1, &select_set, NULL, NULL, NULL) != FAIL;
			 select_set = all_set) {
		// check stdin "exit"
		if (FD_ISSET(0, &select_set)) {
			char inbuf[5];
			fgets(inbuf, 5, stdin);  // reads the input
			if (strncmp(inbuf, "exit", 5) == 0) {
				// detected "exit", close all connections
				close(listen_sockfd);
				for (int i = 0; i < Q; i++) {
					if (connfd_queue[i] != 0) {
						// if connection socket exist, close it
						close(connfd_queue[i]);
					}
				}
				// log out
				printf("all connections are closed by server, bye\n");
				return 0;
			}
		}

		// check the listening socket
		if (FD_ISSET(listen_sockfd, &select_set)) {
			// is the conncetion socket <=> new client inbound
			// find an empty slot in queue
			int slot = Q;
			for (int i = 0; i < Q; i++) {
				if (connfd_queue[i] == 0) {
					slot = i;
					break;
				}
			}

			if (slot != Q) {
				// when queue is available, accept the connection
				int conn;  // connection socket
				if ((conn = accept(listen_sockfd, (struct sockaddr *)&remote_addr,
													 &addr_len)) == FAIL) {
					// failed to accept the connection
					perror("accepting connection");
					return 1;
				}

				// show remote info
				printf("connection #%d established with %s:\n", conn,
							 inet_ntoa(remote_addr.sin_addr));

				// update the max fd index
				max_fd = conn > max_fd ? conn : max_fd;
				connfd_queue[slot] = conn;  // add this socket to the queue
				FD_SET(conn, &all_set);     // register in connection set
			}  // else: select & handle listen_fd in the next round
		}

		// go through the connection queue
		for (int i = 0; i < Q; i++) {
			int thisfd = connfd_queue[i];

			// if the socket has an action
			if (thisfd != 0 && FD_ISSET(thisfd, &select_set)) {
				long len;  // received msg length
				if ((len = recv(thisfd, buffer, BUF_SIZE, 0)) == FAIL) {
					perror("receiving");
					return 1;
				}

				buffer[len] = 0;                          // ensure the string ends
				if (strncmp(buffer, "exit", len) == 0) {  // detected 'exit'
					printf("connection #%d was closed by client\n", thisfd);
					connfd_queue[i] = 0;  // clear the slot in connection queue
					FD_CLR(thisfd, &all_set);
					close(thisfd);  // close this connection
					continue;       // go to the next round
				}

				printf("received from #%d: %s\n", thisfd, buffer);

				// including the ending \0
				if (send(thisfd, buffer, strlen(buffer) + 1, 0) == FAIL) {
					// waiting for user input & check send status
					// reply to server addr, if encounter an error:
					perror("replying");
					return 1;
				}
			}
		}  // end of iterating connection queue
	}    // end of looping

	// if select fails, jumps here
	perror("selecting");
	return 1;
}
