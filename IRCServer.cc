
const char * usage =
"                                                               \n"
"IRCServer:                                                   \n"
"                                                               \n"
"Simple server program used to communicate multiple users       \n"
"                                                               \n"
"To use it in one window type:                                  \n"
"                                                               \n"
"   IRCServer <port>                                          \n"
"                                                               \n"
"Where 1024 < port < 65536.                                     \n"
"                                                               \n"
"In another window type:                                        \n"
"                                                               \n"
"   telnet <host> <port>                                        \n"
"                                                               \n"
"where <host> is the name of the machine where talk-server      \n"
"is running. <port> is the port number you used when you run    \n"
"daytime-server.                                                \n"
"                                                               \n";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "IRCServer.h"

int QueueLength = 5;

int
IRCServer::open_server_socket(int port) {

	// Set the IP address and port for this server
	struct sockaddr_in serverIPAddress; 
	memset( &serverIPAddress, 0, sizeof(serverIPAddress) );
	serverIPAddress.sin_family = AF_INET;
	serverIPAddress.sin_addr.s_addr = INADDR_ANY;
	serverIPAddress.sin_port = htons((u_short) port);

	// Allocate a socket
	int masterSocket =  socket(PF_INET, SOCK_STREAM, 0);
	if ( masterSocket < 0) {
		perror("socket");
		exit( -1 );
	}

	// Set socket options to reuse port. Otherwise we will
	// have to wait about 2 minutes before reusing the sae port number
	int optval = 1; 
	int err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, 
			(char *) &optval, sizeof( int ) );

	// Bind the socket to the IP address and port
	int error = bind( masterSocket,
			(struct sockaddr *)&serverIPAddress,
			sizeof(serverIPAddress) );
	if ( error ) {
		perror("bind");
		exit( -1 );
	}

	// Put socket in listening mode and set the 
	// size of the queue of unprocessed connections
	error = listen( masterSocket, QueueLength);
	if ( error ) {
		perror("listen");
		exit( -1 );
	}

	return masterSocket;
}

	void
IRCServer::runServer(int port)
{
	int masterSocket = open_server_socket(port);

	initialize();

	while ( 1 ) {

		// Accept incoming connections
		struct sockaddr_in clientIPAddress;
		int alen = sizeof( clientIPAddress );
		int slaveSocket = accept( masterSocket,
				(struct sockaddr *)&clientIPAddress,
				(socklen_t*)&alen);

		if ( slaveSocket < 0 ) {
			perror( "accept" );
			exit( -1 );
		}

		// Process request.
		processRequest( slaveSocket );		
	}
}

	int
main( int argc, char ** argv )
{
	// Print usage if not enough arguments
	if ( argc < 2 ) {
		fprintf( stderr, "%s", usage );
		exit( -1 );
	}

	// Get the port from the arguments
	int port = atoi( argv[1] );

	IRCServer ircServer;

	// It will never return
	ircServer.runServer(port);

}

//
// Commands:
//   Commands are started y the client.
//
//   Request: ADD-USER <USER> <PASSWD>\r\n
//   Answer: OK\r\n or DENIED\r\n
//
//   REQUEST: GET-ALL-USERS <USER> <PASSWD>\r\n
//   Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//
//   REQUEST: CREATE-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LIST-ROOMS <USER> <PASSWD>\r\n
//   Answer: room1\r\n
//           room2\r\n
//           ...
//           \r\n
//
//   Request: ENTER-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LEAVE-ROOM <USER> <PASSWD>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: SEND-MESSAGE <USER> <PASSWD> <MESSAGE> <ROOM>\n
//   Answer: OK\n or DENIED\n
//
//   Request: GET-MESSAGES <USER> <PASSWD> <LAST-MESSAGE-NUM> <ROOM>\r\n
//   Answer: MSGNUM1 USER1 MESSAGE1\r\n
//           MSGNUM2 USER2 MESSAGE2\r\n
//           MSGNUM3 USER2 MESSAGE2\r\n
//           ...\r\n
//           \r\n
//
//    REQUEST: GET-USERS-IN-ROOM <USER> <PASSWD> <ROOM>\r\n
//    Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//

	void
IRCServer::processRequest( int fd )
{
	// Buffer used to store the comand received from the client
	const int MaxCommandLine = 1024;
	char commandLine[ MaxCommandLine + 1 ];
	int commandLineLength = 0;
	int n;

	// Currently character read
	unsigned char prevChar = 0;
	unsigned char newChar = 0;

	//
	// The client should send COMMAND-LINE\n
	// Read the name of the client character by character until a
	// \n is found.
	//

	// Read character by character until a \n is found or the command string is full.
	while ( commandLineLength < MaxCommandLine &&
			read( fd, &newChar, 1) > 0 ) {

		if (newChar == '\n' && prevChar == '\r') {
			break;
		}

		commandLine[ commandLineLength ] = newChar;
		commandLineLength++;

		prevChar = newChar;
	}

	// Add null character at the end of the string
	// Eliminate last \r
	commandLineLength--;
	commandLine[ commandLineLength ] = 0;

	printf("RECEIVED: %s\n", commandLine);

	/*
	printf("The commandLine has the following format:\n");
	printf("COMMAND <user> <password> <arguments>. See below.\n");
	printf("You need to separate the commandLine into those components\n");
	printf("For now, command, user, and password are hardwired.\n");
	*/
	char * temp = strdup(commandLine);
	char * token = strtok(commandLine," ");
	int count = 0;
	const char * command = NULL;
	const char * user = NULL;
	const char * password = NULL;
	char * args = (char *) malloc (100);
	args = NULL;
	while (token != NULL) {
		count++;
		if (count==1) command = strdup(token);
		if (count==2) user = strdup(token);
		if (count==3) password = strdup(token);
		if (count==4) {
			args = strdup(strstr(temp,password));
			args = args + strlen(password) + 1;
			break;
		}
		token = strtok(NULL," ");
	}
	printf("command=%s\n", command);
	printf("user=%s\n", user);
	printf( "password=%s\n", password );
	printf("args=%s\n", args);

	if (!strcmp(command, "ADD-USER")) {
		addUser(fd, user, password, args);
	}
	else if (!strcmp(command, "CREATE-ROOM")) {
		createRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "LIST-ROOMS")) {
		listRooms(fd, user, password, args);
	}
	else if (!strcmp(command, "ENTER-ROOM")) {
		enterRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "LEAVE-ROOM")) {
		leaveRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "SEND-MESSAGE")) {
		sendMessage(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-MESSAGES")) {
		getMessages(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-USERS-IN-ROOM")) {
		getUsersInRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-ALL-USERS")) {
		getAllUsers(fd, user, password, args);
	}
	else {
		const char * msg =  "UNKNOWN COMMAND\r\n";
		write(fd, msg, strlen(msg));
	}

	// Send OK answer
	//const char * msg =  "OK\n";
	//write(fd, msg, strlen(msg));

	close(fd);	
}

	void
IRCServer::initialize()
{
	// Open password file
	FILE * file = fopen("password.txt","a+");
	if (file == NULL) {
		printf("Cannot open file\n");
		exit(1);
	}
	// Initialize users in room
	const size_t lineSize = 300;
	char * line = (char *) malloc (lineSize);
	while (fgets(line, lineSize, file) != NULL) {
		char * token = strtok(line,":");
		int count = 0;
		const char * name;
		const char * pswd;
		while (token != NULL) {
			count++;
			if (count==1) name = strdup(token);
			if (count==2) pswd = strdup(token);
			token = strtok(NULL,":");
		}
		char * pswd2 = strdup(pswd);
		pswd2[strlen(pswd2)-1] = '\0';
		users.insertItem(name, (void *) pswd2);
	}
	fclose(file);
	// Initalize message list
}

bool
IRCServer::checkPassword(int fd, const char * user, const char * password) {
	// Here check the password
	HashTableVoidIterator iterator(&users);
	const char * _user;
	void * _data;
	while (iterator.next(_user, _data)) {
		if (!strcmp((const char *)_data, password) && !strcmp(_user, user)) {
			return true;
		}
	}
	return false;
}

	void
IRCServer::addUser(int fd, const char * user, const char * password, const char * args)
{
	if (user==NULL || password==NULL) {
		const char * msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	if (args!=NULL) {
		const char * msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	// Here add a new user. For now always return OK.
	FILE * file = fopen("password.txt","a+");
	if (file == NULL) {
		printf("Cannot open file\n");
		exit(1);
	}
	
	if (users.find(user, (void**) &password) == true) {
		const char * msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	users.insertItem(user, (void *) password);
	
	fprintf(file, "%s:%s\n", user, password);
	fclose(file);

	const char * msg =  "OK\r\n";
	write(fd, msg, strlen(msg));
	return;		
}

void
IRCServer::createRoom(int fd, const char * user, const char * password, const char * args) {
	if (args==NULL || user==NULL || password==NULL) {
		const char * msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	char * extra = strdup(args);
	char * token = strtok(extra," ");
	int counter = 0;
	while (token != NULL) {
		counter++;
		if (counter>=2) {
			const char * msg = "DENIED\r\n";
			write(fd, msg, strlen(msg));
			return;
		}
		token = strtok(NULL," ");
	}
	if (checkPassword(fd, user, password) == false) {
		const char * msg = "ERROR (Wrong password)\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	int i;
	char * room = strdup(args);
	for (i=0; i < n; i++) {
		if (!strcmp(room, roomArray[i])) {
			const char * msg = "DENIED\r\n";
			write(fd, msg, strlen(msg));
			return;
		}
	}
	strcpy(roomArray[n++],room);
	const char * msg = "OK\r\n";
	write(fd, msg, strlen(msg));
	return;
}

void
IRCServer::listRooms(int fd, const char * user, const char * password, const char * args) {
	if (user==NULL || password==NULL) {
		const char * msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	int i, j;
	if (checkPassword(fd, user, password) == false) {
		const char * msg = "ERROR (Wrong password)\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	int isRoom = 0;
	for (i=0; i < n; i++) {
		for (j=0; j < n-1; j++) {
			if (strcmp(roomArray[j],roomArray[j+1])>0) {
				char temp[100];
				strcpy(temp,roomArray[j]);
				strcpy(roomArray[j],roomArray[j+1]);
				strcpy(roomArray[j+1],temp);
			}
		}
	}
	for (i=0; i < n; i++) {
		write(fd, roomArray[i], strlen(roomArray[i]));
		write(fd, "\r\n", 2);
		isRoom = 1;
	}
	if (isRoom == 0) {
		const char * msg = "ERROR (No rooms)\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
}

void
IRCServer::enterRoom(int fd, const char * user, const char * password, const char * args)
{
	if (args==NULL || user==NULL || password==NULL) {
		const char * msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	char * extra = strdup(args);
	char * token = strtok(extra," ");
	int counter = 0;
	while (token != NULL) {
		counter++;
		if (counter>=2) {
			const char * msg = "DENIED\r\n";
			write(fd, msg, strlen(msg));
			return;
		}
		token = strtok(NULL," ");
	}
	if (checkPassword(fd, user, password) == false) {
		const char * msg = "ERROR (Wrong password)\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	char * room = strdup(args);
	char * random = strdup(room);
	const char * _room;
	int findRoom = 0;
	int i;
	for (i=0; i < n; i++) {
		if (!strcmp(room, roomArray[i])) {
			findRoom = 1;
		}
	}
	if (findRoom != 1) {	
		const char * msg = "ERROR (No room)\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	if (rooms.find(user, (void**) &_room) == true) {
		if (strstr((char *)_room, room) != NULL) {
			const char * msg = "OK\r\n";
			write(fd, msg, strlen(msg));
			return;
		}
		else {
			char * num = (char *) malloc (sizeof(char)*100);
			strcpy(num,strcat((char *)_room," "));
			strcpy(num,strcat(num,room));
			rooms.insertItem(user, (void *) num);
		}
	}
	else {
		rooms.insertItem(user, (void *) random);
	}

	const char * msg = "OK\r\n";
	write(fd, msg, strlen(msg));
	return;
}

	void
IRCServer::leaveRoom(int fd, const char * user, const char * password, const char * args)
{
	if (user==NULL || password==NULL) {
		const char * msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	if (args!=NULL) {
	char * extra = strdup(args);
	char * token1 = strtok(extra," ");
	int counter = 0;
	while (token1 != NULL) {
		counter++;
		if (counter>=2) {
			const char * msg = "DENIED\r\n";
			write(fd, msg, strlen(msg));
			return;
		}
		token1 = strtok(NULL," ");
	}
	}
	if (checkPassword(fd, user, password) == false) {
		const char * msg = "ERROR (Wrong password)\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	char * temp;
	if (args!=NULL) temp = strdup(args);
	const char * room;
	if (rooms.find(user, (void**) &room) == false) {
		const char * msg = "ERROR (No user in room)\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	else if (args!=NULL && strstr((char *) room, temp)==NULL) {
		const char * msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	if (args==NULL) rooms.removeElement(user);
	else {
		char * random = strdup(room);
		printf("%s\n", room);
		char * rm = strdup(args);
		char * token = strtok(random," ");
		int count = 0;
		int changes = 0;
		char * num = (char *) malloc (sizeof(char)*100);
		while (token != NULL) {
			count++;
			if (strcmp(token,rm) && changes==0) {
				strcpy(num,token);
				changes++;
			}
			if (strcmp(token,rm) && changes>0 && count>1) {
				strcpy(num,strcat(num," "));
				strcpy(num,strcat(num,token));
				changes++;
			}
			token = strtok(NULL," ");
		}
		if (count==1) {
			rooms.removeElement(user);
		}
		else {
			printf("%s\n", num);
			rooms.insertItem(user, (void *) num);
		}

	}
	const char * msg = "OK\r\n";
	write(fd, msg, strlen(msg));
	return;
}

	void
IRCServer::sendMessage(int fd, const char * user, const char * password, const char * args)
{
	if (args==NULL || user==NULL || password==NULL) {
		const char * msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	if (checkPassword(fd, user, password) == false) {
		const char * msg = "ERROR (Wrong password)\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	char * extra = strdup(args);
	char * dup = strdup(args);
	char * token = strtok(extra," ");
	int counter = 0;
	char * message;
	char * room;
	while (token != NULL) {
		counter++;
		if (counter==1) room = strdup(token);
		if (counter==2) message = strdup(token);
		if (counter>2) {
			message = strcat(message," ");
			message = strcat(message,token);
		}
		token = strtok(NULL," ");
	}
	if (counter <= 1) {
		const char * msg = "ERROR (user not in room)\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	void * _room;
	if (numberOfMessages>=100) numberOfMessages = 0;
	if (rooms.find(user, (void**) &_room) == true) {
		if (strstr((const char *)_room, room) != NULL) {
			strcpy(messages[numberOfMessages],message);
			strcpy(messageRoom[numberOfMessages],room);
			strcpy(messageUser[numberOfMessages],user);
			numberOfMessages++;
			const char * msg = "OK\r\n";
			write(fd, msg, strlen(msg));
			return;
		}
		else {
			const char * msg = "ERROR (user not in room)\r\n";
			write(fd, msg, strlen(msg));
			return;
		}
	}
	else {
		const char * msg = "ERROR (user not in room)\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
}

void
IRCServer::getMessages(int fd, const char * user, const char * password, const char * args)
{
	if (args==NULL || user==NULL || password==NULL) {
		const char * msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	if (checkPassword(fd, user, password) == false) {
		const char * msg = "ERROR (Wrong password)\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	char * extra = strdup(args);
	char * token = strtok(extra," ");
	int counter = 0;
	char * message = NULL;
	char * room = NULL;
	while (token != NULL) {
		counter++;
		if (counter==1) message = strdup(token);
		if (counter==2) room = strdup(token);
		if (counter>2) {
			const char * msg = "DENIED\r\n";
			write(fd, msg, strlen(msg));
			return;
		}
		token = strtok(NULL," ");
	}
	if (counter==1) {
		const char * msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	if (message[0]>'9' || message[0]<'0') {
		const char * msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	void * _room;
	if (rooms.find(user, (void**) &_room) == true) {
		if (strstr((const char *)_room, room) != NULL) {
		}
		else {
			const char * msg = "ERROR (User not in room)\r\n";
			write(fd, msg, strlen(msg));
			return;
		}
	}
	else {
		const char * msg = "ERROR (User not in room)\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	int value = 0;
	value = atoi(message);
	if (value >= numberOfMessages) {
		const char * msg = "NO-NEW-MESSAGES\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	int findRoom = 0;
	int i;
	for (i=0; i < n; i++) {
		if (!strcmp(room, roomArray[i])) {
			findRoom = 1;
		}
	}
	if (findRoom != 1) {	
		const char * msg = "ERROR (No room)\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	i = 0;
	int j = value;
	int ct = 0;
	for (i=value; i < numberOfMessages; i++) {
		if (!strcmp(messageRoom[i],room)) {
			ct++;
		}
	}
	//if (ct != 0) ct--;
	if (ct<=value) {
		const char * msg = "NO-NEW-MESSAGES\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	ct = 0;
	for (i=value; i < numberOfMessages; i++) {
		if (!strcmp(messageRoom[i],room)) {
			char num[100];
			sprintf(num, "%d", j);
			strcpy(num,strcat(num," "));
			char * msg = (char *) malloc (100);
			msg = strdup(messageUser[i]);
			strcpy(msg,strcat(msg," "));
			char * msg2 = (char *) malloc (100);
			msg2 = messages[i];
			write(fd, num, strlen(num));
			write(fd, msg, strlen(msg));
			write(fd, msg2, strlen(msg2));
			write(fd, "\r\n", 2);
			j++;
		}
	}
	write(fd, "\r\n", 2);
}

	void
IRCServer::getUsersInRoom(int fd, const char * user, const char * password, const char * args)
{
	if (args==NULL || user==NULL || password==NULL) {
		const char * msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	char * extra = strdup(args);
	char * token = strtok(extra," ");
	int counter = 0;
	while (token != NULL) {
		counter++;
		if (counter>=2) {
			const char * msg = "DENIED\r\n";
			write(fd, msg, strlen(msg));
			return;
		}
		token = strtok(NULL," ");
	}
	if (checkPassword(fd, user, password) == false) {
		const char * msg = "ERROR (Wrong password)\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	char * room = strdup(args);
	char * random = strdup(args);
	// User does not have to be in the room
	/*
	if (rooms.find(user, (void**) &random) == false) {
		const char * msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}*/
	int findRoom = 0;
	int i;
	for (i=0; i < n; i++) {
		if (!strcmp(room, roomArray[i])) {
			findRoom = 1;
		}
	}
	if (findRoom != 1) {	
		const char * msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	HashTableVoidIterator iterator(&rooms);
	const char * _user;
	void * _data;
	char userRoom[200][100];
	int m = 0;
	while (iterator.next(_user, _data)) {
		char * rm = strdup((char *)_data);
		char * token = strtok(rm," ");
		int count = 0;
		char num[100];
		while (token != NULL) {
			count++;
			if (!strcmp(token, room)) {
				strcpy(userRoom[m],_user);
				m++;
			}
			token = strtok(NULL," ");
		}
	}
	int j;
	for (i=0; i < m; i++) {
		for (j=0; j < m-1; j++) {
			if (strcmp(userRoom[j],userRoom[j+1])>0) {
				char temp[100];
				strcpy(temp,userRoom[j]);
				strcpy(userRoom[j],userRoom[j+1]);
				strcpy(userRoom[j+1],temp);
			}
		}
	}
	for (i=0; i < m; i++) {
		const char * msg = strdup(userRoom[i]);
		write(fd, msg, strlen(msg));
		write(fd, "\r\n", 2);
	}
	write(fd, "\r\n", 2);
	return;
}

	void
IRCServer::getAllUsers(int fd, const char * user, const char * password,const  char * args)
{
	if (user==NULL || password==NULL) {
		const char * msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	if (args!=NULL) {
		const char * msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	if (checkPassword(fd, user, password) == false) {
		const char * msg = "ERROR (Wrong password)\r\n";
		write(fd, msg, strlen(msg));
		return;
	}
	HashTableVoidIterator iterator(&users);
	const char * _user;
	void * _data;
	char userArray[200][100];
	int i, m = 0, j;
	while (iterator.next(_user, _data)) {
		strcpy(userArray[m],_user);
		m++;
		printf("%s\n", _user);
	}
	printf("%d\n", m);
	for (i=0; i < m; i++) {
		for (j=0; j < m-1; j++) {
			if (strcmp(userArray[j],userArray[j+1])>0) {
				char temp[100];
				strcpy(temp,userArray[j]);
				strcpy(userArray[j],userArray[j+1]);
				strcpy(userArray[j+1],temp);
			}
		}
	}
	for (i=0; i < m; i++) {
		const char * msg = strdup(userArray[i]);
		write(fd, msg, strlen(msg));
		write(fd, "\r\n", 2);
	}
	write(fd, "\r\n", 2);
	return;
}

