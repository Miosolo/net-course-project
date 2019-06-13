#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define FAIL -1 // define a friendly flag
#define BUF_SIZE 1 << 10

// Plz add the -C99 flag to compile
int main(int argc, char *argv[]) {
  char server_addr[20] = "127.0.0.1"; // server address
  int server_port = 12000; // server port
  /*/if (argc < 2 || scanf("%s%s", &server_addr, &server_port) != 2) {// check the input params' amount and format
    printf("command line params should be (address, port)\n");
    return 1; // abnormal exit
  }*/

  // init socket id
  int client_sockfd; 
  if((client_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == FAIL) {// apply for a UDP IPv4 socket
    perror("getting socket form the OS"); // explain the latest errorno
		return 1;
  }
  printf("socket established\ntype anything to send to the server, and type 'exit' to stop\n");

  // init server addr
  struct sockaddr_in remote_addr = {0}; // set to 0
  // memset(&remote_addr, 0, sizeof(remote_addr));
	remote_addr.sin_family = AF_INET; // UDP
	remote_addr.sin_addr.s_addr = inet_addr(server_addr); // server IP
	remote_addr.sin_port = htons(server_port); // server port
  
  char buffer[BUF_SIZE];  // the buffer used as pipe of stdin <=> client <=> server
  // starting connection loop
  while(scanf("%s", &buffer)) { // scanf returns the valid param amount, so scanf()==1 <=> valid input
    if (strcmp(buffer, "exit") == 0) { // input "exit"
      printf("bye\n");
      return 0;
    }

    long len = 0; // the length of success transfer
	  if (len = sendto(client_sockfd, buffer, strlen(buffer), 0,
      (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == FAIL) { // send data to the address & port specified
        perror("sending data to server");
        return 1;
      }
    
    int addr_len = sizeof(struct sockaddr_in);
    if (len = recvfrom(client_sockfd, buffer, BUF_SIZE-1, 0, 
      (struct sockaddr *)&remote_addr, (socklen_t *)&addr_len) == FAIL) { // receive data from the address & port specified
        perror("receiving from server"); 
        return 1;
      }

    printf("Received from server: %s\n", buffer);
  }

	close(client_sockfd);
	return 0;
}