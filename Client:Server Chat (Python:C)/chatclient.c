// chatclient.c:: CS_372_400  -- Project 1- TCP Socket Connection Client Side
// Operating Systems:: Oregon State University
// Author: Danny McMurrough
// Program:  This program acts like a client to send input to a server on a TCP connection.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "sys/time.h"
#include "fcntl.h"
#include "arpa/inet.h"
#include <netdb.h>






int createSocket();
struct sockaddr_in generateStruct(int, char[]);
int establishNewConn(int,char[]);
void messenger(int);
int getAndCheckInput(char[], char[]);
void prependEndToMessage(char [], const char []);
void getStdinput(char [], int );

int BUFSIZE = 1024;  //Max buffer for user message length and socket send/rec as well.
// int MSGSIZE = 1024;  Another option is to have send/rec buffer seperate from Message Size.

//================
//Function: main()
//Description: Error checks for valid command line arguemnts and stores them.  Call function to create new socket,
//              then passes the file discriptor from the newly created socket to the messger driver function.
//================
int main(int argc, char *argv[])
{
    int port_connect;
    int socket_file_d;
    char hostName[30];
    
     //Screen for invalid number of arguments
    if (argc != 3) {
        fprintf(stderr,"Usage %s hostname port\n",argv[0]);
        exit(1);
    }
    
    port_connect = atoi(argv[2]);  //Assign string to integer from user input for port#
    strcpy(hostName,argv[1]);
    //ESTABLISH INITIAL CONNECTION
    socket_file_d = establishNewConn(port_connect,hostName);
  

    messenger(socket_file_d);

    return 0;
}


//================
//Function: void settings()
//Description: General setting here.  Don't allow buffer to generate for stdout.
//================
void settings()
{
    setbuf(stdout, NULL);  //Allows for the use of printf because now there is no buffer allowed.
}




//================
//Function: void errorMsgExit(const char* msg)
//Description: Takes as a parameter a string to output to stderr, an 
// error message. Will exit with 1 (error) after printing error message.
//================
void errorMsgExit(const char* msg)
{
    fprintf(stderr,"%s\n",msg);
    exit(1);
}



//================
//Function: int createSocket()
//Description: Generates a new socket, returns a file descpritor
//================
int createSocket()
{
    int newSocketFileDescriptor;
    newSocketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if(newSocketFileDescriptor==-1)
    {
        errorMsgExit("Error creating socket!\n");
    }
    return newSocketFileDescriptor;
}

//================
//Function: struct sockaddr_in generateStruct(int port)
//Description: Generates structure that accepts any address.  Also,
// the htons will assign the interger port to the correct format based
// on a given architecture 
//================
struct sockaddr_in generateStruct(int port, char hostName[])
{
    struct sockaddr_in generate_struct;
    struct hostent *server;
    
  if( (server = gethostbyname(hostName))==NULL)
    {
        errorMsgExit("No localhost var.\n");
    }

    generate_struct.sin_family = AF_INET;
    generate_struct.sin_port = htons(port);
    memcpy(&generate_struct.sin_addr, server->h_addr,server->h_length);
    //generate_struct.sin_addr.s_addr = INADDR_ANY;
    
    return generate_struct;
}

//================
//Function: establishNewConn(int port_connect, char hostName[])
//Description: Using the host name and port number to connect to,
//              this function will create a socket and connect to the server.
//              The file discriptor for the socket is returned.  
//================
int establishNewConn(int port_connect, char hostName[])
{
    int socket_file_d;
    struct sockaddr_in server_address;

    socket_file_d = createSocket();  //Generates a new socket
    
    bzero((char *) &server_address, sizeof(server_address));  //Clears out server_address struct
    server_address = generateStruct(port_connect, hostName);  //Assigns struct variable to specified port
    
    
    if(connect(socket_file_d, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    {
       fprintf(stderr,"Failed to connect to port: %i\n",port_connect);
        exit(2);
    }

    return socket_file_d;


}

//================
//Function: messenger(int socket_file_d)
//Description: Main driver function.  Creates client handle.  Sends and recieves to/from client.
//              Checks for quit message to close connection.  
//================
void messenger(int socket_file_d)
{
    char userInput[BUFSIZE];
    char contactInput[BUFSIZE];
    char contactHandle[16];
    int flagValidInput=0;

    char inputTypeHandle[7]= "handle"; 
    char endOfResponseFlag[6] = "-END-";
    char inputConnEst[8] = "ConnEst";
    char inputClientCheck[12] = "ClientCheck";

    //Gets user input handle, checks for valid input and loops until valid.
    do{
        flagValidInput = getAndCheckInput(userInput, inputTypeHandle);
        
        if(flagValidInput==0)
        {
            strcat(userInput,endOfResponseFlag); //send user handle;
            //printf("%s",userInput);
            send(socket_file_d,userInput, sizeof(userInput), 0);  //sizeof(userInput)
            memset(contactHandle,0,16);
            recv(socket_file_d,&contactHandle,sizeof(contactHandle),0);
            printf("Connection Established to: %s\n", contactHandle);
            break;
        }

    }while(flagValidInput==1);

        
    while(1){
        memset(userInput,0,BUFSIZE);  //Clear out message
       
        //Gets input to send to client
        flagValidInput = getAndCheckInput(userInput, inputConnEst);

        //Runs if the quit message is input from this client user.
        if(flagValidInput==2)
        {
            strcat(userInput,endOfResponseFlag); //send user handle;
            send(socket_file_d,userInput, sizeof(userInput), 0);  //sizeof(userInput)
            printf("You have left the chat.\n");
            break;
        }

        //Send and recieve messages from server.  Check for server quit message.
        if(flagValidInput==0)
        {
            strcat(userInput,endOfResponseFlag); 
            send(socket_file_d,userInput, sizeof(userInput), 0);  //send user message;
            memset(contactInput,0,BUFSIZE);
            recv(socket_file_d,&contactInput,sizeof(contactInput),0);
            //Check for server message quit command
            flagValidInput= getAndCheckInput(contactInput, inputClientCheck);
            
            if(flagValidInput==1)
            {
                printf("%s has closed connection.\n", contactHandle);
                break;
            }

            printf("%s: %s\n",contactHandle,contactInput );
        }

        }
       

    close(socket_file_d);  //Close socket connection
}

//================
//Function: getAndCheckInput(char userInput[], char inputType[])
//Description: This function is used to input, validate, and check input depending on the type.
//              The userInput represents input form client or server.  The input type is used to determine
//              which portion of the following code should be utalized.  A int flag is returned to signify outcome.
//================
int getAndCheckInput(char userInput[], char inputType[])
{
    char inputTypeHandle[7] = "handle";
    char inputConnEst[8] = "ConnEst";
    char inputClientCheck[12] = "ClientCheck";
    int flag=0;
    int checkInputLength;
    int ch;



    //Input and check for valid Client Handle- set flag accordingly
    if(strcmp(inputType,inputTypeHandle) ==0)
    {
        printf("Input Handle: ");
        scanf("%s",userInput); 
        while((ch= getchar()) != '\n');  //Clears buffer Referenced-- http://stackoverflow.com/questions/21045208/how-to-clean-buffer-before-scanf
        checkInputLength = strlen(userInput);
        //Clinet Handle must be between 1-10 characters, if not, flag is set.
        if(checkInputLength > 10 || checkInputLength < 1)
        {
            printf("\nInvalid Entry.  Plese enter a handle of length 1-10 characters one word.\n\n");
            flag=1;
        }
    }

    
    //Input and check client message.  Checks for the \quit symbol as well.
    else if (strcmp(inputType,inputConnEst) ==0)
    {
        printf("> ");

        getStdinput(userInput,1018);

        checkInputLength = strlen(userInput);
        if(strcmp("\\quit",userInput)==0)
        {
            flag=2;

        }
        else if(checkInputLength > 1018 || checkInputLength < 1)  //Max input of 1018 b/c of End Segment
        {
            printf("\nInvalid Entry.  Plese enter a handle of length 1-1018 characters.\n\n");
            flag=1;
        }
    }

    //Checks for server input for the \quit command
    else if (strcmp(inputType,inputClientCheck) ==0)
    {
        if(strcmp("\\quit",userInput)==0)
        {
            flag=1;
        }
    }

return flag;

}

//================
//Function: getStdinput(char userInput[], int size)
//Description: Grabs one character at a time, up to the size-1 for more accurate
//              string storage.  The userInput will store the user input. Stops with \n character.
//================
void getStdinput(char userInput[], int size)
{
    int index =0;
    char c;
    memset(userInput,0,BUFSIZE);
    while(1)
    {

        c = getchar();
        if(c == '\n' || index == size-1)
        {
            break;
        }

        userInput[index] = c;
        index = index + 1;
    }

}






