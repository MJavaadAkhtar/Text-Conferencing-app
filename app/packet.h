#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

enum Type_ack
{
    LOGIN,
    LOGIN_ACK,
    LOGIN_NACK,
    JN_NACK,
    NEW_SESS,
    NS_ACK,
    MESSAGE,
    LIST,
    EXT,
    JOIN,
    JOIN_ACK,
    LEAVE,
    LIST_ACK,
    FTP_MSG,
    FTP_rec
};

typedef struct _packet
{
    unsigned int type; // msgType
    unsigned int size; // Size of data
    unsigned char source[32];
    unsigned char data[512];
    unsigned char file_sock[64];
} Packets;

void toPacket(const char *str, Packets *packet)
{
	regex_t regex;
    memset(packet->data, 0, 512);
    if (strlen(str) == 0)
        return;
    if (regcomp(&regex, "[:]", REG_EXTENDED))
    {
        fprintf(stderr, "Could not compile regex\n");
    }
    regmatch_t match[1];
    char buffer[512]; 
	int cursor = 0;
    if (regexec(&regex, str + cursor, 1, match, REG_NOTBOL))
    {
        fprintf(stderr, "Error matching regex\n");
        exit(1);
    }
    memset(buffer, 0, 512 * sizeof(char));
    memcpy(buffer, str + cursor, match[0].rm_so);
    packet->type = atoi(buffer);
    cursor += (match[0].rm_so + 1);
    if (regexec(&regex, str + cursor, 1, match, REG_NOTBOL))
    {
        fprintf(stderr, "Error matching regex\n");
        exit(1);
    }
    memset(buffer, 0, 512 * sizeof(char));
    memcpy(buffer, str + cursor, match[0].rm_so);
    packet->size = atoi(buffer);
    cursor += (match[0].rm_so + 1);
    if (regexec(&regex, str + cursor, 1, match, REG_NOTBOL))
    {
        fprintf(stderr, "Error matching regex\n");
        exit(1);
    }
    // memcpy(packet->source, str + cursor, match[0].rm_so);
    // packet->source[match[0].rm_so] = 0;
    // cursor += (match[0].rm_so + 1);
    // memcpy(packet->data, str + cursor, packet->size);

    memset(buffer, 0, 512 * sizeof(char));
    memcpy(packet->source, str + cursor, match[0].rm_so);
    packet->source[match[0].rm_so] = 0;
    cursor += (match[0].rm_so + 1);
    if (regexec(&regex, str + cursor, 1, match, REG_NOTBOL))
    {
        fprintf(stderr, "Error matching regex\n");
        exit(1);
    }

    memcpy(packet->data, str + cursor, match[0].rm_so);
    packet->data[match[0].rm_so] = 0;
    cursor += (match[0].rm_so + 1);
    memcpy(packet->file_sock, str + cursor, 64);

    // printf("%d:%d:%s:%s%s\n",packet->type,packet->size, packet->source,packet->data, packet->file_sock);

}



void toString(Packets *packet, char *str){
    memset(str,0,sizeof(char)*600);
    
    // converting data into a string
    int cur=0;
    sprintf(str, "%d", packet->type);
    cur = strlen(str);
    memcpy(str + cur, ":", sizeof(char));
    ++cur;
    sprintf(str + cur, "%d", packet->size);
    cur = strlen(str);
    memcpy(str + cur, ":", sizeof(char));
    ++cur;
    sprintf(str + cur, "%s", packet->source);
    cur = strlen(str);
    memcpy(str + cur, ":", sizeof(char));
    ++cur;
    // sprintf(str + cur, "%s", packet->file_sock);
    // cur += strlen(str);
    // memcpy(str + cur, ":", sizeof(char));
    // ++cur;
    memcpy(str + cur, packet->data, strlen((char*)(packet->data)));
    cur = strlen(str);
    ++cur;

    sprintf(str + cur, "%s", packet->file_sock);
    cur = strlen(str);
    memcpy(str + cur, ":", sizeof(char));
}
// 
