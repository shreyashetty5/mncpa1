/**
* @client
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
* This file contains the client.
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <ctype.h>

#include "../include/global.h"
#include "../include/logger.h"
#include "../include/server.h"

#define TRUE 1
#define MSG_SIZE 512
#define BUFFER_SIZE 256
#define STDIN 0

int loginstatus = 0;

int isValidPort(char *port);
void changeBlockStatus(int block, char *IPaddress, peerSideList **head);
int isAlreadyBlocked(char *IPaddress, peerSideList **head);
void freeMyList(peerSideList **head_ref);
int connect_to_host(char *server_ip, int port, int server_port);
void addToClientList(char buffer[256], peerSideList **head);
void printIP(char **argv);
int propSend(char *message, int file_des);
char *propReceive(int file_des);
int isInClientList(char IPAddress[50], peerSideList **head);
int isValidIP(char *ip_str);

/**
* main function
*
* @param  argc Number of arguments
* @param  argv The argument list
* @return 0 EXIT_SUCCESS
*/
int mainClient(int port, int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage:%s [ip] [port]\n", argv[0]);
        exit(-1);
    }

    // int server;
    // server = connect_to_host(argv[1], argv[2]);

    // while (TRUE)
    // {
    //     printf("\n[PA1-Client@CSE489/589]$ ");
    //     fflush(stdout);

    //     char *msg = (char *)malloc(sizeof(char) * MSG_SIZE);
    //     memset(msg, '\0', MSG_SIZE);
    //     if (fgets(msg, MSG_SIZE - 1, stdin) == NULL) //Mind the newline character that will be written to msg
    //         exit(-1);

    //     printf("I got: %s(size:%d chars)", msg, strlen(msg));

    //     printf("\nSENDing it to the remote server ... ");
    //     if (send(server, msg, strlen(msg), 0) == strlen(msg))
    //         printf("Done!\n");
    //     fflush(stdout);

    //     /* Initialize buffer to receieve response */
    //     char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
    //     memset(buffer, '\0', BUFFER_SIZE);

    //     if (recv(server, buffer, BUFFER_SIZE, 0) >= 0)
    //     {
    //         printf("Server responded: %s", buffer);
    //         fflush(stdout);
    //     }
    // }

    char *ubitname = "sshetty5";
    int server;
    peerSideList *myClientList = NULL;
    //server = connect_to_host(argv[1], atoi(argv[2]));
    //server = connect_to_host("localhost", port);
    int head_socket, selret, sock_index, fdaccept = 0, caddr_len;
    fd_set master_list, watch_list;
    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);
    FD_SET(STDIN, &master_list);
    head_socket = STDIN;
    while (TRUE)
    {
        memcpy(&watch_list, &master_list, sizeof(master_list));

        printf("\n[PA1-Client@CSE489/589]$ ");
        fflush(stdout);
        selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
        if (selret < 0)
            perror("select failed.");

        if (selret > 0)
        {
            for (sock_index = 0; sock_index <= head_socket; sock_index += 1)
            {

                if (FD_ISSET(sock_index, &watch_list))
                {
                    if (sock_index == STDIN)
                    {
                        char *msg = (char *)malloc(sizeof(char) * MSG_SIZE);
                        memset(msg, '\0', MSG_SIZE);
                        if (fgets(msg, MSG_SIZE - 1, stdin) == NULL) //Mind the newline character that will be written to msg
                            exit(-1);
                        if (msg[strlen(msg) - 1] == '\n')
                        {
                            msg[strlen(msg) - 1] = '\0';
                        }
                        int l = strlen(msg);
                        printf("I got: %s (size:%d chars) \n", msg, l);
                        char *messageToSend = (char *)malloc(sizeof(char) * MSG_SIZE);
                        memset(messageToSend, '\0', MSG_SIZE);
                        strncpy(messageToSend, msg, strlen(msg));
                        printf("message at client side: %s \n", messageToSend);
                        if (strlen(msg) == 0)
                        {
                            continue;
                        }

                        char *token = strtok(msg, " ");
                        int argc = 0;
                        char *argv[1000];
                        memset(argv, 0, sizeof(argv));
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
                    }
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
                            //Process incoming data from existing server here ...
                            char delimiter[] = " ";
                            char *firstWord, *secondWord, *remainder, *context;
                            printf("\nServer sent me: %s\n", buffer);
                            firstWord = strtok_r(buffer, delimiter, &context);
                            secondWord = strtok_r(NULL, delimiter, &context);
                            remainder = context;
                            printf("first %s\n", firstWord);
                            printf("second %s\n", secondWord);
                            printf("remainder %s\n", remainder);
                            cse4589_print_and_log("[RECEIVED:SUCCESS]\n");
                            cse4589_print_and_log("msg from:%s\n[msg]:%s\n", secondWord, remainder);
                            cse4589_print_and_log("[RECEIVED:END]\n");
                        }

                        free(buffer);
                    }
                }
            }
        }
    }
}

int connect_to_host(char *server_ip, int port, int server_port)
{
    // int fdsocket;
    // struct addrinfo hints, *res;

    // /* Set up hints structure */
    // memset(&hints, 0, sizeof(hints));
    // hints.ai_family = AF_INET;
    // hints.ai_socktype = SOCK_STREAM;

    // /* Fill up address structures */
    // if (getaddrinfo(server_ip, server_port, &hints, &res) != 0)
    //     perror("getaddrinfo failed");

    // /* Socket */
    // fdsocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    // if (fdsocket < 0)
    //     perror("Failed to create socket");

    // /* Connect */
    // if (connect(fdsocket, res->ai_addr, res->ai_addrlen) < 0)
    //     perror("Connect failed");

    // freeaddrinfo(res);

    // return fdsocket;

    int fdsocket, len;
    struct sockaddr_in remote_server_addr;

    fdsocket = socket(AF_INET, SOCK_STREAM, 0);
    if (fdsocket < 0)
        perror("Failed to create socket");

    bzero(&remote_server_addr, sizeof(remote_server_addr));
    remote_server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip, &remote_server_addr.sin_addr);
    remote_server_addr.sin_port = htons(server_port);

    if (connect(fdsocket, (struct sockaddr *)&remote_server_addr, sizeof(remote_server_addr)) < 0)
        perror("Connect failed");
    char portNo[100];
    memset(portNo, 0, sizeof(portNo));
    sprintf(portNo, "%d", port);

    printf("\nSENDing it to the remote server ... ");
    if (send(fdsocket, portNo, strlen(portNo), 0) == strlen(portNo))
        printf("Done!\n");
    fflush(stdout);
    // cse4589_print_and_log((char *)"[%s:SUCCESS]\n", "LOGIN");
    // cse4589_print_and_log((char *)"[%s:END]\n", "LOGIN");

    return fdsocket;
}
