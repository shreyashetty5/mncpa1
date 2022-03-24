/**
* @server
* @author  Swetank Kumar Saha <swetankk@buffalo.edu>, Shivang Aggarwal <shivanga@buffalo.edu>
* @version 1.0
*
* @section LICENSE
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details at
* http://www.gnu.org/copyleft/gpl.html
*
* @section DESCRIPTION
*
* This file contains the server init and main while loop for tha application.
* Uses the select() API to multiplex between network I/O and STDIN.
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>

#define BACKLOG 5
#define STDIN 0
#define TRUE 1
#define CMD_SIZE 100
#define BUFFER_SIZE 512
#define MSG_SIZE 512

#include "../include/global.h"
#include "../include/logger.h"
#include "../include/server.h"

int isValidIP(char *ip_str);
int isInServerList(char *IPaddress, clientList **head);
void displayBlockedList(char ip[100], clientList **list);
void sendToIP(char ipaddress[256], clientList **list, char messageToSend[256]);
void addToServerList(char host[200], int clientlistenport, int s, clientList **list);
void displayPeerList(clientList **list);
const char *inet_ntop(int af, const void *src,
                      char *dst, socklen_t size);

void sendServerList(int socket, clientList **list);
void printIP(char **argv);
/**
* main function
*
* @param  argc Number of arguments
* @param  argv The argument list
* @return 0 EXIT_SUCCESS
*/
int mainServer(int port, int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage:%s [port]\n", argv[0]);
        exit(-1);
    }

    int server_socket, head_socket, selret, sock_index, fdaccept = 0, caddr_len;
    struct sockaddr_in server_addr, client_addr;
    fd_set master_list, watch_list;
    char *ubitname = "sshetty5";
    clientList *myServerList = NULL;

    /* Set up hints structure */
    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    /* Fill up address structures */
    // if (getaddrinfo(NULL, argv[1], &hints, &res) != 0)
    //     perror("getaddrinfo failed");

    /* Socket */
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
        perror("Cannot create socket");

    /* Bind */
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        perror("Bind failed");

    //freeaddrinfo(res);

    /* Listen */
    if (listen(server_socket, BACKLOG) < 0)
        perror("Unable to listen on port");

    /* ---------------------------------------------------------------------------- */

    /* Zero select FD sets */
    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);

    /* Register the listening socket */
    FD_SET(server_socket, &master_list);
    /* Register STDIN */
    FD_SET(STDIN, &master_list);

    head_socket = server_socket;

    while (TRUE)
    {
        memcpy(&watch_list, &master_list, sizeof(master_list));

        printf("\n[PA1-Server@CSE489/589]$ ");
        fflush(stdout);

        /* select() system call. This will BLOCK */
        selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
        if (selret < 0)
            perror("select failed.");

        /* Check if we have sockets/STDIN to process */
        if (selret > 0)
        {
            /* Loop through socket descriptors to check which ones are ready */
            for (sock_index = 0; sock_index <= head_socket; sock_index += 1)
            {

                if (FD_ISSET(sock_index, &watch_list))
                {

                    /* Check if new command on STDIN */
                    if (sock_index == STDIN)
                    {
                        char *cmd = (char *)malloc(sizeof(char) * CMD_SIZE);

                        memset(cmd, '\0', CMD_SIZE);
                        if (fgets(cmd, CMD_SIZE - 1, stdin) == NULL) //Mind the newline character that will be written to cmd
                            exit(-1);

                        if (cmd[strlen(cmd) - 1] == '\n')
                        {
                            cmd[strlen(cmd) - 1] = '\0';
                        }

                        printf("\nI got: %s\n", cmd);

                        //Process PA1 commands here ...
                        char *token = strtok(cmd, " ");
                        int argc = 0;
                        char *argv[1000];
                        //parse cmd and args from input
                        while (token)
                        {
                            argv[argc] = malloc(strlen(token) + 1);
                            strcpy(argv[argc], token);
                            argc += 1;
                            token = strtok(NULL, " ");
                        }

                        if (strcmp(argv[0], "AUTHOR") == 0)
                        {
                            cse4589_print_and_log("[%s:SUCCESS]\n", argv[0]);
                            cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", ubitname);
                            cse4589_print_and_log("[%s:END]\n", argv[0]);
                        }
                        else if (strcmp(argv[0], "IP") == 0)
                        {
                            cse4589_print_and_log("[%s:SUCCESS]\n", argv[0]);
                            printIP(**argv);
                            cse4589_print_and_log("[%s:END]\n", argv[0]);
                        }
                        else if (strcmp(argv[0], "PORT") == 0)
                        {
                            cse4589_print_and_log("[%s:SUCCESS]\n", argv[0]);
                            cse4589_print_and_log("PORT:%d\n", port);
                            cse4589_print_and_log("[%s:END]\n", argv[0]);
                        }

                        free(cmd);
                    }
                    /* Check if new client is requesting connection */
                    else if (sock_index == server_socket)
                    {
                        caddr_len = sizeof(client_addr);
                        fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);
                        if (fdaccept < 0)
                            perror("Accept failed.");

                        printf("\nRemote Host connected!\n");

                        char host[1024];
                        char service[20];
                        // pretend sa is full of good information about the host and port...

                        getnameinfo(&client_addr, sizeof client_addr, host, sizeof host, service, sizeof service, 0);
                        char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
                        memset(buffer, '\0', BUFFER_SIZE);

                        if (recv(fdaccept, buffer, BUFFER_SIZE, 0) >= 0)
                        {
                            //printf("Client sent me listen port %s \n", buffer);
                        }
                        int clientListenPort = atoi(buffer);
                        addToServerList(host, clientListenPort, fdaccept, &myServerList);
                        sendServerList(fdaccept, &myServerList);

                        /* Add to watched socket list */
                        FD_SET(fdaccept, &master_list);
                        if (fdaccept > head_socket)
                            head_socket = fdaccept;
                    }
                    /* Read from existing clients */
                    else
                    {
                        /* Initialize buffer to receieve response */
                        char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
                        memset(buffer, '\0', BUFFER_SIZE);

                        if (recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0)
                        {
                            close(sock_index);
                            printf("Remote Host terminated connection!\n");

                            /* Remove from watched list */
                            FD_CLR(sock_index, &master_list);
                        }
                        else
                        {
                            //Process incoming data from existing clients here ...

                            printf("\nClient sent me: %s\n", buffer);
                            printf("ECHOing it back to the remote host ... ");
                            if (send(fdaccept, buffer, strlen(buffer), 0) == strlen(buffer))
                                printf("Done!\n");
                            fflush(stdout);
                        }

                        free(buffer);
                    }
                }
            }
        }
    }

    return 0;
}

void printIP(char **argv)
{
    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];
    char hostname[128];
    gethostname(hostname, sizeof hostname);
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0)
    {
        //fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        perror("getaddrinfo failed");
        cse4589_print_and_log("[%s:ERROR]\n", argv[0]);
        cse4589_print_and_log("[%s:END]\n", argv[0]);


    }

    for (p = res; p != NULL; p = p->ai_next)
    {
        void *addr;
        char *ipver;

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (p->ai_family == AF_INET)
        { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        }
        else
        { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // convert the IP to a string and print it:
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        cse4589_print_and_log("IP:%s\n", ipstr);
    }

    freeaddrinfo(res); // free the linked list
}