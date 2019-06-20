#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define FAIL -1           // define a friendly flag
#define BUF_SIZE 1 << 10  // server I/O buffer size
#define Q 10 // max online client amount

// Plz add the -C99 flag to compile
int main(int argc, char *argv[]) {
  // read the port from cli command
  short unsigned int server_port = 12000;
  /*/if (argc < 2 || sscanf(argv[1], "%i", &server_port) != 1) {
    // check param list length & read the port num
    printf("command line param should be the listening port\n");
    return 1;
  }*/

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

  fd_set conn_set;                   // the set to trace connections
  FD_ZERO(&conn_set);                // clear
  FD_SET(listen_sockfd, &conn_set);  // register the socket
  int max_fd = listen_sockfd;        // maximum file describer

	// a simple client queue
	int connfd_queue[Q] = {0};
	int index = 0;

  while (select(max_fd + 1, &conn_set, NULL, NULL, NULL) != FAIL) {
    // select one of the connections, waiting until any fd changes
    if (FD_ISSET(listen_sockfd, &conn_set)) {
      // is the conncetion socket <=> new client inbound

      // accept the connection
      int conn_sockfd;  // connection socket
      if ((conn_sockfd = accept(listen_sockfd, (struct sockaddr *)&remote_addr,
                                &addr_len)) == FAIL) {
        // failed to accept the connection
        perror("accepting connection");
        return 1;
      }

      // show remote info
      printf("\nconnection established with %s:\n",
             inet_ntoa(remote_addr.sin_addr));

      // update the max fd index
      max_fd = conn_sockfd > max_fd ? conn_sockfd : max_fd;
      FD_SET(conn_sockfd, &conn_set);
    } else {  
			// chosed a connection socket
      long len;
      if ((len = recv(selected_fd, buffer, BUF_SIZE, 0)) == FAIL) {
        perror("receiving");
        return 1;
      }

      buffer[len] = 0;                          // ensure the string ends
      if (strncmp(buffer, "exit", len) == 0) {  // detect 'exit'
        printf("a connection was closed by client\n");
        close(selected_fd);
        FD_CLR(selected_fd, &conn_set);
        continue;
      }

      printf("received: %s\n", buffer);
      printf("type your response: ");
      scanf("%s", buffer);

      // check the admin's input
      if (strncmp(buffer, "exit", len) == 0) {  // detect 'exit'
        printf("all connections were closed by server\n");
        return 0;  // let OS recycle the sockets & ports
      }

      // including the ending \0
      if (send(selected_fd, buffer, strlen(buffer) + 1, 0) == FAIL) {
        // waiting for user input & check send status
        // reply to server addr, if encounter an error:
        perror("replying");
        return 1;
      }
    }
  }

  // if select fails, jumps here
  perror("selecting");
  return 1;
}
