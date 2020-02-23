#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <sys/socket.h> //functionalities and APIs used for sockets
#include <sys/types.h> 
#include <arpa/inet.h>
#include <netinet/in.h> //structures to store addr info
#include "packet_ftp.h"
#include <netdb.h>

#define IP_PROTOCOL 0 
#define MAX 500

int main(int argc, char const * argv[]){
    

    //check format of arguments given
    if (argc != 4)
    {
        fprintf(stderr, "Invalid use of arguments. -> ./deliver <server address> <server port number>\n");
        exit(1);
    } 
    //take the server addr and port number from arguments
    // int serv_addr = atoi(argv[1]); ///////////NEED TO FIX
    int serv_PORT_NO = atoi(argv[2]);

    char data[MAX];
    // printf("Input file name follwing the format: ftp <file name>\n");
    // scanf("%[^\n]s", data);
    //printf("1 %s\n", data);
    //check if input includes ftp
    int ok_to_send = 0; 
    char fname[32];
    // char* ftp = strtok(data," ");
    // strtok(data," ");
    // char* fname = strtok(NULL," ");
    strcpy(fname, argv[3]);
    //printf("2 %s\n", data);
    //printf("3 %s\n", fname);
    if (( access( fname, F_OK ) != -1 )){
        ok_to_send = 1;
    }
    else{
        printf("File doesn't exist.");
        exit(1);
    }

    if(ok_to_send == 1){
        // create a socket
        int network_socket;
      
        char client_message[256] = "ftp";

        network_socket = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL); //domain (internet), tcp(sock_stream)/udp, protocol (0 default)
        if (network_socket < 0) 
            printf("\nfile descriptor not received!!\n"); 
        else
            printf("\nfile descriptor %d received\n", network_socket); 

        //specify an address for the socket
        struct sockaddr_in server_address;
        memset(&server_address, 0, sizeof(server_address)); 

        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(serv_PORT_NO);
        // server_address.sin_addr.s_addr = serv_addr; //?
        if (inet_aton(argv[1], &server_address.sin_addr) == 0){
            char ip[100];
            struct addrinfo first, *servinfo, *p;
            struct sockaddr_in *h;
            int rv;

            memset(&first, 0, sizeof first);
            first.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
            first.ai_socktype = SOCK_STREAM;

            if ( (rv = getaddrinfo( argv[1] , "http" , &first , &servinfo)) != 0) 
                {
                    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
                    return 1;
                }
                for(p = servinfo; p != NULL; p = p->ai_next) 
                {
                    h = (struct sockaddr_in *) p->ai_addr;
                    strcpy(ip , inet_ntoa( h->sin_addr ) );
                }
                if (inet_aton(ip, &server_address.sin_addr)==0){
                    fprintf(stderr, "error with inet_aton, please try again\n");
                    exit(1);
                }




            // fprintf(stderr, "error with inet_aton, please try again\n");
            // exit(1);
        }


        
     
        //client connecting to server - NOT NEEDED IN UDP
        //int connection_status = connect(network_socket, (struct sockaddr *)&server_address, sizeof(server_address));
        //if(connection_status == -1){
        //    printf("There was a problem making a connection to the remote server.\n");
        //}

        // printf("client sending...\n");
        //send the ftp message
        ////send(network_socket, client_message, sizeof(client_message),0);
        //receive message
        char server_response[256];
        
        ////recv(network_socket, &server_response, sizeof(server_response),0);

        // sendto(network_socket, (const char *)client_message, sizeof(client_message), 0, (const struct sockaddr *) &server_address,  sizeof(server_address));
        sendto(network_socket, "ftp", strlen("ftp"), 0, (const struct sockaddr *)&server_address, sizeof(server_address));
        // printf("Hello message sent.\n"); 
        
        // printf("client receiving from server...\n");
        int n;
        socklen_t len;
        n = recvfrom(network_socket, (char *)server_response, 500, 0, (struct sockaddr *) &server_address, &len); 

        server_response[n] = '\0'; 
        printf("server response: %s\n", server_response);
        if(strcmp(server_response,"yes")==0){
            printf("A file transfer can start.\n");
        }
        else{
            printf("File transfer can't start.\n");
            exit(1);
        }

        file_transfer(network_socket, server_address,fname);
            //closing the socket
            close(network_socket);
    }
    return 0;
}



