/******************************************************************************
* echo_server.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server runs on a hard-coded port and simply write back anything*
*              sent to it by connected clients.  It does not support          *
*              concurrent clients.                                            *
*                                                                             *
* Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
*          Wolf Richter <wolf@cs.cmu.edu>                                     *
*                                                                             *
*******************************************************************************/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define ECHO_PORT 9999
#define BUF_SIZE 4096

int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    int sock, newfd, fdmax;
    ssize_t readret;
    socklen_t cli_size;
    struct sockaddr_in addr;
    struct sockaddr_storage remoteaddr;
    char buf[BUF_SIZE];

    fd_set master, read_fds;

    fprintf(stdout, "----- Echo Server -----\n");
    
    /* all networked programs must create a socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(ECHO_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)))
    {
        close_socket(sock);
        fprintf(stderr, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }

    if (listen(sock, 1024))
    {
        close_socket(sock);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }

    FD_SET(sock, &master);
    fdmax = sock;

    /* finally, loop waiting for input and then write it back */
    while (1)
    {
       int i = 0;
       read_fds = master;    //length = sizeof server;

       if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
          fprintf(stderr, "Select() error.\n");
          return EXIT_FAILURE;
       }

       for (i = 0; i <= fdmax; i++) {
          if (FD_ISSET(i, &read_fds)) { // got one
            if (i == sock) {
              cli_size = sizeof remoteaddr;
              if ((newfd = accept(sock, (struct sockaddr *)&remoteaddr, &cli_size)) == -1) {
                fprintf(stderr, "Error accepting connection.\n");
                } else {
                  FD_SET(newfd, &master);
                  if (newfd > fdmax) {
                    fdmax = newfd;
                  }
              }
            } else {
              readret = 0;

              if ((readret = recv(i, buf, BUF_SIZE, 0)) >= 1) {
                if (send(i, buf, readret, 0) != readret){
                     close_socket(i);
                     close_socket(sock);
                     fprintf(stderr, "Error sending to client.\n");
                     return EXIT_FAILURE;
                 }
                 memset(buf, 0, BUF_SIZE);
              } else{
                if (readret == -1){
                   close_socket(i);
                   close_socket(sock);
                   fprintf(stderr, "Error reading from client socket.\n");
                   return EXIT_FAILURE;
                }
                if (close_socket(i))
                 {
                     close_socket(sock);
                     fprintf(stderr, "Error closing client socket.\n");
                     return EXIT_FAILURE;
                 }
            }
          }
       }
       
       }

       

       
    }

    close_socket(sock);

    return EXIT_SUCCESS;
}