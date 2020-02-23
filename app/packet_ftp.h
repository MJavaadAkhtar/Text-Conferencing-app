#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h> //functionalities and APIs used for sockets
#include <sys/types.h>
#include <arpa/inet.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define IP_PROTOCOL 0

#define size_packet 1000
#define buff 1200


struct packet
{
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char *filename;
    char filedata[size_packet];
};

void toPacket(char *str, struct packet *packet);
void toString(struct packet *packet, char *str);

void file_transfer(int socketfd, struct sockaddr_in server_socket, char *filename){
    
    char buf[1100];
    struct timeval timee;
    int setTime = 0;

    //opening the file
    FILE *file;
    if ((file=fopen(filename, "r")) == NULL){
        fprintf(stderr,"Failed to open file. fopen error \n"); exit(1);
    }
    //reading file
    fseek(file, 0, SEEK_END);
    int packet_total = ftell(file)/size_packet +1 ;
    printf("Total number of packets: %d\n", packet_total);
    rewind(file);

    

    char **packets = (char**)malloc(sizeof(char*) * packet_total);

    

    
    for (int i = 1; i<= packet_total;i++)
    {
        //Creating a packet
        struct packet packets_data;
        memset(packets_data.filedata, 0, size_packet);
        fread(packets_data.filedata, sizeof(char), size_packet, file);
        
        // update packet info
        packets_data.frag_no = i;
        packets_data.total_frag = packet_total;
        
        // strcpy(packets_data.filename, filename);
        packets_data.filename = filename;
            

        //Checking if the packet sent is the last one or not
        //save packet to array
        if (i==packet_total)
            fseek(file,0,SEEK_END);
        packets_data.size = i!=packet_total ? size_packet : (ftell(file)-1) % size_packet + 1;
        packets[i-1] = (char *)malloc(1100 * sizeof(char));
        
        toString(&packets_data,packets[i-1]);

    }

    
    timee.tv_usec = 0;
    timee.tv_sec = 1;

    if (setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timee, sizeof(timee)) < 0)
    {
        fprintf(stderr, "error with set socket\n");
    }
    
    socklen_t size_server = sizeof(server_socket);



    struct packet packet_ACK;

    packet_ACK.filename = (char *)malloc(1100 * sizeof(char));


    for (int i = 1; i<=packet_total; i++)
    {
        setTime++;
        

        if ((sendto(socketfd, packets[i - 1], 1100, 0, (struct sockaddr *)&server_socket, sizeof(server_socket))) == -1)
        {
            fprintf(stderr, "send to error inside packet.h \n"); exit(1);
       }

        memset(buf, 0, sizeof(char) *  1100);
        if ((recvfrom(socketfd, buf, 1100, 0, (struct sockaddr *)&server_socket, &size_server))==-1){
            fprintf(stderr,"recv error or Timeout for ACK packet:%d, resending ...\n", i--);
            if (setTime < 32){
                continue;
            }
            else{
                fprintf(stderr, "File Transfer Terminated. Too many sends \n"); exit(1);
            }
        }
        toPacket(buf,&packet_ACK);
        
        if ((strcmp(packet_ACK.filename,filename) == 0) && 
                (packet_ACK.frag_no == i) &&
                    (strcmp(packet_ACK.filedata,"ACK") == 0)){
                        setTime=0; continue;
                    }
        fprintf(stderr,"Failed to send ACK Packet:%d, resending ....\n", i);
    }

    for(int i =1; i<=packet_total ; i++ ){
        free(packets[i-1]);
    }
    free(packets);
    free(packet_ACK.filename);

   
    

}


void toPacket(char * str, struct packet *packet){
    regex_t reg; 
    regmatch_t match[1];
    int cur  = 0;
    char buffer[buff];

    // Compiling regex to match the packet convention that is ':'
    if (regcomp(&reg, "[:]", REG_EXTENDED)){
        fprintf(stderr, "Regex Expression issue\n"); exit(1);
    }

    //Parsing a string to a packet
    //total_frag
    if (regexec(&reg, str+cur,1 , match, REG_NOTBOL)){
        fprintf(stderr, "Error in matching total_frag\n");exit(1);
    }
    memset(buffer,0,buff);
    memcpy(buffer, str+cur, match[0].rm_so);
    packet->total_frag = atoi(buffer);
    cur+=(match[0].rm_so + 1);
    //frag_no
    if (regexec(&reg, str + cur, 1, match, REG_NOTBOL))
    {
        fprintf(stderr, "Error in matching total_frag\n");
        exit(1);
    }
    memset(buffer, 0, buff);
    memcpy(buffer, str + cur, match[0].rm_so);
    packet->frag_no = atoi(buffer);
    cur += (match[0].rm_so + 1);
    //size
    if (regexec(&reg, str + cur, 1, match, REG_NOTBOL))
    {
        fprintf(stderr, "Error in matching total_frag\n");
        exit(1);
    }
    memset(buffer, 0, buff);
    memcpy(buffer, str + cur, match[0].rm_so);
    packet->size = atoi(buffer);
    cur += (match[0].rm_so + 1);
    //filename and file data
    if (regexec(&reg, str + cur, 1, match, REG_NOTBOL))
    {
        fprintf(stderr, "Error in matching total_frag\n");
        exit(1);
    }
    memcpy(packet->filename, str + cur, match[0].rm_so);
    packet->filename[match[0].rm_so] = 0;
    cur += (match[0].rm_so + 1);
    memcpy(packet -> filedata, str+cur,packet->size);
    
}

void toString(struct packet *packet, char *str){
    memset(str,0,1100);
    
    // converting data into a string
    int cur=0;
    sprintf(str, "%d", packet->total_frag);
    cur = strlen(str);
    memcpy(str + cur, ":", sizeof(char));
    ++cur;
    sprintf(str + cur, "%d", packet->frag_no);
    cur = strlen(str);
    memcpy(str + cur, ":", sizeof(char));
    ++cur;
    sprintf(str + cur, "%d", packet->size);
    cur = strlen(str);
    memcpy(str + cur, ":", sizeof(char));
    ++cur;
    sprintf(str + cur, "%s", packet->filename);
    cur += strlen(packet->filename);
    memcpy(str + cur, ":", sizeof(char));
    ++cur;
    memcpy(str + cur, packet->filedata, size_packet);
    
}
// 