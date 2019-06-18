#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define FAIL -1           // define a friendly flag
#define BUF_SIZE 1 << 10  // client I/O buffer size

// Plz add the -C99 flag to compile
int main(int argc, char *argv[]) {
	// read the port from cli command
	short unsigned int server_port;
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

	int conn_sockfd;                                     // connection socket
	struct sockaddr_in remote_addr;                      // client address
	unsigned int addr_len = sizeof(struct sockaddr_in);  // address length
	char buffer[BUF_SIZE];                               // server I/O buffer
	if ((conn_sockfd = accept(listen_sockfd, (struct sockaddr *)&remote_addr,
														&addr_len)) != FAIL) {
		// get the connection socket with accept() when someone connects
		// show the remote addr string
		printf("\nconnection established with %s:\n", inet_ntoa(remote_addr.sin_addr));

		int len;  // indicates the content length
		while ((len = recv(conn_sockfd, buffer, BUF_SIZE, 0)) != FAIL) {
			// receive from the client, if not fail, then:
			buffer[len] = 0;                          // ensure the string ends
			if (strncmp(buffer, "exit", len) == 0) {  // detect 'exit'
				printf("connection closed by client\n");
				close(conn_sockfd);
				close(listen_sockfd);
				return 0;
			}

			printf("received: %s\n", buffer);
			printf("type your response: ");
			if (scanf("%s", buffer) &&
					// including the end \0
					send(conn_sockfd, buffer, strlen(buffer) + 1, 0) == FAIL) {
				// waiting for user input & check send status
				// reply to server addr, if encounter an error:
				perror("replying");
				return 1;
			}
		}
	}
}
