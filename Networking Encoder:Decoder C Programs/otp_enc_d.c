// otp_enc_d.c:: CS_344_400  -- Program 4 - one-time-pad encryption using networks
// Operating Systems:: Oregon State University
// Author: Danny McMurrough
// Program:  This program acts like a server to recieve input from a client 
// key and encoded files.  This program will do decryption on these two inputs,
// storing them in temp files, and passing the information back to the client.


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
#include <time.h>




void errorMsgExit(const char*);
int createSocket();
struct sockaddr_in generateStruct(int);
void runChild(int);
void settings();
int checkStatus(int);
FILE* readClientFiles(int);
FILE* makeMagic(FILE*,FILE*);
void checkCorrectClient(int);
int validateChar(char);
void sendEncodedToClient(FILE*,int);
void confirmSizes(int);

int BUFSIZE = 32;
int MAXINQUE = 5;
char STATUSV[30];

int main(int argc, char *argv[])
{
    srand(time(NULL));  //Seed random#generator

    int port_main_running;
    int socket_file_d;
    int check_bind_status;
    int keep_server_running =1;
    int check_running_processes=0;
    int client_file_descriptor,pid,status;
    struct sockaddr_in server_address;
    
     //Screen for invalid number of arguments
    if (argc < 2) {
        fprintf(stderr,"Usage %s port#\n", argv[0]);
        exit(1);
    }
    
    port_main_running = atoi(argv[1]);  //Assign string to integer from user input
    
    socket_file_d = createSocket();  //Generates a new socket
    
    bzero((char *) &server_address, sizeof(server_address));  //Clears out server_address struct
    
    server_address = generateStruct(port_main_running);  //Assigns struct variable to specified port
    
    //Here we attempt to bind the structure to the user input port set in structure server_address
    if(bind(socket_file_d, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    {
        fprintf(stderr,"Failed to bind port: %i\n",port_main_running);
        exit(2);
    }
    
    
    //Listen, set max queue to global varaible MAXINQUE
    if(listen(socket_file_d,MAXINQUE) == -1)
    {
        errorMsgExit("Listen Failed\n");
    }
    
    while(keep_server_running)
    {
    	 pid = waitpid(-1, &status, WNOHANG);
//////        printf("Server looping to accet\n");
        client_file_descriptor = accept(socket_file_d,NULL,NULL);
        if(client_file_descriptor >0)
        {
            runChild(client_file_descriptor);
        }
        else{
           errorMsgExit("Failed to accept client.\n");
        }
        
    }
    
    
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
struct sockaddr_in generateStruct(int port)
{
    struct sockaddr_in generate_struct;
    
    
    
    generate_struct.sin_family = AF_INET;
    generate_struct.sin_port = htons(port);
    generate_struct.sin_addr.s_addr = INADDR_ANY;
    
    return generate_struct;
}


//================
//Function: void runChild(int fileD)
//Description: Here we fork off new client connection, and test for a valid 
// client.  First we establish a new port of these two to connect on, and 
// assume that the server can only connect to one client at a time.   We validate
// good new port through handshaking methods, then call functions to read client
// files, more validation on file sizes, and send encrypted file to client. 
//================
void runChild(int fileD)
{
    pid_t pid=fork();  //-----FORK------ (Start child process)
    int status=0;
    int new_socket,signal;
    int localFileD=fileD;
    struct sockaddr_in new_server_address;
    int good_connection=1;
    int client_file_descriptor;
    FILE* temp_file;
   	//Error
    if(pid < 0)
    {
        errorMsgExit("Error forking");
    }
    
    //Parent process
    else if(pid >0)
    {
        signal = checkStatus(status);  //Grab return value and print it to stdout
        //printf("background pid %i is done: exit value %i\n",pid,signal);
        fflush(stdout);
        return;
    }
    
    //-------------
    //Child Process
    //-------------
    else
    {
        checkCorrectClient(localFileD);  //Handshaking, make sure only otp_enc can connect
        while(good_connection)
        {
            int randport = rand() % 15000 + 50000; //range of 50,000 - 65,000
            char newPort[10];
            char buffer[256];
            ssize_t readit;
            
            pid_t getPID = getpid();
//            printf("Server has created new process: %i\n",getPID);
            sprintf(newPort, "%i",randport);
            
            new_socket = createSocket();  //Generates a new socket
            setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));  //Allow program to use the same port
            bzero((char *) &new_server_address, sizeof(new_server_address));  //Clears out server_address struct
            new_server_address = generateStruct(randport);  //Assigns struct variable to specified port
            
            if(bind(new_socket,(struct sockaddr *) &new_server_address, sizeof(new_server_address)) < 0)
            {
//                fprintf(stderr,"Server Error binding, trying different port.\n");
            }
            else
            {
                //Send of newPort number so Client can know what to connect to.
                send(localFileD,newPort, sizeof(newPort), 0);
                memset(buffer,0,256);
                
                listen(new_socket, 1);  //Listen for client to see if they connected (handshake)
                
                readit = recv(localFileD,&buffer,sizeof(buffer),0);
                
                if(strcmp(buffer,"22Good")==0)  //Check for good client connection
                {
//////                   printf("\tSuccessfully connected to new port: %s\n",newPort);
                    client_file_descriptor = accept(new_socket,NULL,NULL);
                    good_connection=0;
                }
                else
                {
                    fprintf(stderr,"Port failed, trying different port connection.\n");
                }
            }
            
        }
        
        //Read Both Files, returns encoded file
        temp_file = readClientFiles(client_file_descriptor);

        confirmSizes(client_file_descriptor); //Check with client to make sure sizes of files correct
        
        sendEncodedToClient(temp_file,client_file_descriptor); //Pass encoded file and connection
        
        close(client_file_descriptor);  //Close connectoin

        exit(0);
    }
    
    
}

//================
//Function: int checkStatus(int)
//Description: Get status information and store in global status variable.
// Using MACROS, the status from a waitpid funciton is passed in, and the
// return variable is then passed back via int.
//================
int checkStatus(int status)
{
    //Used the following site for reference: http://www.tutorialspoint.com/unix_system_calls/waitpid.htm
    
    int signal;
    if(WIFSIGNALED(status))  //Child process was terminated by a signal.
    {
        signal=WTERMSIG(status);
        memset(STATUSV,0,30);
        sprintf(STATUSV, "terminated by signal %i\n",signal);
        //printf("terminated by signal %d\n", WSTOPSIG(status));
        
    }
    else if(WIFEXITED(status))
    {
        signal=WEXITSTATUS(status); //Child process was exited.
        memset(STATUSV,0,30);
        sprintf(STATUSV, "exit value %d\n",signal);
        // printf("exited, status=%d\n", WEXITSTATUS(status));
    }
    return signal;
    
}

//================
//Function: FILE* readClientFiles(int clinet_file_d)
//Description: Takes as an argument current conneciton.  
// Recieves both key file and plain text file from client, validates, stores in temp files.
// Then, passes those file to function make magic that encrypts and returns a temp file descriptor.
// This funciton will return a file descriptor back to runChild of the encrypted temp file
// Note that makeMagic will close temp files. 
//================
FILE* readClientFiles(int clinet_file_d)
{
    int reading=1;
    char buffer[BUFSIZE];
    ssize_t readit;
    FILE *temp_plain_text= tmpfile();
    FILE *temp_key= tmpfile();
    FILE *temp_encoded;
    int do_twice=0;
    
    
    for(do_twice=0; do_twice < 2; do_twice++) //Loop to read until a "key" is sent from client.
    {
        while(reading==1)
        {
            readit = recv(clinet_file_d,&buffer,sizeof(buffer),0);
            if(strcmp(buffer,"22done22")==0) //The "key" sent by the client to signify done sending.
            {
                reading=0;
                break;  
            }
            if(strcmp(buffer,"invalid")==0)
            {
                errorMsgExit("Server exiting: Invalid character found by client.\n");
            }
            else
            {
                if(do_twice==0)
                {
//////                    printf("Plain Text: %s\n",buffer);
                    fprintf(temp_plain_text,"%s",buffer);
                }
                else
                {
//////                    printf("Key Text: %s\n",buffer);
                    fprintf(temp_key,"%s",buffer);
                }
            }
        }
        reading=1;
    }
    
    temp_encoded = makeMagic(temp_plain_text,temp_key);
    
    return temp_encoded;
}



//================
//Function: FILE* makeMagic(FILE* temp_cipher_text,FILE* temp_key)
//Description: Takes as an argument key and plaintext files previously recieved from client.  
// Encodes based on key/cipher one character at a time and puts encoded conversion
// into a temp files.  It will return this new temp encoded file.
//================
FILE* makeMagic(FILE* temp_plain_text,FILE* temp_key)
{
    char plain_char, key_char;
    FILE *temp_encoded= tmpfile();
    int file_size = (int)ftell(temp_plain_text);
    rewind(temp_plain_text);
    rewind(temp_key);
    int lup;
    char char_plain,char_key, new_char;
    int int_plain, int_key;
    int do_twice=0;
    int new_char_ascii_rep;
    
//////   printf("FILE SIZE: %i\n",file_size);
    
    for(lup=0;lup<file_size;lup++)
    {
        char_plain=fgetc(temp_plain_text);
        char_key=fgetc(temp_key);
        
        
        if(char_plain=='\n' || char_key=='\n')
        {
            //do nothing
        }
        
        else
        {
            if(char_plain==' ')  //A=0, B=1,...,Z=25, (space)=26
            {
                int_plain=26;
            }
            else
            {
                int_plain= char_plain - 65;
            }
            if(char_key==' ')  //A=0, B=1,...,Z=25, (space)=26
            {
                int_key=26;
            }
            else
            {
                int_key= char_key - 65;
            }
            new_char_ascii_rep = (int_key + int_plain) % 27;  //add together and mod by 27
            
            if(new_char_ascii_rep==26)  //If 26, encoded char is space
            {
                new_char=' ';
            }
            else  //else new character is respective capitol letter
            {
                new_char= new_char_ascii_rep +65;
            }
            fputc(new_char,temp_encoded);
        }
    }
    
    fclose(temp_plain_text);
    fclose(temp_key);  //close both temp files, we are done with them
    
    return temp_encoded;
}



//================
//Function: void checkCorrectClient(int localFileD)
//Description: Takes current connection as parameter 
// Does a handshaking between client/server to ensure that the correct
// client (opt_enc) is the one we are trying to connect to.  If not, send back
// specific exit message (No), else verify handshake send back (Yes).  
//================
void checkCorrectClient(int localFileD)
{
    
    char buffer[BUFSIZE];
    char message_yes[5] = "Yes";
    char message_no[5] = "No";
    
    recv(localFileD,&buffer,sizeof(buffer),0);
    if(strcmp(buffer,"Conn to opt_enc_d")==0)
    {
        send(localFileD,message_yes, sizeof(message_yes), 0);
    }
    else
    {
        send(localFileD,message_no, sizeof(message_no), 0);
        errorMsgExit("Exit: Wrong Client Connection\n");
    }
    
}

//================
//Function: void sendDecodedToClient(FILE* temp_enc, int new_conn_file_d)
//Description: Takes server created decoded file and current connection.
// We send the entire contents of this file to the client.
//================
void sendEncodedToClient(FILE* temp_enc, int new_conn_file_d)
{
    int sending=1;
    char buffer[BUFSIZE];
    int lup, found_char;
    long file_size = 0;
    char get_char;
    int valid_char;
    int new_line_found;
    ssize_t send_status;
    rewind(temp_enc);
    
    memset(buffer,0,BUFSIZE);
    do
    {
        found_char=0;
        new_line_found=0;
        //strcpy(buffer, "HELLO MY DEAR FRIEND");
        
        for(lup=0; lup < BUFSIZE-2; lup++)
        {
            get_char = fgetc(temp_enc);
            valid_char = validateChar(get_char);
            if(valid_char==0 && get_char !=EOF)
            {
                strcpy(buffer,"invalid");
                send_status=send(new_conn_file_d,buffer,sizeof(buffer),0);
                close(new_conn_file_d);
                //errorMsgExit("Invalid Character.\n");
            }
            
            if(get_char==EOF)
            {
                
                buffer[lup-new_line_found]='\n';
                file_size = ftell(temp_enc);
                sending=0;
                break;
            }
            else if(get_char=='\n')
            {
//////                printf("New line found in lup: %i\n",lup);
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
//////            printf("SEND: %s\n",buffer);
            send_status=send(new_conn_file_d,buffer,sizeof(buffer),0);
            if(send_status <= 0)
            {
                fprintf(stderr,"Error sending.\n");
            }
            memset(buffer,0,BUFSIZE);
        }
        
        
        
    }while(sending==1);
    strcpy(buffer,"33done33");
    
    send_status=send(new_conn_file_d,buffer,sizeof(buffer),0);
    fclose(temp_enc);  //Close temp encoded file.
    
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
//Function: void confirmSizes(int new_port)
//Description: A handshaking function with the client, with determinant from
// the client.  If the client notices improper key size file, it will communicate
// that to the server here with a key word (bad).
//================
void confirmSizes(int new_port)
{

	char buffer[BUFSIZE];

	recv(new_port,&buffer,sizeof(buffer),0);

	if(strcmp(buffer,"bad")==0)
	{
		close(new_port);
		errorMsgExit("Server exited due to inproper size key file.\n");
	}


}






