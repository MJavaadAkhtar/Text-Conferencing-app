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
 Now your server is up and running. Time to connect clients.
 
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
