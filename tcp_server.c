#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <math.h>  // Added
#include "utils.h"


#define numExternals 4     // Number of external processes 


int * establishConnectionsFromExternalProcesses()
{

    // This socket is used by the server (i.e., Central process) to listen for 
    // connections from the External process. 
    int socket_desc;
     
    static int client_socket[numExternals]; 

    unsigned int client_size;
    struct sockaddr_in server_addr, client_addr;

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        exit(0);
    }
    printf("Socket created successfully\n");
    
    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        printf("Couldn't bind to the port\n");
        exit(0);
    }
    printf("Done with binding\n");
    
    // Listen for clients:
    if(listen(socket_desc, 1) < 0){
        printf("Error while listening\n");
        exit(0);
    }
    printf("\n\nListening for incoming connections.....\n\n");

    printf("-------------------- Initial connections ---------------------------------\n");

    //========================================================
    //  Connections from externals 
    //========================================================
    int externalCount = 0; 
    while (externalCount < numExternals){

        // Accept an incoming connection:
        client_socket[externalCount] = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);
        
        if (client_socket[externalCount] < 0){
            printf("Can't accept\n");
            exit(0);
        }

        printf("One external process connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        externalCount++; 
    }
    printf("--------------------------------------------------------------------------\n");
    printf("All four external processes are now connected\n");
    printf("--------------------------------------------------------------------------\n\n");

    return client_socket;   // Pointer to the array of file descriptors of client sockets 
}



int main(void)
{
    struct msg messageFromClient;
    int *client_socket = establishConnectionsFromExternalProcesses();
    float temperature[numExternals];
    float centralTemp = 100;  // Initial central temperature (can be passed as argument)
    float prevCentralTemp;
    float threshold = 0.01;
    bool stable = false;

    while (!stable) {
        float tempSum = 0;

        // Receive temperatures from external processes:
        for (int i = 0; i < numExternals; i++) {
            if (recv(client_socket[i], (void *)&messageFromClient, sizeof(messageFromClient), 0) < 0) {
                printf("Error receiving message\n");
                return -1;
            }
            temperature[i] = messageFromClient.T;
            tempSum += temperature[i];
            printf("Received temp from external process %d: %f\n", i, temperature[i]);
        }

        // Calculate new central temperature:
        prevCentralTemp = centralTemp;
        centralTemp = (2 * centralTemp + tempSum) / 6;
        printf("Updated central temperature: %f\n", centralTemp);

        // Send updated central temperature to external processes:
        struct msg updated_msg;
        updated_msg.T = centralTemp;
        updated_msg.Index = 0;

        for (int i = 0; i < numExternals; i++) {
            if (send(client_socket[i], (void *)&updated_msg, sizeof(updated_msg), 0) < 0) {
                printf("Error sending message\n");
                return -1;
            }
        }

        // Check if system has stabilized:
        if (fabs(centralTemp - prevCentralTemp) < threshold) {
            stable = true;
            printf("System stabilized at temperature: %f\n", centralTemp);

            // Notify external processes that the system is stable:
            updated_msg.Index = -1;  // Signal to terminate
            for (int i = 0; i < numExternals; i++) {
                if (send(client_socket[i], (void *)&updated_msg, sizeof(updated_msg), 0) < 0) {
                    printf("Error sending stabilization signal\n");
                    return -1;
                }
            }
        }
    }

    // Close sockets
    for (int i = 0; i < numExternals; i++) {
        close(client_socket[i]);
    }

    return 0;
}
