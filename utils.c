#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include "utils.h"


struct msg prepare_message(int i_Index, float i_Temperature)
{
    struct msg message; 

    message.Index = i_Index; 
    message.T = i_Temperature; 

    return message;
}


