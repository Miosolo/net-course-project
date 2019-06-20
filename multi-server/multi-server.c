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
#define Q 10              // max online client amount

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
  FD_SET(listen_sockfd, &conn_set);  // register the listening socket
  int max_fd = listen_sockfd;        // maximum file describer

  // a simple connection queue, with the listenr at the head
  int connfd_queue[Q] = {listen_sockfd, 0};
  int empty_slot = 1;  // trace the first empty index in the queue

  while (select(max_fd + 1, &conn_set, NULL, NULL, NULL) != FAIL) {
    // go through the connection queue
    for (int i = 0; i < empty_slot; i++) {
      int thisfd = connfd_queue[i];
      if (thisfd == 0) {
        continue;  // pass the empty slot
      }

      if (!FD_ISSET(thisfd, &conn_set)) {
        FD_SET(thisfd, &conn_set);  // keep alive
        continue;
      }

      // below: catched changes

      // handle the connecting request at listening port
      if (thisfd == listen_sockfd) {
        // is the conncetion socket <=> new client inbound
        // check the max available slot index
        if (empty_slot != Q) {
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
          connfd_queue[empty_slot++] = conn;  // add this socket to the queue
          // NOTICE: Delayed FD_SET(conn, &conn_set)
        } else {
          // if the queue is full, delay accpeting the connection
          FD_SET(listen_sockfd, &conn_set);
          // reactivate the listening port, waiting for someone's exit
        }
      } else {     // chosed a connection socket
        long len;  // received msg length
        if ((len = recv(thisfd, buffer, BUF_SIZE, 0)) == FAIL) {
          perror("receiving");
          return 1;
        }

        buffer[len] = 0;                          // ensure the string ends
        if (strncmp(buffer, "exit", len) == 0) {  // detected 'exit'
          printf("connection #%d was closed by client\n", thisfd);
          if (i != empty_slot - 1) {
            // copy the last connfd there & decrease the empty_slot
            // i--, so in the next round connfd[i] will be checked again
            // also, the FD flag is kept
            connfd_queue[i] = connfd_queue[empty_slot - 1];
            i--, empty_slot--;
          } else {
            // if they overlaps <-> the last one
            empty_slot--;
          }
          FD_CLR(thisfd, &conn_set);
          close(thisfd);  // close this connection
          continue;       // go to the next round
        }

        printf("received from #%d: %s\n", thisfd, buffer);
        printf("type your response: ");
        scanf("%s", buffer);  // read the stdin input

        // check the server user's input
        if (strncmp(buffer, "exit", len) == 0) {  // detect 'exit'
          close(listen_sockfd);
          for (int i = 0; i < Q; i++) {
            if (connfd_queue[i] != 0) {
              close(connfd_queue[i]);
            }  // release all the existing sockets
          }
          printf("all connections were closed by server\n");
          return 0;
        }

        // including the ending \0
        if (send(thisfd, buffer, strlen(buffer) + 1, 0) == FAIL) {
          // waiting for user input & check send status
          // reply to server addr, if encounter an error:
          perror("replying");
          return 1;
        }

        FD_ISSET(thisfd, &conn_set);  // reactivate monitoring this so
      }                               // end of else
    }                                 // end of for
  }                                   // end of while

  // if select fails, jumps here
  perror("selecting");
  return 1;
}
