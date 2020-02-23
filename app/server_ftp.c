#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h> //functionalities and APIs used for sockets
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h> //structures to store addr info
#include "packet_ftp.h"
#include <stdbool.h>
#include <libgen.h>



int main(int argc, char const * argv[]){

    struct packet packet_data;

    //check format of arguments given
    if (argc != 2)
    {
        fprintf(stderr, "Invalid use of arguments. -> ./server <UDP listen port>\n");
        exit(1);
    } 
    //take the port number from arguments
    int PORT_NO = atoi(argv[1]);

    //create the server socket
    int server_socket;
    struct sockaddr_in server_address;

    server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //domain (internet), tcp(sock_stream)/udp, protocol (0 default)
    // if (server_socket < 0) 
    //     printf("\nfile descriptor not received.\n"); 
    // else
    //     printf("\nfile descriptor %d received\n", server_socket);

    //define the server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT_NO);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(server_address.sin_zero,0,sizeof(server_address.sin_zero));

    //bind specified ip address and port # to server socket
    // if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == 0) //cast struct sockaddr only
    //     printf("\nSuccessfully binded!\n"); 
    // else
    //     printf("\nBinding Failed!\n"); 

    while (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) != 0){
        server_address.sin_family = AF_INET;
        PORT_NO = PORT_NO+1;
        server_address.sin_port = htons(PORT_NO);
        server_address.sin_addr.s_addr = htonl(INADDR_ANY);
        memset(server_address.sin_zero,0,sizeof(server_address.sin_zero));

    }
    char server_tempfile[250];
    sprintf(server_tempfile, "echo %d > temp_file.txt\n", PORT_NO+1);
    system(server_tempfile);
 
    //printf("server listening..\n");
    for(;;){



        char client_response[5]={0};
        struct sockaddr_in client_addr;
        socklen_t cli;

        //printf("server receiving from client...\n");

        //int len, n; 
        //n = recvfrom(server_socket, (char *)client_response, 500, MSG_WAITALL, ( struct sockaddr *) &client_address, &len); 
        //client_response[n] = '\0'; 
    
        // recv(server_socket, &client_response, sizeof(client_response), 0);
        if (recvfrom(server_socket, client_response, 5, 0, (struct sockaddr *) &client_addr, &cli)==-1){
            fprintf(stderr,"issue with recieving an ftp response");
            exit(1);
        }
        //printf("client sent: %s\n", client_response); 
        if(strcmp(client_response,"ftp")==0){
            // strcmp(ftp_confirm, "yes");
            //printf("server confirming ftp... sending yes\n");
            //printf("comeshhere\n");
            //printf("%d, %d, %d, %s", client_addr.sin_family,client_addr.sin_port,client_addr.sin_addr.s_addr,client_addr.sin_zero);
            ///send(client_socket, &ftp_confirm, sizeof(ftp_confirm),0);
            // //printf("comeshhere");
            if ((sendto(server_socket, "yes", strlen("yes"), 0, (struct sockaddr *) &client_addr, sizeof(client_addr))) == -1) {
                fprintf(stderr, "Send error. Reply was not sent to the client.\n");
                exit(1);
            }else{
                //printf("sent yes..\n");
            }
        }
        // else{
        //     strcmp(ftp_confirm, "yes");
        //     //printf("server rejecting ftp... sending no\n");
        //     ///send(client_socket, &ftp_confirm, sizeof(ftp_confirm),0);
        // }
        ///sendto(server_socket, (const char *)ftp_confirm, sizeof(ftp_confirm),0, (const struct sockaddr *) &client_address, len); 
        //sendto(server_socket, &ftp_confirm, sizeof(ftp_confirm),0, (const struct sockaddr *) &client_address, len);
        // send(server_socket, &ftp_confirm, sizeof(ftp_confirm),0);

        char name[buff] = {0};
        FILE *file = NULL;
        bool *fragment = NULL;
        packet_data.filename = malloc(1100);

        char bufferr[1100] = {0};
        while (1){


            if (recvfrom(server_socket, bufferr, 1100, 0, (struct sockaddr *) &client_addr, &cli) == -1){
                fprintf(stderr, "recvfrom error\n"); exit(1);
            }

            toPacket(bufferr,&packet_data);



            if (!file){
                strcpy(name,basename(packet_data.filename));
                while (access(name,F_OK) == 0){
                    char *suf = strrchr(name,'.');
                    char suffix[buff] = {0};
                    strcpy(suffix, suf);
                    *suf = '\0';
                    strcat(name, "copy");
                    strcat(name, suffix);
                }
                file = fopen(basename(name),"w");
            }


            if (!fragment){
                fragment = (bool *)malloc(packet_data.total_frag * sizeof(fragment));
                for (int i=0; i<packet_data.total_frag; i++){
                    fragment[i] = false;
                }
            }

            if (!fragment[packet_data.frag_no]){
                if (fwrite(packet_data.filedata,sizeof(char),packet_data.size, file)!=packet_data.size){
                    fprintf(stderr,"error during writing the file [fwrite]\n");exit(1);
                }
                fragment[packet_data.frag_no]=true;
            }

            strcpy(packet_data.filedata,"ACK");

            toString(&packet_data, bufferr);

            if ((sendto(server_socket,bufferr,1100,0,(struct sockaddr *) &client_addr, sizeof(client_addr))) == -1){
                fprintf(stderr,"error while sending info \n"); exit(1);
            }

            if (packet_data.frag_no == packet_data.total_frag){
                //printf ("File transfer is complete for %s\n", name);
                break;
            }

        }

            //close the socket
            //printf("closing server socket...\n");
        // close(server_socket);
        // fclose(file);
        // free(packet_data.filename);
        // free(fragment);

        // char jij[32];
        // sprintf(jij, "./server_ftp %d &", PORT_NO);
        // system(jij);
    }
    return 0;
}