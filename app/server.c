#include <sys/socket.h> //functionalities and APIs used for sockets
#include <netinet/in.h> //structures to store addr info
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "message.h"
#include "packet.h"



#define MAX_CLIENTS 100
#define BUFFER_SZ 2048
#define BUFFER_SZ2 600
#define UserList "user_db.txt"

static _Atomic unsigned int cli_count = 0;
static int uid = 10;

// //Client Structure
// typedef struct{
// 	struct sockaddr_in address; //client address
// 	int sockfd;                 //connection file descriptor
// 	int uid;                    //client ID
// 	char name[32];              //client name
//     char pwd[32];               //password
// } client_t;

client_t *usrlst=NULL,*loggedin = NULL;
char sess_name_h[32];
int conectedUsr=0, sess_count=1;
client_t *clients[MAX_CLIENTS];
Sessions *sesslst;


// Pthreads and synchornisation
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t login_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t session_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sess_count_mutex = PTHREAD_MUTEX_INITIALIZER;


//getting sockaddr, IPv4 or IPv6
void *get_addr(struct sockaddr *s)
{
	if (s->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in *)s)->sin_addr);
	}
	return &(((struct sockaddr_in6 *)s)->sin6_addr);
}


//handle communications with clients
void *handle_client(void *arg){
	char buff_out[BUFFER_SZ2];
	char name[32];
	int leave_flag = 0;

	// cli_count++;
	client_t *cli = (client_t *)arg;
	Sessions *join = NULL;

	printf("New thread created\n");



	//State of the User
	int log_in_status = 0, Exit = 0, byte_sent, byte_recv;

	// Packet sent and recieved
	Packets sent, recv_p; 

	

	while(1){


		//---
		memset(buff_out, 0, sizeof(char) * BUFFER_SZ2);
		memset(&recv_p, 0, sizeof(Packets));
		memset(&recv_p, 0, sizeof(Packets));
		//---
		if (leave_flag) {
			break;
		}

		if ((byte_recv = recv(cli->sockfd, buff_out, BUFFER_SZ2-1, 0))==-1){
			fprintf(stderr,"error in recv_p inside handler\n"); exit(1);
		}

		if (byte_recv == 0)
			Exit = 1;
		buff_out[byte_recv]='\0';

		int sendto=0;
		printf("Recieved Message: \"%s\"\n", buff_out);

		toPacket(buff_out, &recv_p);
		memset(&sent,0,sizeof(Packets));

		if (recv_p.type == EXT){
			Exit=1;
		}

		if (log_in_status == 0){
			if(recv_p.type == LOGIN) {	
                // Parse and check username and password
                int cursor = 0;
                strcpy(cli -> name, (char *)(recv_p.source));
                strcpy(cli -> pwd, (char *)(recv_p.data));
                
                printf("Name: %s\n", cli -> name);
                printf("Password: %s\n", cli -> pwd);

                // Check if user already joined
                int alreadyJnd = in_list(loggedin, cli);				

                int vldusr = is_valid_user(usrlst, cli);
                // Clear user password for safety
                memset(cli -> pwd, 0, 32);

                if(vldusr && !alreadyJnd) {
                    printf("User credentials are positive, is valid user\n");
                    sent.type = LOGIN_ACK;
                    sendto = 1;
                    log_in_status = 1;
                    cli -> ftp_socket= (int)(atoi(recv_p.file_sock));
                    // Add user to the login user list
                    client_t *tmp = malloc(sizeof(client_t));
                    memcpy(tmp, cli, sizeof(client_t));
                    pthread_mutex_lock(&login_mutex);
                    loggedin = adding(loggedin, tmp);
                    pthread_mutex_unlock(&login_mutex);
                    strcpy(name, cli -> name);
                    printf("User %s: Successfully logged in...\n", cli -> name);
                    printf("User %s: FTP port is %d && and ip_addr:%s\n", cli->name,cli->ftp_socket, cli->ip_adddr);

                } else {
                    printf("User credentials are negative, not a valid user.\n");
                    sent.type = LOGIN_NACK;
                    sendto = 1;
                    if(alreadyJnd) {
                        strcpy((char *)(sent.data), "Multiple log-in etecte");
                    } else if(in_list(usrlst, cli)) {	
                        strcpy((char *)(sent.data), "Password does not matches");
                    } else {
                        strcpy((char *)(sent.data), "User not registeredd");
                    }
                    printf("Log in failed\n");
                    Exit = 1;
                }
            } else {
                sent.type = LOGIN_NACK;
                sendto = 1;
                strcpy((char *)(sent.data), "Please log in first");
            }

		}

        else if (recv_p.type == JOIN){
            // int session_name = atoi((char *)recv_p.data);
            char *session_name = (char *)recv_p.source;
            printf("source %s\n", session_name);
            // int session_id_ = atoi((char *)recv_p.data);
            // strcpy(session_name,recv_p.data);
            printf("Server Log: User %s is trying to join session %s\n",cli->name,session_name);
            if (ValidSession_name(sesslst, session_name)==NULL){
                sent.type = JN_NACK;
                sendto = 1;
                // int cursor = sprintf((char*)(sent.data),"%s",session_name);
                int cursor = sprintf((char*)(sent.data),"'%s'",session_name);
                printf("cursor:%d, strlen:%d\n", cursor,strlen(sent.source));
                // strcpy(sent.data+strlen(name_temp),"Session does not exist.\n");
                strcpy((char *)(sent.data+cursor)," Session does not exist.\n");
                printf("ssent data %s\n",sent.data);
                fprintf(stdout, "Session Log: User %s failed to join a session cause \
                session %s doesn't exist.\n", cli->name, session_name);
            }
            else if ((name_insession(sesslst, session_name,cli))){
                sent.type = JN_NACK;
                sendto=1;
                int cursor = sprintf((char*)(sent.data),"%s",session_name);
                // strcpy(sent.data+strlen(name_temp),"Session does not exist.\n");
                strcpy((char *)(sent.data+cursor),"Session already joined.\n");
                fprintf(stdout, "Session Log: User %s failed to join a session cause \
                session %s doesn't exist.\n", cli->name, session_name);
            }
            else{
                sent.type = JOIN_ACK;
                sendto=1;
                sprintf((char*)(sent.data),"%s",session_name);
                pthread_mutex_lock(&session_mutex);
                sesslst = join_session_name(sesslst,session_name,cli);
                pthread_mutex_unlock(&session_mutex);
                int temp4= valid_sess_id(sesslst, session_name);

                join = init_session(join, temp4,session_name);
                printf("%s User Successfully joined session %d, %s\n", cli->name,temp4, session_name);

                pthread_mutex_lock(&login_mutex);
                client_t *temp_usr = loggedin;
                while(temp_usr!=NULL){
                    if (strcmp(temp_usr -> name, name) == 0){
                        temp_usr -> session_status = 1;
                        temp_usr -> join_sess = init_session(temp_usr->join_sess,sess_count,sess_name_h);
                    }
                    temp_usr = temp_usr->next;
                }
                pthread_mutex_unlock(&login_mutex);

            }
        }

        else if(recv_p.type == FTP_MSG){
            printf("User %s: Sending FTP \"%s\"\n", cli->name, recv_p.data);
            int curSess = atoi(recv_p.source);
            memset(&sent, 0, sizeof(Packets));
            //preparing to send a message
            sent.type = FTP_rec;
            // strcpy((char *)(sent.source), cli->name);
            // strcpy((char *)(sent.data), (char *)(recv_p.data));
            // sent.size = strlen((char *)(sent.data));
            strcpy((char*)(sent.data),"");

            memset(buff_out, 0, sizeof(char) * 600);
            sendto=1;
            // toString(&sent, buff_out);
            // fprintf(stderr, "Server Log: message broadcasting \"%s\" to session:", buff_out);
            for (Sessions *cur = join; cur != NULL; cur = cur->next)
            {
                Sessions *sessToSend;
                if ((sessToSend = ValidSession(sesslst, cur->id)) == NULL)
                    continue;
                printf(" %d", sessToSend->id);
                for (client_t *usr = sessToSend->usr; usr != NULL; usr = usr->next)
                {

                        char buf_temp_[32];
                        sprintf(buf_temp_, ";%s %d", usr->ip_adddr, usr->ftp_socket);
                        printf("buff_temp_:%s\n", buf_temp_);
                        strcat((char *)sent.data, buf_temp_);
                    
                }
            }
            printf("sent: %s\n",sent.data);
            toString(&sent, buff_out);

            // printf("\n");
        }

	
		else if(recv_p.type == NEW_SESS) {
            printf("User %s: Trying to create new session...:\n", cli -> name);
			strcpy(sess_name_h, recv_p.source);

            // Update global session_list
            toString(&sent ,buff_out); 
            pthread_mutex_lock(&session_mutex);
            sesslst = init_session(sesslst, sess_count,sess_name_h);
            pthread_mutex_unlock(&session_mutex);

            // User join just created session
            join = init_session(join, sess_count,sess_name_h);
            pthread_mutex_lock(&session_mutex);
            sesslst = join_session(sesslst, sess_count, cli);
            pthread_mutex_unlock(&session_mutex);

            // Update user status in userLoggedin;
            pthread_mutex_lock(&login_mutex);
            for(client_t *usr = loggedin; usr != NULL; usr = usr -> next) {
                if(strcmp(usr -> name, name) == 0) {
                    usr -> session_status = 1;
                    usr -> join_sess = init_session(usr -> join_sess, sess_count,sess_name_h);
                }
            }
            pthread_mutex_unlock(&login_mutex);
            sent.type = NS_ACK;
            sendto = 1;
            sprintf((char *)(sent.data), "%d", sess_count);
			sprintf((char *)(sent.source), "%s", sess_name_h);
            pthread_mutex_lock(&sess_count_mutex);
            sess_count++;
            pthread_mutex_unlock(&sess_count_mutex);
            printf("User %s: Session created with id - %d\n", cli -> name, sess_count - 1);

        }


        // User send message
        else if(recv_p.type == MESSAGE) {
            printf("User %s: Sending message \"%s\"\n", cli -> name, recv_p.data);
            int curSess = atoi(recv_p.source);
            memset(&sent, 0, sizeof(Packets));
            //preparing to send a message
            sent.type = MESSAGE;
            strcpy((char *)(sent.source), cli -> name);
            strcpy((char *)(sent.data), (char *)(recv_p.data));
            sent.size = strlen((char *)(sent.data));
            memset(buff_out, 0, sizeof(char) * 600);
            toString(&sent, buff_out);
            fprintf(stderr, "Server Log: message broadcasting \"%s\" to session:", buff_out);
            for(Sessions *cur = join; cur != NULL; cur = cur -> next) {
                Sessions *sessToSend;
                if((sessToSend = ValidSession(sesslst, cur -> id)) == NULL) continue;
                printf(" %d", sessToSend -> id);
                for(client_t *usr = sessToSend -> usr; usr != NULL; usr = usr -> next) {
                    if((byte_sent = send(usr -> sockfd, buff_out, 600 - 1, 0)) == -1) {
                        perror("Error in sening message\n");
                        exit(1);
                    }
                }
            }
            printf("\n");
            sendto = 0;
        }

        else if (recv_p.type == LEAVE){
            printf("come here\n");
            // while(join!=NULL){
            //     int temp_sess = join -> id;
            //     Sess *ccr = join;
            //     // join = 
            // }
        }

        else if(recv_p.type == LIST) {
            printf("User %s: Requesting List\n", cli -> name);
            int cursor = 0;
            sent.type = LIST_ACK;
            sendto = 1;
            for(client_t *usr = loggedin; usr != NULL; usr = usr -> next) {
                cursor += sprintf((char *)(sent.data) + cursor, "%s", usr -> name);
                for(Sessions *sess = usr -> join_sess; sess != NULL; sess = sess -> next) {
                    cursor += sprintf((char *)(sent.data) + cursor, "\t\t\t%d", sess -> id);
					cursor += sprintf((char *)(sent.data) + cursor, "\t\t%s", sess -> name);
                }
                sent.data[cursor++] = '\n';
            }

            printf("List Result:\n%s", sent.data);
        }

		if(sendto) {
            memcpy(sent.source, cli -> name, 32);
            sent.size = strlen((char *)(sent.data));
            memset(buff_out, 0, 600);

            toString(&sent, buff_out);
            printf("This is buffer: %s\n",buff_out);
            if((byte_recv = send(cli -> sockfd, buff_out, 600 - 1, 0)) == -1) {
                perror("error send\n");
            }
        }
        printf("\n");

        if(Exit) break;
	}

	close(cli->sockfd);
	
	return NULL;
}
//send list of clients (AND SHOULD PRINT AVAILABLE SESSIONS)
void send_active_clients(int sockfd){
    char s[64];

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i){
        if (clients[i]) {
            sprintf(s, "<< [%d] %s\r\n", clients[i]->uid, clients[i]->name);
            if (write(sockfd, s, strlen(s)) < 0) {
               perror("Write to descriptor failed");
               exit(-1);
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

int main(int argc, char **argv){
	if (argc != 2)
    {
        fprintf(stderr, "Invalid use of arguments. -> ./server <TCP listen port>\n");
        EXIT_FAILURE;
    } 

	//char *ip = "127.0.0.1";
	int port = atoi(argv[1]);
	int option = 1;
    int connfd = 0;
    struct sockaddr_in server_address;
    struct sockaddr_in cli_addr;
    pthread_t tid;
	socklen_t size_sin;
	struct sockaddr_storage addr_stor;
	char t[INET6_ADDRSTRLEN];



	//---
	// Load Users from a file
	FILE *f;
	if ((f=fopen(UserList, "r")) == NULL){
		fprintf(stderr, "Can not user database file %s\n", UserList);
	}
	usrlst = initializeDB(f);
	fclose(f);
	printf("Server Log: Database of Users are loaded in the system");

	
	//---




    //set server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) 
        printf("\nfile descriptor not received.\n"); 
    else
        printf("\nfile descriptor %d received\n", server_socket);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);
    memset(server_address.sin_zero,0,sizeof(server_address.sin_zero));

    //pipe signals ignore
	//signal(SIGPIPE, SIG_IGN);

	if(setsockopt(server_socket, SOL_SOCKET,(SO_REUSEADDR),(char*)&option,sizeof(option)) < 0){
		perror("\nSetsockopt failed\n");
        return EXIT_FAILURE;
	}

	//bind specified ip address and port # to server socket
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == 0) //cast struct sockaddr only
        printf("\nSuccessfully binded!\n"); 
    else
        printf("\nBinding Failed!\n");

    //listen 
    if (listen(server_socket, 10) < 0) {
        perror("\nSocket listening failed\n");
        return EXIT_FAILURE;
    }
    printf("Welcome~\n");
	//////////should create different sessions 
    do {
        while(1){

            //---
            client_t *new = (client_t*) calloc(sizeof(client_t), 1);
            size_sin = sizeof(addr_stor);
            new->sockfd = accept(server_socket, (struct sockaddr *)&addr_stor, &size_sin);
            if (new -> sockfd == -1){
                perror("issue with accept by the new user"); break;
            }
            inet_ntop(addr_stor.ss_family, get_addr((struct sockaddr *)&addr_stor), t, sizeof(t));
            // printf("%s\n", addr_stor.ss_family);
            strcpy(new->ip_adddr,t);
            printf("Server Log: accepted connection from %s\n", t);
            //---
            pthread_mutex_lock(&clients_mutex);
            conectedUsr++;
            pthread_mutex_unlock(&clients_mutex);

          
            pthread_create(&(new->p), NULL, handle_client, (void *)new);

            ///mayhaps
            //sleep(1);
        }
    } while (conectedUsr > 0);

    return EXIT_SUCCESS;
}
