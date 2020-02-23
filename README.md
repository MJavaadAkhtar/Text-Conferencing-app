# Text-Conferencing-app
Conferening Application in C
<hr>

### Overview:
This is a Text Conferencing application integerated with my own implementation of FTP and SMTP.

- The application is created using the **C language**
- It utilizes Sockets for packet transportation
- Uses 3-way handshake of TCP connection to create a connection between two computers and UDP model to transfer file between computers.
- Created our own custom **Round-Trip delay time (RTT)** which the TCP uses to estimate timeout.

### Features:
- Allows users to create account and login 
- Users are able to create their own conferencing sessions or can join an already existing session. 
- Users can invite other valid users to their conferencing sessions via ping
- Gives thhe ability to the users to run bash terminal commands inside their conference session
- ALlows users to send files within the conference session using custom made FTP(File Transfer protocol)

### How to Use
Make sure you have __gcc__ installed in your terminal. Open two terminals, one for client and on for server.

#### Server Terminal:
- go to __app__ directory
- run the following command:
`./server <TCP port_number>`
 Now your server is up and running. Here is an example below:
 
 ```sh
mohammadjavaadakhtar:~/Documents/Text-Conferencing-app/app> ./server 50000
Server Log: Database of Users are loaded in the system
file descriptor 3 received

Successfully binded!
Welcome~
 ```
 
 Time to connect clients. 
 
 #### Client Terminal
 - go to __app__ directory
 - run the following command:
 `./client`
 
 Now your client is connected to the server. You can use the following commands:
 
 | Command | Description |
|---------|-------------|
|  `/login <name> <pswd> <server-ip> <server-port>`  |    Logins to the server at this given address. You can have multiple server running on different computers, this will specify which server to login.        |
| `/logout`  | Exit the server. You need to login again to continue. |
|`/createsession <session_name>`|Create a new conference session and join it.|
|`/joinsession <session_name>`|Join a session that is already created by someone else.|
|`\leavesession`|Leave a session you already in. If you are the only one in the session, the session will be destroyed when you leave.|
|`/list`| Shows the list of client and avaliable sessions.|
|`/quit`| quit the text conferencing application.|
|`<text>`| Send a text to everyone in the session.|
|`/ls`| Similar to bash command `ls`. |
|`/changesess`| Change a your session. |
|`/ftp`| In order to do a File Transfer using my own implementation of FTP. |

Here is an example:

This is client 1 (name _a_):
```properties
mohammadjavaadakhtar:~/Documents/Text-Conferencing-app/app:~/app$ ./client
/login a b 127.0.0.1 50000
client ID: a 
password: b 
server IP: 127.0.0.1 
server Port: 50000 
Client Log: connecting with 127.0.0.1
port:50005
Client Log: login successful.
/createsession scrumMeeting
Successfully created and joined session 1.
javaad: Hi this is javaad
Hi this is a
a: Hi this is a
```

This is client 2 (name _javaad_):
```properties
mohammadjavaadakhtar:~/Documents/Text-Conferencing-app/app:~/app$ ./client
/login javaad 1234 127.0.0.1 50000
client ID: javaad 
password: 1234 
server IP: 127.0.0.1 
server Port: 50000 
Client Log: connecting with 127.0.0.1
port:50002
Client Log: login successful.

/list
User_name		Session_id		Session_name
javaad
a			1		scrumMeeting

/joinsession scrumMeeting
Successfully joined session scrumMeeting.

Hi this is javaad
javaad: Hi this is javaad
a: Hi this is a
/list
User_name		Session_id		Session_name
javaad			2		scrumMeeting
a			1		scrumMeeting
/ls
client	    deliver_ftp.c  packet_ftp.h  server.c      temp_file.txt
client.c    Makefile	   packet.h	 server_ftp    testing.c
client_ftp  message.h	   server	 server_ftp.c  user_db.txt
```
