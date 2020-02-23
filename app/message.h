#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h> //functionalities and APIs used for sockets
#include <sys/types.h>
#include <arpa/inet.h>
#include <regex.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define IP_PROTOCOL 0

#define size_packet 1000
#define buff 1200

// void toMsgPacket(char *str, struct message *message);
// void toString(struct message *message, char *str);

struct message
{
    unsigned int type;
    unsigned int size;
    unsigned char source[100];
    unsigned char data[1000];
};
//gonna implement
typedef struct Users client_t;
//Sessions id
typedef struct Sess {
    int id;
	char name[32]; // name of the session
    struct Sess *next;
    struct Users *usr;
} Sessions;

//Client Structure
typedef struct Users
{
    char name[32]; //client name
    char pwd[32];  //password
    //struct sockaddr_in address; //client address
    int sockfd;                 //connection file descriptor
    int uid;                    //client ID

    pthread_t p;

    // Information for chain hashing purposes
    int session_status;
    struct Users *next; 
    Sessions *join_sess;

	int ftp_socket; // Socket for FTP connections
	char ip_adddr[32];

} client_t;

client_t *adding(client_t *lst, client_t *new_usr){
    new_usr -> next = lst;
    return new_usr;
}

client_t *initializeDB(FILE *p) {
    int read;
    client_t *superUser=NULL;

    for(;;){
        client_t *usrs = (client_t *)calloc(1, sizeof(client_t));
        read = fscanf(p, "%s %s\n",usrs->name, usrs->pwd);
        if (read==EOF){
            free(usrs);break;
        }

        superUser = adding(superUser, usrs);
    }
    return superUser;
}


// Check username and password of user (valid, connected, logged in, etc)
int is_valid_user(const client_t *userList, const client_t *usr) {
    const client_t *current = userList;
    while(current != NULL) {
        if(strcmp(current -> name, usr -> name) == 0 && strcmp(current -> pwd, usr -> pwd) == 0) {
            return 1;    
        }
        current = current -> next;
    }
    return 0;
}

// Check if user is in list (matches username)
int in_list(const client_t *userList, const client_t *usr) {
    const client_t *current = userList;
    while(current != NULL) {
        if(strcmp(current -> name, usr -> name) == 0) {
            return 1;    
        }
        current = current -> next;
    }
    return 0;
}




Sessions *init_session(Sessions *sesslst, const int sessionId, char *name) {
	Sessions *newSession = (Sessions*)calloc(sizeof(Sessions), 1);
	newSession -> id = sessionId;
	strcpy(newSession -> name , name);
	newSession -> next  = sesslst;
	return newSession;
}


Sessions *ValidSession(Sessions *sesslst, int sessionId) {
	Sessions *sess = sesslst;
	while(sess != NULL) {
		if(sess -> id == sessionId) 
			return sess;
		sess = sess -> next;
	}
	return NULL;
}

int valid_sess_id(Sessions *sesslst, char *name){
    Sessions *sess = sesslst;
	while(sess != NULL) {
		// if(sess -> name == name)
		// 	return sess;
        if((strcmp(sess->name,name)) == 0)
			return sess->id;
		sess = sess -> next;
	}
	return -1;
}

Sessions *ValidSession_name(Sessions *sesslst, char *name) {
	Sessions *sess = sesslst;
	while(sess != NULL) {
		// if(sess -> name == name)
		// 	return sess;
        if((strcmp(sess->name,name)) == 0)
			return sess;
		sess = sess -> next;
	}
	return NULL;
}

Sessions *join_session(Sessions *sesslst, int sessionId, const client_t *usr) {
	Sessions *cur = ValidSession(sesslst, sessionId);
	assert(cur != NULL);
	// Malloc new user
	client_t *newUsr = (client_t*)malloc(sizeof(client_t));
	memcpy((void *)newUsr, (void *)usr, sizeof(client_t));
	// Insert into session list
	cur -> usr = adding(cur -> usr, newUsr);
	return sesslst;
}

Sessions *join_session_name(Sessions *sesslst, char * name, const client_t *usr) {
	Sessions *cur = ValidSession_name(sesslst, name);
	assert(cur != NULL);
	// Malloc new user
	client_t *newUsr = (client_t*)malloc(sizeof(client_t));
	memcpy((void *)newUsr, (void *)usr, sizeof(client_t));
	// Insert into session list
	cur -> usr = adding(cur -> usr, newUsr);
	return sesslst;
}

int name_insession(Sessions *sesslst, char * name, const client_t *usr) {
	Sessions *sess = ValidSession_name(sesslst, name);
	if(sess != NULL) {
		return in_list(sess -> usr, usr);
	} else {
		return 0;
	}
}

int inSession(Sessions *sesslst, int sessionId, const client_t *usr) {
	Sessions *session = ValidSession(sesslst, sessionId);
	if(session != NULL) {
		// Sessions exists, then check if user is in session
		return in_list(session -> usr, usr);
	} else {
		return 0;
	}
}
