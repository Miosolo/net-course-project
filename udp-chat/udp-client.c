#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define FAIL -1           // define a friendly flag
#define BUF_SIZE 1 << 10  // client I/O buffer size

// Plz add the -C99 flag to compile
int main(int argc, char *argv[]) {
	char *server_addr = argv[1];     // server address
	short unsigned int server_port;  // server port, 16 bits
	if (argc < 3 || sscanf(argv[2], "%i", &server_port) != 1) {
		// check the input params' amount and format
		printf("command line params should be (IP, port)\n");
		return 1;  // abnormal exit
	}

	// init socket id
	int client_sockfd;
	if ((client_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == FAIL) {
		// apply for a UDP IPv4 socket
		perror("getting socket form the OS");  // explain the latest errorno
		return 1;
	}
	printf(
			"socket established\ntype anything to send to the server, and type "
			"'exit' to stop\n");

	// init server addr
	struct sockaddr_in remote_addr = {0};                  // set to 0
	remote_addr.sin_family = AF_INET;                      // IPv4
	remote_addr.sin_addr.s_addr = inet_addr(server_addr);  // server IP
	remote_addr.sin_port = htons(server_port);             // server port

	char buffer[BUF_SIZE];  // the buffer used as pipe of stdin <=> client <=>
													// server
	// starting connection loop
	while (scanf("%s", buffer)) {
		// scanf returns the valid param amount, so scanf()==1 <=> valid input
		long len = 0;  // the length of success transfer
		if ((len = sendto(client_sockfd, buffer, strlen(buffer) + 1, 0,
											(struct sockaddr *)&remote_addr,
											sizeof(struct sockaddr))) == FAIL) {
			// send data to the address & port specified
			perror("sending data to server");
			return 1;
		}

		if (strncmp(buffer, "exit", BUF_SIZE) == 0) {
			// if inputed "exit": send to server, then close it self
			printf("bye\n");
			close(client_sockfd);
			return 0;
		}

		unsigned int addr_len = sizeof(struct sockaddr_in);
		if ((len = recvfrom(client_sockfd, buffer, BUF_SIZE - 1, 0,
												(struct sockaddr *)&remote_addr,
												(socklen_t *)&addr_len)) == FAIL) {
			// receive data from the address & port specified
			perror("receiving from server");
			return 1;
		}

		if (len == 0) {
			// received 0 length <=> server closed the conneciton
			printf("conection closed by server");
			close(client_sockfd);  // close the socket
			return 0;
		}

		printf("Received from server: %s\n", buffer);
	}
}