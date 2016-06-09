// otp_dec.c:: CS_344_400  -- Program 4 - one-time-pad encryption using networks
// Operating Systems:: Oregon State University
// Author: Danny McMurrough
// Program:  This program acts like a client to send input to a server 
// We send a encoded and key file to server, and recieve back an decoded
// strings.  This gets output to stdout.


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


void settings();
void errorMsgExit(const char*);
int createSocket();
int validateChar(char);
struct sockaddr_in generateStruct(int);
void checkCorrectServer(int);
int establishNewConn(int);
long writeFile(int,const char*);
void readAndOutputDec(int);
void sendFileSizeStatus(int,int);

int BUFSIZE = 32;
int MAXINQUE = 5;
char STATUSV[30];

int main(int argc, char *argv[])
{

	int socket_file_d, port_main_running;
    struct sockaddr_in server_address;
    int new_socket;
    long ciphertext_size, key_file_size;

    //Screen for invalid number of arguments
    if (argc < 4) {
        fprintf(stderr,"Usage %s ciphertext key port\n",argv[0]);
        exit(1);
    }
    
    //LABEL FILES
    const char* ciphertext = argv[1];
    const char* key_file = argv[2];
    
    //ESTABLISH INITIAL CONNECTION
    port_main_running = atoi(argv[3]);  //Assign string to integer from user input
    socket_file_d = createSocket();  //Generates a new socket
    
    bzero((char *) &server_address, sizeof(server_address));  //Clears out server_address struct
    server_address = generateStruct(port_main_running);  //Assigns struct variable to specified port
    
    if(connect(socket_file_d, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    {
       fprintf(stderr,"Failed to connect to port: %i\n",port_main_running);
        exit(2);
    }

    checkCorrectServer(socket_file_d);
    

//////   printf("Connected to main server port: %i\n",port_main_running);
    
    //ESTABLISH NEW CONNECTION
    new_socket = establishNewConn(socket_file_d);
    
    //WRITE Cipher text FILE
    ciphertext_size = writeFile(new_socket,ciphertext);
//////    printf("Ciphertext file size: %li\n",ciphertext_size);
    //WRITE KEY FILE
    key_file_size = writeFile(new_socket,key_file);
//////    printf("Key file size: %li\n",key_file_size);
    
    if(key_file_size < ciphertext_size)
    {
    	sendFileSizeStatus(1,new_socket);
        close(new_socket);
        errorMsgExit("Client exit: Key file size must be as large as plaintext.\n");
    }
    
    sendFileSizeStatus(0,new_socket);


    readAndOutputDec(new_socket);


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
//Function: int validateChar(char current_char)
//Description: Ensure a charater is A-Z and space (65-90) and 32:returns 1
// Also checks for newline: returns 2
// If not valid return 0
//================
int validateChar(char current_char)
{
    int valid=0;
 
        if((current_char >= 65 && current_char <=90) || current_char ==32 )
        {
            valid=1;
        }
        else if(current_char=='\n')
        {
            valid=2;
        }

    return valid;
}


//================
//Function: struct sockaddr_in generateStruct(int port)
//Description: Generates structure that accepts any address.  Also,
// the htons will assign the interger port to the correct format based
// on a given architecture 
//================
struct sockaddr_in generateStruct(int port)
{
    struct sockaddr_in generate_struct;
    struct hostent *server;
    
  if( (server = gethostbyname("localhost"))==NULL)
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
//Function: void checkCorrectServer(int localFileD)
//Description: Takes current connection as parameter 
// Does a handshaking between client/server to ensure that the correct
// server (opt_dec_d) is the one we are trying to connect to.  If not, we will
// get message (No), else good connection.
//================
void checkCorrectServer(int localFileD)
{
    char validate_connection[20]= "Conn to opt_dec_d";
    char server_message[BUFSIZE];

    send(localFileD,validate_connection, sizeof(validate_connection), 0);
    recv(localFileD,&server_message,sizeof(server_message),0);
    if(strcmp(server_message,"No")==0)
    {
        errorMsgExit("Exit: Wrong Server Connection\n");
    }
    
}


//================
//Function: int establishNewConn(int main_file_d)
//Description: Handshaking for new port connection.  Will recieve new connection port
// number from server, attempt to connection, and send back status to server.  Returns
// file descriptor of new connection.
//================
int establishNewConn(int main_file_d)
{
    ssize_t readit;
    char buffer[10];
    char connection_status[20];
    bzero(buffer,10);
    int newPort;
    int newSocket = 0;
    int notestablished=1;
    struct sockaddr_in server_address;
    
    while(notestablished)  //Loop through until valid connection is made 
    {
        
        readit = recv(main_file_d,&buffer,sizeof(buffer),0);
        if (readit <= 0)
        {
            errorMsgExit("Unable to read message.\n\n");
        }
        
        else
        {
            memset(connection_status,0,20);
            
            newPort = atoi(buffer);
//////         printf("Attempt to connect to port: %i\n",newPort);
            newSocket = createSocket();  //Generates a new socket
            bzero((char *) &server_address, sizeof(server_address));  //Clears out server_address struct
            server_address = generateStruct(newPort);  //Assigns struct variable to specified port
            
            
            if(connect(newSocket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
            {
                strcpy(connection_status, "11Bad");  //Bad connection
                send(main_file_d,connection_status, sizeof(connection_status), 0);
            }
            
            else
            {
                strcpy(connection_status, "22Good");  //Good connection
//////          printf("New connection established on port: %i\n",newPort);
                send(main_file_d,connection_status, sizeof(connection_status), 0);
                notestablished=0;
                close(main_file_d);
            }
        }
    }
    
    return newSocket;
}


//================
//Function: long writeFile(int new_conn_file_d,const char* fileName)
//Description: General function to open file and send contents to server.  Keeps track of size
// in long variable.  Checks for valid characters, and initally removes newlines.  Will append newline
// to the end of file.  Once finished sending entire file contents, send key to server (22done22)
// to signify we are done sending.  
//================
long writeFile(int new_conn_file_d,const char* fileName)
{
    
    FILE *fptr;
    int sending=1;
    char buffer[BUFSIZE];
    int lup, found_char;
    long file_size = 0;
    char get_char;
    int valid_char;
    int new_line_found;
    ssize_t send_status;
    
    fptr = fopen(fileName,"r");
    if(fptr == NULL)
    {
        errorMsgExit("Cannot open file.\n");
    }
    memset(buffer,0,BUFSIZE);   //Clear buffer
    do
    {
        found_char=0;
        new_line_found=0;  //Initally set found_char and new line found to 0
        
        for(lup=0; lup < BUFSIZE-2; lup++)  //Allow two spaces for buffer (/n and '0')
        {
            get_char = fgetc(fptr);  //Retrieve sequential charater from file
            valid_char = validateChar(get_char);  //Make sure valid character
            if(valid_char==0 && get_char !=EOF)//If character is invalid (and not EOF), error
            {
                strcpy(buffer,"invalid");
                send_status=send(new_conn_file_d,buffer,sizeof(buffer),0);
                close(new_conn_file_d);
                errorMsgExit("Client exit, invalid character.\n");
            }

            if(get_char==EOF) 
            {

                buffer[lup-new_line_found]='\n';  //Add newline to end.
                file_size = ftell(fptr);  //Get current file size
                sending=0;
                break;
            }
            else if(get_char=='\n')
            {
 //////               printf("New line found in lup: %i\n",lup);
                new_line_found++;
            }
            else
            {
                found_char++;
                buffer[lup-new_line_found]=get_char;
            }
        }
        
        if(found_char >0)
        {
 //////           printf("SEND: %s\n",buffer);
            send_status=send(new_conn_file_d,buffer,sizeof(buffer),0);
            if(send_status <= 0)
            {
                fprintf(stderr,"Error sending.\n");
            }
            memset(buffer,0,BUFSIZE);
        }
        
        
        
    }while(sending==1);
    strcpy(buffer,"22done22");  //Key for server to signify we are done sending.
    
    send_status=send(new_conn_file_d,buffer,sizeof(buffer),0);  //send key
    fclose(fptr);
    return file_size;
}


//================
//Function: void readAndOutputDec(int new_socket)
//Description: Get decoded output from the server, and direct to stdout.
// Will recieve until recieves key from server (33done33) signifying its done sending.
// Server will send error message if invalid character is found.  Close connection afterwords, finished.
//================
void readAndOutputDec(int new_socket)
{

    int reading=1;
    char buffer[BUFSIZE];
    ssize_t readit;
    int do_twice=0;
    
    
     while(reading==1)
        {
            readit = recv(new_socket,&buffer,sizeof(buffer),0);
            if(strcmp(buffer,"33done33")==0)
            {
                reading=0;
                break;  //New addition
            }
            if(strcmp(buffer,"invalid")==0) //Invlaid character found by server
            {
                errorMsgExit("Exiting: Invalid character found by server.\n");
            }
            else
            {
                    printf("%s",buffer);  //Print each string to stdout 
            }
        }
        reading=1;

    
        close(new_socket);
}

//================
//Function: void sendFileSizeStatus(int status_of_size, int new_socket)
//Description: Communicate with server, give status of file sizes.  If the key file is
// smaller than the encrypted file, tell server so both can exit, else, send good signal
//================
void sendFileSizeStatus(int status_of_size, int new_socket)
{

char buffer[BUFSIZE];

	if(status_of_size==1) //1 represents a smaller key file size, send bad to server
	{
		strcpy(buffer,"bad");
	}
	else
	{
		strcpy(buffer,"good");  //Good file sizes, send good key
	}

	send(new_socket,buffer,sizeof(buffer),0);

}



