#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define FAIL -1           // define a friendly flag
#define BUF_SIZE 1 << 10  // client I/O buffer size

int socket_exit(int sockfd, int flag) {
  // defines the exit method with releasing the socket
  close(sockfd);  // closes the socket
  // flag is 0: normal exit(return 0); 1: abnormal exit(return 1)
  return flag;
}

// Plz add the -C99 flag to compile
int main(int argc, char *argv[]) {
  // read the port from cli command
  short unsigned int server_port;
  if (argc < 2 || sscanf(argv[1], "%i", &server_port) != 1) {
    printf("command line param should be the listening port\n");
    return 1;
  }

  int listen_sockfd;  // listening socket
  // get socket, IPv4 & TCP
  if ((listen_sockfd = socket(PF_INET, SOCK_STREAM, 0)) == FAIL) {
    perror("getting socket from the OS");  // describe the error
    return socket_exit(listen_sockfd, 1);  //
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
    return socket_exit(listen_sockfd, 1);
  }

  // listening to the connection request with queue length: 10
  if (listen(listen_sockfd, 5) == FAIL) {
    perror("listening on port");
    return socket_exit(listen_sockfd, 1);
  }

	int conn_sockfd; // connection socket
	unsigned int addr_len = sizeof(struct sockaddr_in); // address length
	while ((conn_sockfd = accept(listen_sockfd, (struct sockaddr *)&local_addr,
                              &addr_len)) != FAIL) {
    // get the connection socket with accept() when someone connects
  }
  printf("listening on :%i\n", server_port);

  struct sockaddr_in remote_addr;  // remote client addr
  char buffer[BUF_SIZE];           // server I/O buffer
  long len;                        // the length of bytes read from socket

  // processing loop
  while ((len = recvfrom(listen_sockfd, buffer, BUF_SIZE, 0,
                         (struct sockaddr *)&remote_addr, &addr_len)) != FAIL) {
    // if could read from that socket
    // show the remote addr
    printf("\nreceived packet from %s:\n", inet_ntoa(remote_addr.sin_addr));
    buffer[len] = '\0';  // ensure the string ends
    printf("contents: %s\n", buffer);

    if (strncmp(buffer, "exit", BUF_SIZE) == 0) {  // detect 'exit'
      printf("connection closed by client\n");
      return socket_exit(listen_sockfd, 0);
    }

    printf("typing your reply: ");  // let user input the reply
    if (scanf("%s", buffer) &&
        sendto(listen_sockfd, buffer, strlen(buffer) + 1, 0,
               (struct sockaddr *)&remote_addr, addr_len) == FAIL) {
      // waiting for user input & check sendto status
      // reply to server addr, if encounter an error:
      perror("replying to the client");
      return socket_exit(listen_sockfd, 1);
   }
  }

  sin_size = sizeof(struct sockaddr_in);

  /*等待客户端连接请求到达*/
  
  printf("accept client %s\n", inet_ntoa(remote_addr.sin_addr));
  len = send(client_sockfd, "Welcome to my server\n", 21, 0);  //发送欢迎信息

  /*接收客户端的数据并将其发送给客户端--recv返回接收到的字节数，send返回发送的字节数*/
  while ((len = recv(client_sockfd, buf, BUFSIZ, 0)) > 0) {
    buf[len] = '\0';
    printf("%s\n", buf);
    if (send(client_sockfd, buf, len, 0) < 0) {
      perror("write");
      return 1;
    }
  }
  close(client_sockfd);
  close(listen_sockfd);
  return 0;
}