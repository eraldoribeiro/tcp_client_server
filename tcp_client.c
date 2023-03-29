#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include "utils.h"


int main (int argc, char *argv[])
{
    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[100], client_message[100];

    struct msg the_message; 
    
    // Command-line input arguments (user provided)
    int externalIndex = atoi(argv[1]); 
    float initialTemperature = atof(argv[2]); 

    
    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    
    if(socket_desc < 0){
        printf("Unable to create socket\n");
        return -1;
    }
    
    printf("Socket created successfully\n");
    
    // Set port and IP the same as server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        

    // Send connection request to server:
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected with server successfully\n");
    printf("--------------------------------------------------------\n\n");
       
    // Package to the sent to server 
    the_message = prepare_message(externalIndex, initialTemperature); 

    // Send the message to server:
    if(send(socket_desc, (const void *)&the_message, sizeof(the_message), 0) < 0){
        printf("Unable to send message\n");
        return -1;
    }
 

    // Receive the server's response:
    if(recv(socket_desc, (void *)&the_message, sizeof(the_message), 0) < 0){
        printf("Error while receiving server's msg\n");
        return -1;
    }
    
    printf("--------------------------------------------------------\n");
    printf("Updated temperature sent by the Central process = %f\n", the_message.T);
    
    // Close the socket:
    close(socket_desc);
    
    return 0;
}
