
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define FAIL -1 // define a friendly flag
#define BUF_SIZE 1 << 10 // server I/O buffer size

// Plz add the -C99 flag to compile
int main(int argc, char *argv[]) {
  // read the port from cli command 
  short unsigned int server_port = 12000;
  /*if (argc < 2 || sscanf(argv[1], "%i", &server_port) != 1) {
    printf("command line param should be the listening port\n");
    return 1;
  }*/

  int server_sockfd; // server socket
  if((server_sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == FAIL) { // get socket, IPv4 & UDP  
    perror("getting socket from the OS");
    return 1;
  }

  struct sockaddr_in local_addr = {0}; // server's local address, init to zero
  local_addr.sin_family = AF_INET; // IPv4
  local_addr.sin_addr.s_addr = INADDR_ANY; // bind IP to any inbound IP
  local_addr.sin_port = htons(server_port); // port
  if (bind(server_sockfd, (struct sockaddr *)&local_addr, sizeof(struct sockaddr_in)) == FAIL) {
    // bind the socket to a local addr
    perror("binding local address");
    return 1;
  }
  printf("listening on :%i\n", server_port);

  struct sockaddr_in remote_addr; // remote client addr
  char buffer[BUF_SIZE]; // server I/O buffer
  long len; // the length of bytes read from socket
  unsigned int addr_len = sizeof(struct sockaddr_in); // socket length
  // processing loop
  while ((len = recvfrom(server_sockfd, buffer , BUF_SIZE, 0, 
    (struct sockaddr *)&remote_addr, &addr_len)) != FAIL) { // if could read from that socket
      printf("\nreceived packet from %s:\n", inet_ntoa(remote_addr.sin_addr)); // show the remote addr
      buffer[len]='\0'; // ensure the string ends
      printf("contents: %s\n", buffer);

      if (strncmp(buffer, "exit", BUF_SIZE) == 0) {// detect 'exit'
        printf("connection closed by client\n");
        return 0;
      }
      
      printf("typing your reply: "); // let user input the reply
      if (scanf("%s", buffer) && sendto(server_sockfd, buffer, BUF_SIZE, 0,
        (struct sockaddr *)&remote_addr, addr_len) == FAIL) {// waiting for user input & check sendto status
          // reply to server addr, if encounter an error:
          perror("replying to the client");
          return 1;
        }
  }
}