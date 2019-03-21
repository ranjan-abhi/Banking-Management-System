#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>


#define PACKET_SIZE 512

char* messageFromServer(int socket_fd);
void messageToServer(int socket_fd, char *str);


void encrypt(char str[],int key)		   // encrypt function to encrypt password
{
    unsigned int i;
    for(i=0;i<strlen(str);++i)
    {
          str[i] = str[i] - key;			// adding key value to actual password
    }
}

void closeConnection(int socket_fd)				// function to close Connection between client and server
{
	shutdown(socket_fd, SHUT_WR);
}

int unauthorizeAccess(char *receivedMessage)			// function which compare message comes from server
{
	return strcmp(receivedMessage, "Wrong username or password");
}


int wrongAccess(char *receivedMessage)						// function which compare message comes from server
{
	return strcmp(receivedMessage, "Unauthorized User, Process Terminated");
}

int exitRequest(char *sendMessage)				// function which compare message comes from server for exit status
{
	return strcmp(sendMessage,"exit");
}

int talkToServer(int socket_fd)							//function which sends to or recieve from server
{
  	int flag = 0;
  	char *receivedMessage;
  	unsigned char sendMessage[256];

    receivedMessage = messageFromServer(socket_fd);				//recieve message from server

    if(receivedMessage == NULL)														//if message is null, close Connection
    	return 1;

			int temp = 0;
		if(strcmp(receivedMessage, "Enter Password: ") == 0){				// if recieved message from server is password, increment temp
			temp = 1;
		}

    if(!unauthorizeAccess(receivedMessage))				// if username or password is Wrong
    {
        printf("Wrong username or password. Please enter valid username or password.\n");
        return 0;
    }

		if(!wrongAccess(receivedMessage)){								// if username or password is entered more than 3 times
			printf("Unauthorized Access, Process Terminated.\n");
			closeConnection(socket_fd);													// close Connection
			return 1;
		}

    printf("%s\n",receivedMessage);											// print message comes from server
    free(receivedMessage);

    memset(sendMessage, 0, sizeof(sendMessage));
    scanf("%s", sendMessage);															// takes message from client which needs to be sent
		if(temp == 1){
			 encrypt(sendMessage,0xFACA);												//if recieved message was "enter password" then encrypt the message which needs to sent
		}
    messageToServer(socket_fd, sendMessage);							// send message to server

    if(!exitRequest(sendMessage))
    {																									// if recieved message is exit then close Connection
        closeConnection(socket_fd);
        return 1;
    }

	return 0;
}

int messageAfterClientCloseConnection(int socket_fd)		// fuction which listen if some message is coming from server after Connection is closed
{
	char *receivedMessage;
	receivedMessage = messageFromServer(socket_fd);
    if(receivedMessage == NULL)											// if recieved message is null then close Connection
    	return 1;
    printf("%s\n",receivedMessage);										// print recieved message
    free(receivedMessage);
    return 0;
}

char* messageFromServer(int socket_fd) {
    int numPacketsToReceive = 0;													// function which recieves message from server and store in a string str
    int n = read(socket_fd, &numPacketsToReceive, sizeof(int));
    if(n <= 0) {
        closeConnection(socket_fd);
        return NULL;
    }
    char *str = (char*)malloc(numPacketsToReceive*PACKET_SIZE);			// allocate size to str
    memset(str, 0, numPacketsToReceive*PACKET_SIZE);
    char *str_p = str;
    int i;
    for(i = 0; i < numPacketsToReceive; ++i) {
        read(socket_fd, str, PACKET_SIZE);											// read message from server and put in str
        str = str+PACKET_SIZE;
    }
    return str_p;
}

void messageToServer(int socket_fd, char *str) {				//	function to send message to server
    int numPacketsToSend = (strlen(str)-1)/PACKET_SIZE + 1;
    int n = write(socket_fd, &numPacketsToSend, sizeof(int));
    char *msgToSend = (char*)malloc(numPacketsToSend*PACKET_SIZE);
    strcpy(msgToSend, str);
    int i;
    for(i = 0; i < numPacketsToSend; ++i) {
        write(socket_fd, msgToSend, PACKET_SIZE);						// write message to socket
        msgToSend += PACKET_SIZE;
    }
}
int main(int argc,char **argv)					// main function
{
	int socket_fd,port_no;
	struct sockaddr_in serv_addr;

	socket_fd=socket(AF_INET, SOCK_STREAM, 0);				//creating socket descripter
	port_no = atoi(argv[2]);													// taking port no. from argument

	memset(&serv_addr, 0, sizeof(serv_addr));  //  setting sockaddr_in serv_addr
	serv_addr.sin_family = AF_INET;         //set DOMAIN
    serv_addr.sin_port = htons(port_no);     //set port number
    inet_aton(argv[1], &serv_addr.sin_addr);		  // taking ip address from argument and set it in socket

    connect(socket_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));			// make Connection between client and server

    printf("Connection Established.\n");

    while(1)
    {
        int responseOfServer = talkToServer(socket_fd);				// talk to server until exit status comes

        if(responseOfServer) break;
    }

    while(1)
    {																										// listen to server after Connection is closed until message is null
        int responseOfServerAfterClientClose = messageAfterClientCloseConnection(socket_fd);
        if(responseOfServerAfterClientClose)
        	break;
    }
    printf("Write end closed by the server.\n");
    closeConnection(socket_fd);															// finally close Connection between client and server
    printf("Connection closed successfully.\n");
    return 0;


}
