#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>



#define PACKET_SIZE 512
#define LINESINMINISTATEMENT 6
#define EXIT -1
#define CUSTOMER 'C'
#define POLICE 'P'
#define ADMIN 'A'
#define INVALID_USER -1

void decrypt(char str[],int key);
void CustomerChoices(int Client_FD,char *username,char *password);
char *printMS(int Client_FD,char *username);
char *ShowAvailableBalance(char *username);
void AccountUpdate(char *username,int option,double amount,double balance);
int UserExist(char *customername);
void AdminChoices(int Client_FD);
int Transaction(int Client_FD,char *username);
void policeRequests(int Client_FD);
char getClientAndAuthorize(int Client_FD,char* username,char *password);
void ConnectWithClient(int Client_FD,int count);
char* MessageFromClient(int Client_FD);
void MessagetoClient(int Client_FD, char *str);



void decrypt(char str[],int key)   //function to decrypt
{
    unsigned int i;
    for(i=0;i<strlen(str);++i)     // adding key value
    {
          str[i] = str[i] + key;
    }
}

void CustomerChoices(int Client_FD,char *username,char *password)     // function to take choice from costumer and do process
{
	MessagetoClient(Client_FD,"Input your choice\n1. Print Available Balance\n2. Print Mini Statement\nWrite exit to terminate.");
	char *store=NULL;
	int flag=1;

	while(flag)                                               //take choice from costumer
	{
		if(store!=NULL)
			store=NULL;
		store=MessageFromClient(Client_FD);

		int option;

		if(strcmp(store,"exit")==0)                         // if choice is exit then  select option 0
			option=0;
		else
		    option=atoi(store);
		char *Availablebalance=(char *)malloc(1000*sizeof(char));
		char *miniStatement=(char *)malloc(10000*sizeof(char));
		strcpy(Availablebalance,"*****************\nCurrent Balance: ");
		strcpy(miniStatement,"*****************\nYour Mini Statement: \n");

    if(option==1)                                         // if choice is 1 then extract balance from costumer file and send it to client
    {
      strcat(Availablebalance,ShowAvailableBalance(username));
      MessagetoClient(Client_FD,strcat(Availablebalance,"\n********************\n\nInput your choice\n1. Print Available Balance\n2. Print Mini Statement\nWrite exit to terminate."));
      free(Availablebalance);
    }
    else if(option==2)                                  // if choice is 2 then extract miniStatement from costumer file and send it to client
    {
      strcat(miniStatement,printMS(Client_FD,username));
      MessagetoClient(Client_FD,strcat(miniStatement,"\n********************\n\nInput your choice\n1. Print Available Balance\n2. Print Mini Statement\nWrite exit to terminate."));
      free(miniStatement);
    }
    else if(option==0)
    {
      	flag=0;
    }
    else                                            // send message to client
    {
      MessagetoClient(Client_FD, "Wrong Query. Enter Correct Query or exit to terminate.");
    }
	}
}

char *printMS(int Client_FD,char *username)                  // fuction to print miniStatement
{
	FILE *fp_MS_user = fopen(username,"ab+");                 // open user file

	char *MS = NULL;                                       
    // Initializing miniStatement to a blank char.             
	MS = (char *)malloc(10000*sizeof(char));
    MS[0] = '\0';

    size_t len = 0;
    ssize_t read;
    int count=0;
    char * line_MS_user = NULL;
    

    while((read = getline(&line_MS_user, &len, fp_MS_user)) != -1)   // read line by line from user file and put it in string
    {
  		strcat(MS,line_MS_user);
      count++;
      if(count>=LINESINMINISTATEMENT)                    // maximum no. of lines in miniStatement
        break;


}

	fclose(fp_MS_user);


	if(strlen(MS)==0)                            // if user has no transaction records
		strcpy(MS,"None");

	return MS;
}

char *ShowAvailableBalance(char *username)      // function to show available balance of costumer
{
	FILE *fp_bal_user=fopen(username,"ab+");    //open costumer file
	
	size_t len = 0;
    ssize_t read;

    char * line_bal_user = NULL;                
    char * temp_bal_user = NULL;


    read = getline(&line_bal_user, &len, fp_bal_user);    // read line from costumer file
    if((read = getline(&line_bal_user, &len, fp_bal_user)) != -1)
    {
    	char *token,*storeval;                            //tokenize the line
        storeval = (char *)malloc(400*sizeof(char));
    	token=strtok(line_bal_user," ");                 
        int i;
        while(token != NULL){
          strcpy(storeval,token);                     // copy balance part from line to string storeval
          token=strtok(NULL," ");
        }

    	fclose(fp_bal_user);
    	return storeval;                            // return available balance
    }
    else
    {
    	fclose(fp_bal_user);
    	char *balance;
    	balance=(char *)malloc(2*sizeof(char));    // if no balance then return 0
    	balance[0]='0';
    	balance[1]='\0';
    	return balance;
    }

}

void AccountUpdate(char *username,int option,double amount,double balance)    //function which update costumer file
{
	FILE *fp_account_update = fopen(username,"ab+");                // open costumer file

  char c[7];
  if(option==1)                                                  // checking if Account is credited or debited
    strcpy(c,"Credit");
  else
    strcpy(c,"Debit");

    char * line = NULL; 
	char *line_1;
    char *temp=NULL;

    line_1 = (char *)malloc(sizeof(char)*10000);
    
    time_t update_time; /* calendar time */
    size_t len = 0;
    ssize_t read;

  if((read = getline(&line, &len, fp_account_update)) == -1){              // print the data info in first line costumer file
    fprintf(fp_account_update,"%s                 %s      %s         %s\n","Date and Time","Type","Amount Credited/Debited","Current Balance");
  }

	update_time=time(NULL); /* get current cal time */
                                                                    // print the transaction in the costumer file
	sprintf(line_1,"%.*s     %s         %f                   %f\n",(int)strlen(asctime(localtime(&update_time)))-1,asctime(localtime(&update_time)),c,amount,balance);
  
	while((read = getline(&line, &len, fp_account_update)) != -1)
		strcat(line_1,line);

	fclose(fp_account_update);

	fp_account_update=fopen(username,"r+");                // open costumer file and write updated transaction int it
  
    read = getline(&temp, &len, fp_account_update);
    int sizeToWrite=strlen(line_1);
  
	fwrite(line_1, sizeof(char), sizeToWrite, fp_account_update);      // close file
	fclose(fp_account_update);
}

int UserExist(char *customername)               // function to check if a customer exists or not
{
	FILE *fp_userexist=fopen("login_file","r");     // open login file
	
	ssize_t read;
    size_t len = 0;
    
    char * line_userexist = NULL;

	while((read = getline(&line_userexist, &len, fp_userexist)) != -1)   //read line by line from login file
	{
		char *token=strtok(line_userexist," ");
		if(strcmp(token,customername)==0)                // if username matches then return user type
		{
      int i;
      for(i=0;i<2;i++)
		   token=strtok(NULL," ");

			if(token[0]=='C')
			{
				fclose(fp_userexist);
				return 1;
			}
        }
    }

    fclose(fp_userexist);                       // close file
    return 0;


}

void AdminChoices(int Client_FD)                // function which enables to  check if a costumer is exists or not
{
	MessagetoClient(Client_FD,"Enter a customer name or write exit to terminate.");

	while(1)
	{
		char *store;                              // recieve message from client
		store=NULL;
		store=MessageFromClient(Client_FD);


		if(strncmp(store,"exit",4)==0)              // if recieved message is exit then break
			break;
		else if(UserExist(store))
		{
			char *userrequest;
			userrequest = (char *)malloc(40*sizeof(char));          // check if costumer exist
			strcpy(userrequest,store);

			if(Transaction(Client_FD,userrequest)==EXIT)        // if transaction done
				break;
		}
		else
			MessagetoClient(Client_FD,"Customer with this username does not exist. Please enter a valid user or write exit to terminate.");
	}

}

int Transaction(int Client_FD,char *username)         // function to ceedit or debit in costumer account by admin
{
	
	MessagetoClient(Client_FD,"Enter your option.\n1. Credit\n2. Debit\nWrite exit to quit.");
	
	int flag=1;

	while(flag)
	{
		char* store;
		store=MessageFromClient(Client_FD);           // take choice from client

		if(strcmp(store,"exit")==0)                  // if choice is exit then closeConnection
			return EXIT;
		else
		{
			int option=atoi(store);                  
			double availablebalance=strtod(ShowAvailableBalance(username),NULL);    // get available balance of costumer

			if(option!=1 && option!=2)
				MessagetoClient(Client_FD,"Wrong Option. Select correct option.");  // if client chooses wrong choice
			else
			{
				MessagetoClient(Client_FD,"Enter amount for transaction.");       // send message to client to enter

				while(1)
				{
					char *store=MessageFromClient(Client_FD);
					double amount;
					amount=strtod(store,NULL);                                       // if amount to be credited or debited is less
					if(amount<=0)
						MessagetoClient(Client_FD,"Negative amount can't be used for transaction.Enter valid amount.");
					else
					{
						if(option==2 && availablebalance<amount)          // if client wants to debit more amount than balance
						{
							MessagetoClient(Client_FD,"Insufficient Balance.\n*******************\n\nEnter a customer name or write exit to terminate.");
							flag=0;
							break;
						}
						else if(option==2)
							availablebalance = availablebalance - amount;        // if debited
						else if(option==1)
							availablebalance =availablebalance + amount;        // if credited

						AccountUpdate(username,option,amount,availablebalance);    // update costumer
						MessagetoClient(Client_FD,"User's account updated successfully after transaction.\n******************-\n\nEnter a customer name or write exit to terminate.");
						flag=0;
						break;
					}

				}

			}
		}
	}
}

// function to access all the rights provided to a police user
void policeRequests(int Client_FD)
{
   
	// ask the user police to choose his/her choice
	MessagetoClient(Client_FD, "Write your option,\n1. Show Balance of all customer\n2. Show Balance of a particular customer.\n3. Print mini Statement of a customer\nWrite exit to terminate");
	
	int flag=1;
  	int loop=1;

	while(flag)
	{
		char *store=NULL;
		store=MessageFromClient(Client_FD);

		char *balance=(char *)malloc(1000*sizeof(char));
		char *MS=(char *)malloc(10000*sizeof(char));
		
		strcpy(balance,"*****************\nCurrent Balance: \n");
		strcpy(MS,"*****************\nMini Statement: \n");
		
		// if client asks to exit
		if(strncmp(store,"exit",4)==0)
			break;
		else
		{
			int option=atoi(store);

			// if client asks to check the balance of all the users
			if(option==1)
			{
					FILE *fp_policechoice=fopen("login_file","r");       // To Get Balance of All User
	
					size_t len = 0;
				    ssize_t read;

					char * line_policechoice = NULL;
				    
				    char *balanceOfAll;
				    balanceOfAll=(char *)malloc(10000*sizeof(char));
				    balanceOfAll[0]='\0';

					while((read = getline(&line_policechoice, &len, fp_policechoice)) != -1)
					{
						char *token=strtok(line_policechoice," ");
				    	char *checker;
				    	int i;
				    	for(i=0;i<2;i++)
				      		checker=strtok(NULL," ");

						if(checker[0]=='C')
						{
							strcat(balanceOfAll,token);
							strcat(balanceOfAll," ");
							strcat(balanceOfAll,ShowAvailableBalance(token));
				    	}
				  	}
				    fclose(fp_policechoice);
				
				strcat(balance,balanceOfAll);
				MessagetoClient(Client_FD,strcat(balance,"\n************\n\nWrite your option,\n1. Show Balance of all customer\n2. Show Balance of a particular customer.\n3. Print mini Statement of a customer\nWrite exit to terminate"));
			}

			// if client asks to check balance of a particular user, then we have to check if that user exists or not
			else if(option == 2)
			{
				loop =1;
				MessagetoClient(Client_FD,"Write Customer name or exit to quit");          // ask the name of customer
				while(loop)
				{
					store = MessageFromClient(Client_FD);
					if(strcmp(store,"exit")==0)                                             // check if the client chose to exist
					{
						flag=1;
	            		loop=0;
						break;
					}

					// check if the user exists
					else if(UserExist(store))
					{
						char *customer_name;
						customer_name=(char *)malloc(sizeof(char)*40);
						strcpy(customer_name,store);
						strcat(balance,ShowAvailableBalance(customer_name));
						MessagetoClient(Client_FD,strcat(balance,"\n************\n\nWrite your option,\n1. Show Balance of all customer\n2. Show Balance of a particular customer.\n3. Print mini Statement of a customer\nWrite exit to terminate"));
						loop=0;
	        			break;
	        		}
	        		else
	        		{
	        			MessagetoClient(Client_FD,"Customer with this username does not exist. Please enter a valid user or write exit to terminate.");
	        		}
	        	}
	        }

	        // if the client chose to print the mini statement of a customer, then ask for the name of the client and check if that customer exists
			else if(option==3)
			{
				loop=1;
				MessagetoClient(Client_FD,"Write Customer name or exit to quit");

				while(loop)
				{
					store=MessageFromClient(Client_FD);

					if(strncmp(store,"exit",4)==0)
					{
						flag=1;
            			loop=0;
						break;
					}
					else if(UserExist(store))
					{
						char *customer_name;
						customer_name=(char *)malloc(sizeof(char)*40);
						strcpy(customer_name,store);
						strcat(MS,printMS(Client_FD,customer_name));
						MessagetoClient(Client_FD,strcat(MS,"\n************\n\nWrite your option,\n1. Show Balance of all customer\n2. Show Balance of a particular customer.\n3. Print mini Statement of a customer\nWrite exit to terminate"));
						loop=0;
            			break;
					}
					else
						MessagetoClient(Client_FD,"Customer with this username does not exist. Please enter a valid user or write exit to terminate.");
				}
			}
			else
			{
				MessagetoClient(Client_FD,"Enter valid option...");
				flag = 1;
			}	
		}
	}
}

//this function takes in the username and password entered by the client and returns the type of the user after checking the credentials correctness
char getClientAndAuthorize(int Client_FD,char* username,char *password)
{
    
	MessagetoClient(Client_FD,"Enter Username of account holder: ");       // ask for username of the client
	char *user=MessageFromClient(Client_FD);

	MessagetoClient(Client_FD,"Enter Password: ");                         // ask for password
	char *pass=MessageFromClient(Client_FD);

    decrypt(pass,0xFACA);                         //printf("Decrypted value = %s\n",pass);

	int i=0;
	while(user[i]!='\0' && user[i]!='\n')
	{
		username[i]=user[i];
		i++;
	}

	username[i]='\0';

	i=0;
	while(pass[i]!='\0' && pass[i]!='\n')
	{
		password[i]=pass[i];
		i++;
	}
	password[i]='\0';

    ssize_t read;
    size_t len = 0;

    printf("Checking Credentials....\n");
	
	char * line_authorize = NULL; 

	// check with the login file if the corresponding user exists and if so then check the type of the user
    FILE *fp_authorize=fopen("login_file","r");
	while((read = getline(&line_authorize, &len, fp_authorize)) != -1)
	{
		char *token;
		token = strtok(line_authorize," ");
		
		if(strcmp(token,username)==0)
		{
			token=strtok(NULL," ");
			if(strcmp(token,password)==0)
			{
				token=strtok(NULL," ");
        		switch(token[0])
        		{
          			case 'C' :
            				fclose(fp_authorize);
            				return CUSTOMER;
           			 		break;
          			case 'A':
            				fclose(fp_authorize);
            				return ADMIN;
            				break;
          			case 'P':
            				fclose(fp_authorize);
            				return POLICE;
            				break;
          			default:
            				fclose(fp_authorize);
            		return INVALID_USER;
    			}

      		}
    	}
  	}
    
    fclose(fp_authorize);
	
    if(line_authorize!=NULL)
    free(line_authorize);
	
	return INVALID_USER;
}

int main(int argc,char **argv)
{
	int socket_FD,Client_FD,port_no;                                        // declaring socket file descriptor, client file descriptor and port number of server
	struct sockaddr_in server_address, client_address;                      // server address and client address are of given struct type

	memset((void*)&server_address, 0, sizeof(server_address));              // initializing server_address
	port_no=atoi(argv[1]);                                                  // using arg[1] as port number

	socket_FD=socket(AF_INET, SOCK_STREAM, 0);                              // TCP uses SOCK_STREAM

	server_address.sin_port = htons(port_no);                               //setting port number
	server_address.sin_family = AF_INET;                                    //setting DOMAIN
	server_address.sin_addr.s_addr = INADDR_ANY;                            //INADDR_ANY allows all IPs

	// Error in binding is reflected by a negative value of bind function
	if(bind(socket_FD, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
	    printf("Binding Error.\n");
	    exit(EXIT_FAILURE);
	}
	int reuse=1;
	setsockopt(socket_FD, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));           // works without this as well but used generally
	listen(socket_FD, 5);															// maximum 5 requests can backlog
	int client_size=sizeof(client_address);

	while(1) {
	    
	    memset(&client_address, 0, sizeof(client_address));							// initialize client_address

	    // a negative value in accept function implies an error on accept
	    if((Client_FD = accept(socket_FD, (struct sockaddr*)&client_address, &client_size)) < 0) {
	        printf("Error on accept.\n");
	        exit(EXIT_FAILURE);
	    }

	    switch(fork()) {
	        case 0: {
	            close(socket_FD);
	            ConnectWithClient(Client_FD,0);      // ConnectWithClient function interacts with the client. Second arguement is passed to check the number of invalid credentials entered.
	            exit(EXIT_SUCCESS);
	            break;
	        }
	        case -1:								// terminate if error in fork which is reflected by negative value of fork
	            printf("Error in fork.\n");
	            break;
			default:
	            close(Client_FD);
	    }
	}

}

// function to direct user requests according to the user specification after checking for their access rights by checking their credentials
void ConnectWithClient(int Client_FD,int count)         // count stores the number of times this particular user has enterd wrong credentials. Connection will terminate once count value reaches 3.
{

    char user_type;
	char *username=(char *)malloc(100);               // for username entered by user
	char *password=(char *)malloc(100);				  // for password enterd by user
	
	user_type = getClientAndAuthorize(Client_FD,username,password);  // this checks whether a user with the provided credentials exists and returns the type of the user

	char *msgToSend;
	msgToSend=(char *)malloc(sizeof(char)*70);                     // msgToSend stores the message to sent to the client after all the work is done.
	
	strcpy(msgToSend,"Thanks ");
    strcat(msgToSend,username);

    // directing the client requests according to the values given by the getClientAndAuthorize() function.
	if(user_type == CUSTOMER)                                 // if the client is a customer
	{
		CustomerChoices(Client_FD,username,password);         //go to the access rights provided to customer
		MessagetoClient(Client_FD, msgToSend);				  // send a goodbye message to the client after all the work is done
		shutdown(Client_FD, SHUT_RDWR);						  // finally close connection
	}

	else if(user_type == POLICE)                             // if the client is a Police 
	{
		policeRequests(Client_FD);							//go to the access rights provided to police
		MessagetoClient(Client_FD, msgToSend);              // send a goodbye message to the client after all the work is done
		shutdown(Client_FD, SHUT_RDWR);					   // finally close connection
	}
	else if(user_type == ADMIN)                             // if the client is a admin 
	{
		AdminChoices(Client_FD);							//go to the access rights provided to admin
		MessagetoClient(Client_FD, msgToSend);              // send a goodbye message to the client after all the work is done
    	shutdown(Client_FD, SHUT_RDWR);					   // finally close connection
	}

	// Allow wrong credentials upto 3 times and clode connection afte that
	else if(user_type == INVALID_USER)
	{
		count++;
      	if(count == 3){
      		MessagetoClient(Client_FD,"Unauthorized User, Process Terminated");
        	shutdown(Client_FD, SHUT_RDWR);
      	}

      	// if count not reached 3, then print message but allow the user to still try
      	else{
        	MessagetoClient(Client_FD,"Wrong username or password");
      		ConnectWithClient(Client_FD,count);
    	}
	}

	else
	{
		MessagetoClient(Client_FD,"Wrong username or password");
        shutdown(Client_FD, SHUT_RDWR);
	}	
}

char* MessageFromClient(int Client_FD) {                            // function which recieves from client
    int numberOfPacketsToReceive = 0;
    int n = read(Client_FD, &numberOfPacketsToReceive, sizeof(int));
    if(n <= 0) {
        shutdown(Client_FD, SHUT_WR);               // if size of recieved msg is less than or equal to 0 then close Connection
        return NULL;
    }
    char *receivedMessage = (char*)malloc(numberOfPacketsToReceive*PACKET_SIZE);
    memset(receivedMessage, 0, numberOfPacketsToReceive*PACKET_SIZE);
    char *receivedMessage_first = receivedMessage;
    int i;
    for(i = 0; i < numberOfPacketsToReceive; ++i) {
        read(Client_FD, receivedMessage, PACKET_SIZE);        // copy recieved message socket to string named receivedMessage
        receivedMessage = receivedMessage+PACKET_SIZE;
    }
    return receivedMessage_first;
}

void MessagetoClient(int Client_FD, char *str) {                        // function which sends message to client
    int numberOfPacketsToSend = (strlen(str)-1)/PACKET_SIZE + 1;
    write(Client_FD, &numberOfPacketsToSend, sizeof(int));
    char *sendMessage = (char*)malloc(numberOfPacketsToSend*PACKET_SIZE);
    strcpy(sendMessage, str);
    int i;
    for(i = 0; i < numberOfPacketsToSend; ++i) {
        write(Client_FD, sendMessage, PACKET_SIZE);                   // writting message to socket
        sendMessage += PACKET_SIZE;
    }
}