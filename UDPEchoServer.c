#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include "Garden.h"

#define ECHOMAX 255     /* Longest string to echo */

void DieWithError(char *errorMessage);  /* External error handling function */

int main(int argc, char *argv[]) {
    struct Garden garden, *gardenptr;
    garden.field = {
            {0, 0, 0, 0, -1, 0, 0, 0, 0, 0},
            {-1, 0, 0, 0, 0, 0, 0, -1, 0, 0},
            {0, 0, -1, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, -1, 0, -1, 0},
            {0, -1, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, -1, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, -1, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, -1, 0, 0, 0},
            {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    garden.m = M;
    garden.n = N;
    garden.first_pos = {0, 0};
    garden.second_pos = {0, 0};
    garden.first_way = 1;
    garden.second_way = -1;

    gardenptr = &garden;

    int sock;                        /* Socket */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int cliAddrLen;         /* Length of incoming message */
    char echoBuffer[ECHOMAX];        /* Buffer for echo string */
    unsigned short echoServPort;     /* Server port */
    int recvMsgSize;                 /* Size of received message */

    if (argc != 2) {       /* Test for correct number of parameters */
        fprintf(stderr,"Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);  /* First arg:  local port */

    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");

    for (;;) {/* Run forever */
        /* Set the size of the in-out parameter */
        cliAddrLen = sizeof(echoClntAddr);

        /* Block until receive message from a client */
        if ((recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0,
            (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
            DieWithError("recvfrom() failed");

        printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

        int workPlace[2] = {-1, -1};
        if (echoBuffer[0] == '1') {
            if (gardenptr->first_pos[1] + gardenptr->first_way < 0 ||
                gardenptr->first_pos[1] + gardenptr->first_way == gardenptr->n) {
                ++gardenptr->first_pos[0];
                gardenptr->first_way *= -1;
                if (gardenptr->first_pos[0] == gardenptr->m) {
                    echoBuffer[0] = '\0';
                    recvMsgSize = 1;
                }
            } else {
                gardenptr->first_pos[1] += gardenptr->first_way;
            }
            workPlace[0] = gardenptr->first_pos[0];
            workPlace[1] = gardenptr->first_pos[1];
        }

        if (echoBuffer[0] == '2') {
            if (gardenptr->second_pos[0] + gardenptr->second_way < 0 ||
                gardenptr->second_pos[0] + gardenptr->second_way == gardenptr->m) {
                --gardenptr->second_pos[0];
                gardenptr->second_way *= -1;
                if (gardenptr->second_pos[0] == gardenptr->-1) {
                    echoBuffer[0] = '\0';
                    recvMsgSize = 1;
                }
            } else {
                gardenptr->second_pos[1] += gardenptr->second_way;
            }
            workPlace[0] = gardenptr->second_pos[0];
            workPlace[1] = gardenptr->second_pos[1];
        }

        if (echoBuffer[0] == '3') {
            printf("Garden is:\n");
            for (size_t i = 0; i < gardenptr->m; ++i) {
                for (size_t j = 0; j < gardenptr->n; ++j) {
                    printf("%d\t", gardenptr->field[i][j]);
                }
                printf("\n");
            }
            printf("\n");
            sleep(1);
        }

        if (workPlace[0] != -1) {
            sleep(1);
            if (gardenptr->field[workPlace[0]][workPlace[1]] == 0) {
                sleep(atoi(&echoBuffer[2]));
                gardenptr->field[workPlace[0]][workPlace[1]] = echoBuffer[0] - '0';
            }
            sprintf(echoBuffer, "Worker %c work in place (%d, %d)", echoBuffer[0], workPlace[0], workPlace[1]);
            printf("Worker %c work in place (%d, %d)", echoBuffer[0], workPlace[0], workPlace[1]);
        }

        /* Send received datagram back to the client */
        if (sendto(sock, echoBuffer, recvMsgSize, 0,
             (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != recvMsgSize)
            DieWithError("sendto() sent a different number of bytes than expected");
    }
    /* NOT REACHED */
}
