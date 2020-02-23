/*
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <sys/socket.h> //functionalities and APIs used for sockets
#include <sys/types.h> 
#include <arpa/inet.h>
#include <netinet/in.h> //structures to store addr info
//#include "packet.h"
#include <netdb.h>
#include <pthread.h>

#define IP_PROTOCOL 0 
#define MAX_CLIENTS 100 
//

int main(int argc, char const * argv[]){

}
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h> //structures to store addr info
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "message.h"
#include "packet.h"

#define LENGTH 2048
#define MAX 500
#define IP_PROTOCOL 0 

int sess_in = 0;
char buf_in[600];
// Global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[32];
char filename[32];

void str_overwrite_stdout() {
  printf("%s", "> ");
  fflush(stdout);
}

void *get_addr(struct sockaddr *s)
{
	if (s->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in *)s)->sin_addr);
	}
	return &(((struct sockaddr_in6 *)s)->sin6_addr);
}


void str_trim(char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) { // trim \n
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}

void catch_ctrl_c_exit(int sig) {
    flag = 1;
}

int arg_login_error(char* uid, char* pwd, char* server_ip, char* server_port){
    if ((uid == NULL) || (pwd== NULL) || (server_ip==NULL) || (server_port== NULL)){
        printf("Missing arguments: format. -> /login <client ID> <password> <server-IP> <server-port>\n");
        return 1;
    }
    return 0;
}

void send_msg_handler() {
  char message[LENGTH] = {};
	char buffer[LENGTH + 32] = {};

  while(1) {
  	str_overwrite_stdout();
    fgets(message, LENGTH, stdin);
    str_trim(message, LENGTH); //hm

    if (strcmp(message, "exit") == 0) {
			break;
    } else {
      sprintf(buffer, "%s: %s\n", name, message);
      send(sockfd, buffer, strlen(buffer), 0);
    }

		bzero(message, LENGTH);
    bzero(buffer, LENGTH + 32);
  }
  catch_ctrl_c_exit(2);
}

void recv_msg_handler() {
	char message[LENGTH] = {};
    while (1) {
		int receive = recv(sockfd, message, LENGTH, 0);
    if (receive > 0) {
      printf("%s", message);
      str_overwrite_stdout();
    } else if (receive == 0) {
			break;
    } 
	memset(message, 0, sizeof(message));
  }
}

void *receive(void *socketfd_void_p) {
	// receive may get: JOIN_ACK, JN_NAC, NS_ACK, LIST_ACK, MESSAGE
  // only LOGIN packet is not listened by receive
    int *socketfd_p = (int *)socketfd_void_p;
	int numbytes;
	Packets packet;
    for (;;) {	
		if ((numbytes = recv(*socketfd_p, buf_in, 600 - 1, 0)) == -1) {
			fprintf(stderr, "client receive: recv\n");
			return NULL;
		}
	if (numbytes == 0) continue;
    buf_in[numbytes] = 0;
    //fprintf(stdout, "buf: %s\n", buf);
		toPacket(buf_in, &packet);
    //fprintf(stdout, "packet.type: %d, packet.data: %s\n", packet.type, packet.data);
    if (packet.type == JOIN_ACK) {
      fprintf(stdout, "Successfully joined session %s.\n", packet.data);
      sess_in = 1;
    } else if (packet.type == JN_NACK) {
      fprintf(stdout, "Join failure. Detail: %s\n", packet.data);
      sess_in = 0;
    } else if (packet.type == NS_ACK) {
      fprintf(stdout, "Successfully created and joined session %s.\n", packet.data);
      sess_in = 1;
    } else if (packet.type == LIST_ACK) {
      fprintf(stdout, "User_name\t\tSession_id\t\tSession_name\n%s", packet.data);
    } else if (packet.type == MESSAGE){   
      	fprintf(stdout, "%s: %s\n", packet.source, packet.data);
    }
    else if (packet.type == FTP_rec){
      char *ips_addr = packet.data;
      char buffer_ips[512];
      char buffer_tempo[512];
      char *lol = strtok(ips_addr, ";");
      while (lol != NULL)
      {
        strcpy(buffer_ips, lol);
        // printf("%s\n", buffer_ips);
        printf("system s: %s\n", buffer_tempo);
        sprintf(buffer_tempo, "./client_ftp %s %s", buffer_ips, filename);
        system(buffer_tempo);
        printf("system s: %s\n", buffer_tempo);

        lol = strtok(NULL, ";");

      }
    }

      else
      {
        fprintf(stdout, "Unexpected packet received: type %d, data %s\n",
                packet.type, packet.data);
      }
    // solve the ghost-newliner 
    fflush(stdout);
	}
	return NULL;
}

void login(char *uid, char* password, char* server_ip, char* server_port, pthread_t *recv_t, int *socket_fd){
  int rec;
    struct addrinfo client_addr, *server_inf, *pointer_addr;
    char s[INET6_ADDRSTRLEN];
    memset(&client_addr, 0, sizeof client_addr);
    client_addr.ai_socktype = SOCK_STREAM;
    client_addr.ai_family = AF_UNSPEC;
 
    
    if ((rec = getaddrinfo(server_ip, server_port, &client_addr, &server_inf)) != 0) {
      fprintf(stderr, "Error in client login getaddrinfo: %s\n", gai_strerror(rec));return;
    }
    for(pointer_addr = server_inf; pointer_addr != NULL; pointer_addr = pointer_addr->ai_next) {
      if ((*socket_fd = socket(pointer_addr->ai_family, pointer_addr->ai_socktype, pointer_addr->ai_protocol)) == -1) {
        fprintf(stderr ,"Client Log: socket[inside login]\n");
        continue;
      }
      if (connect(*socket_fd, pointer_addr->ai_addr, pointer_addr->ai_addrlen) == -1) {
        close(*socket_fd);
        fprintf(stderr, "Client Log: connect[insie login]\n");
        continue;
      }
      break; 
    }
    if (pointer_addr == NULL) {
      fprintf(stderr, "Client Log: failed to connect\n");
      close(*socket_fd);
      *socket_fd = -1;
      
    }
    inet_ntop(pointer_addr->ai_family, get_addr((struct sockaddr *)pointer_addr->ai_addr), s, sizeof s);
    printf("Client Log: connecting with %s\n", s);
    freeaddrinfo(server_inf); // all done with this structure

    int numbytes;
    Packets packet;
    packet.type = LOGIN;
    strncpy(packet.source, uid, 32);
    strncpy(packet.data, password, 32);
    packet.size = strlen(packet.data);

    system("./server_ftp 50001 &");
    FILE *fp;
    char temp_buff[64];
    fp = fopen("temp_file.txt", "r");
    fscanf(fp, "%s", temp_buff);
    printf("port:%s\n", temp_buff);

    strncpy(packet.file_sock, temp_buff,64);
    // system("rm ")

    toString(&packet, buf_in);
    if ((numbytes = send(*socket_fd, buf_in, 600 - 1, 0)) == -1)
    {
      fprintf(stderr, "Client Log: socket[inside login]\n");
      close(*socket_fd);
      *socket_fd = -1;
      return;
    }

    if ((numbytes = recv(*socket_fd, buf_in, 600 - 1, 0)) == -1) {
      fprintf(stderr, "Client Log: recv[inside login]\n");
      close(*socket_fd);
      *socket_fd = -1;
      return;
    }
    buf_in[numbytes] = 0; 
    toPacket(buf_in, &packet);
    if (packet.type == LOGIN_ACK &&
        pthread_create(recv_t, NULL, receive, socket_fd) == 0)
    {
      fprintf(stdout, "Client Log: login successful.\n");  
    } else if (packet.type == LOGIN_NACK) {
      fprintf(stdout, "Client Log: login failed. Detail: %s\n", packet.data);
      close(*socket_fd);
      *socket_fd = -1;
      return;
    } else {
      fprintf(stdout, "Unexpected packet received: type %d, data %s\n", 
          packet.type, packet.data);
      close(*socket_fd);
      *socket_fd = -1;
      return;
    } 
        
}



int main(int argc, char **argv){
    
    char data[MAX];
    char *command;
    int toklen;
    pthread_t client_thread;
    int sok = -1;
    while(1){
        // scanf("%[^\n]s", buf_in);
        fgets(buf_in, 6000-1, stdin);
        buf_in[strcspn(buf_in,"\n")] = 0;

        // char* command = strtok(buf_in," ");
        command = buf_in;
        while (*command == ' ') command++;
        if (*command == 0)
        {
          // ignore empty command
          continue;
        }

        //printf("command, socket: %s, %d \n ", command,sok);
        command = strtok(buf_in, " ");
        // printf("command, socket: %s, %d \n ", command, sok);
        toklen = strlen(command);
        if (strcmp(command,"/login")==0){
            command= strtok(NULL," ");
            char* uid = command;
            printf("client ID: %s \n", uid);
            command = strtok(NULL," ");
            char* password = command;
            printf("password: %s \n", password);
            command = strtok(NULL," ");
            char* server_ip = command;
            printf("server IP: %s \n", server_ip);
            command = strtok(NULL," ");
            char* server_port = command;
            printf("server Port: %s \n", server_port);

            login(uid, password, server_ip, server_port, &client_thread,&sok);

              

        }

        else if (strcmp(command, "/joinsession")==0){
          if (sok == -1)
            fprintf(stderr,"You have not logged in yet, please login.\n");
          else if (sess_in)
            fprintf(stdout,"You are already in a session. Please exit the current session to \
            to join a new session.\n");
          else{
            command = strtok(NULL,"");
            char *name_sess = command;
            if (name_sess == NULL)
              fprintf(stdout, "usage: /joinsession <session name>");
            Packets packs;
            packs.type = JOIN;
            packs.size = strlen(name_sess);
            strncpy(packs.source, name_sess, 512);
            toString(&packs, buf_in);
            int a;
            if ((a = send(sok, buf_in, 600-1,0))==-1)
              fprintf(stderr, "Client Log: Send failed in join session\n");
            
          }
        }

        else if (strcmp(command, "/list") == 0)
        {
          //send list of all active clients in SERVER
          if (sok == -1)
          {
            fprintf(stdout, "You have not logged in yet, please login.\n");
          }
          int total_byte;
          Packets pack;
          pack.type = LIST;
          pack.size = 0;
          toString(&pack, buf_in);
          if ((total_byte = send(sok, buf_in, 600 - 1, 0)) == -1)
          {
            fprintf(stderr, "client: send\n");
          }
        }

        else if (strcmp(command, "/ls") == 0)
        {
          //send list of all active clients in SERVER
          if (sok == -1)
          {
            fprintf(stdout, "You have not logged in yet, please login.\n");
          }
          system("ls");
        }

      
        else if (strcmp(command,"/createsession")==0){
            command= strtok(NULL," ");
            char* session_id = command;
            // //printf("session_id: %s \n", session_id);
            if(session_id == NULL){
                 printf("Missing arguments: format. -> /createsession <session ID>\n");
                //  return EXIT_FAILURE;
             }
            if (sok == -1)
              fprintf(stdout, "You have not logged in yet, please login.\n");
            else if (sess_in)
              fprintf(stdout, "You are already in a session.\n");
            else{
              int bytes_;
              Packets packs;
              packs.type = NEW_SESS;
              packs.size = 0;
			  strncpy(packs.source, session_id, 512);

              toString(&packs, buf_in);		
              // printf("socket %d", sok);
              if ((bytes_ = send(sok, buf_in, 600 - 1, 0)) == -1) {
                // printf("this is bytes_ : %d", bytes_);
                fprintf(stderr, "client: send\n");
              } 
            }
         
        }

        else if (strcmp(command,"/ftp")==0){
          command=strtok(NULL, " ");
          char* session_id = command;

          if (session_id == NULL)
          {
            printf("Missing arguments: format. -> /ftp <path to the file>\n");
            return EXIT_FAILURE;
          }
          if (sok == -1)
            fprintf(stdout, "You have not logged in yet, please login.\n");
          else if (!sess_in)
            fprintf(stdout, "You have not joined a session. Please join a session.\n");
          else
          {
            int bytes_;
            Packets pack;
            pack.type = FTP_MSG;

            strncpy(pack.data, buf_in, 512);
            strcpy(filename,command);
            pack.size = strlen(pack.data);

            toString(&pack, buf_in);

            if ((bytes_ = send(sok, buf_in, 600 - 1, 0)) == -1)
            {
              fprintf(stderr, "client: send\n");
            }
          }
        }


        
        // if (strcmp(command, "/logout")==0){

        // }
 

        else if (strcmp(command, "/leavesession")==0){}
        else if (strcmp(command, "/quit")==0){
          if (sok == -1){
            fprintf(stdout,"You have not logged into any server. Please login first.\n");
            // break;
          }
          int num;
          Packets pack;
          pack.size=0;
          pack.type = EXT;
          toString(&pack,buf_in);
          num = send(sok,buf_in, 600-1,0);
          pthread_cancel(client_thread);
          sess_in = -1;
          close(sok);
          sok=-1;
        }
        // else if (strcmp(command,"/changesess")){

        // }

        else{ //text message
          // buf_in[]
          buf_in[toklen]=' ';


          if (sok == -1)
            fprintf(stderr, "You have no logged in yet, please login\n");
          else if (!sess_in)
            fprintf(stdout,"You have not joined a session. Please join a session.\n");
          else{
            int bytes_;
            Packets pack;
            pack.type = MESSAGE;


            strncpy(pack.data, buf_in, 512);
         

            pack.size = strlen(pack.data);
        

            toString(&pack, buf_in);
           
            if ((bytes_ = send(sok, buf_in, 600 - 1, 0)) == -1)
            {
              fprintf(stderr, "client: send\n");
            }
          }
           
        }

       
    }
	return 0;
}
