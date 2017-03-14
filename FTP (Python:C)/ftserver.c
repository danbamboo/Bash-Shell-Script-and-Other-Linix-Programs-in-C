// chatclient.c:: CS_372_400  -- Project 2- File Transfer Client/Server
// Operating Systems:: Oregon State University
// Author: Danny McMurrough
// ftserver.c:  This program acts like a ftp server in the way that it maintains a client
// 	state while also being able to transfer files to the client (using multiple sockets).  The 
// 	server opens up an inital port to wait for client to connect, establishes connection, and handles
// 	additional client reqests as they come in.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "fcntl.h"

#include "arpa/inet.h"
#include <netdb.h>


int createSocket(int);
void errorMsgExit(const char*);
struct sockaddr_in generateStruct(int);
int getAndValidateClientCmd(int, char*);
void executeCommand(int, char*, char*, int);
void sendContents(int,FILE*);


//GLOBAL VARIABLES 
int MAXINQUE = 5;  //How many connections handle at once, here we are only establishing one connection at a time.
int BUFSIZE = 50;  //Amount of data sent thought connection 


//===================
//======= MAIN ======
//===================
int main(int argc, char *argv[])
{
    const struct sockaddr client_address;
    socklen_t addr_size = sizeof client_address;
    int port_main_running;
    int socket_file_d;
    int keep_server_running =1;
    int client_file_descriptor_P;
    char storeCommand[BUFSIZE];
    char clientAdd[1024];  //Referenced for extacting client IP: http://stackoverflow.com/questions/20472072/c-socket-get-ip-adress-from-filedescriptor-returned-from-accept
    char service[20];  
    char* clientFlipLoc;

    //Check for correct number of command line arguments
    if (argc != 2 ) {
        fprintf(stderr,"Usage %s port#\n", argv[0]);
        exit(1);
    }

    port_main_running = atoi(argv[1]);  //Assign string to integer(port#) from user input
    printf("Server open on %i\n\n", port_main_running);
    fflush(stdout);

    socket_file_d = createSocket(port_main_running);  //Generates a new socket



    //Loops to keep server running forever, must be shut down with SIGINT
    while(keep_server_running)
    {
        
        client_file_descriptor_P = accept(socket_file_d, (struct sockaddr *) &client_address, &addr_size);
        memset(clientAdd,0,1024); 

        //extract the address from the client
        //http://www.beej.us/guide/bgnet/output/html/multipage/acceptman.html referenced for getting client address
        getnameinfo(&client_address, sizeof(client_address), clientAdd, sizeof(clientAdd),service, sizeof(service),0);
        clientFlipLoc = strtok(clientAdd,".");
        printf("Connection from %s\n", clientFlipLoc);
        fflush(stdout);

        if(client_file_descriptor_P >0)
        {
            //Check for valid input by calling 'getAndValidateClientCmd' and getting message from client
            //Pass string to hold command so don't have to call twice
            if(getAndValidateClientCmd(client_file_descriptor_P, storeCommand)==0)  //check for -g or -l command from client
            {
                executeCommand(client_file_descriptor_P, storeCommand, clientFlipLoc,port_main_running);
            }
            else  // The -l or -g commands were not found in correct order from client
            {
                printf("Bad command sent by client (%i)", client_file_descriptor_P);
            }     

            memset(storeCommand,0,BUFSIZE); //clear out client command string holder   
        }
        else{
           errorMsgExit("Failed to accept client.\n");
        }
        
    }
    

    return 0;
}
//===================
//===================






//================
//Function: int createSocket(int portNum)
//Description:  Takes input of port number for server connection
//             Generates a new socket, bind, and listen.  Return socket file descriptor.
//================
int createSocket(int portNum)
{
    struct sockaddr_in server_address;
    int newSocketFileDescriptor;
    newSocketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    if(newSocketFileDescriptor==-1)
    {
        errorMsgExit("Error creating socket!\n");
    }


    bzero((char *) &server_address, sizeof(server_address));  //Clears out server_address struct

    server_address = generateStruct(portNum);

    //Here we attempt to bind the structure to the user input port set in structure server_address
    if(bind(newSocketFileDescriptor, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    {
        fprintf(stderr,"Failed to bind port: %i\n",portNum);
        exit(2);
    }

    //Listen, set max queue to global varaible MAXINQUE
    if(listen(newSocketFileDescriptor,MAXINQUE) == -1)
    {
        errorMsgExit("Listen Failed\n");
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
//Function: nt getAndValidateClientCmd(int client_file_descriptor_P, char* storeCommand)
//Description: Takes the Server Socket connection and client input for validation.
//             Send acknowledgement or nack based on command validation.  Returns flag  of result for outside handling. 
//================
int getAndValidateClientCmd(int client_file_descriptor_P, char* storeCommand)
{
    int errorFlag=0;
    char message_invalidCommand[7] = "Error1";  //Error 1 is registered as bad command
    char message_yes[5] = "Yes1";
    char server_message[BUFSIZE];
    memset(server_message,0,BUFSIZE);

    //Get command from client
    recv(client_file_descriptor_P,&server_message,sizeof(server_message),0);
    strcpy(storeCommand, server_message);

    if(strcmp(storeCommand,"-l")==0 || strcmp(storeCommand,"-g")==0 )
    {
        send(client_file_descriptor_P,message_yes, sizeof(message_yes), 0);
    }
    else
    {
        send(client_file_descriptor_P,message_invalidCommand, sizeof(message_invalidCommand), 0);
        errorFlag=1;
    }
    fflush(stdout);
    return errorFlag;  //(0) no error and (1) error
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
//Function: void executeCommand(int client_file_descriptor_P, char* storeCommand, char* flipLoc, int port_main_running)
//Description: Main driver for routing both the -l and -g funcitonality.  
//             Takes the server Port desriptor, command, client location, and server port number as input.
//             Generates files for both -g and -l commands to push to sendContents for data socket transfer
//    Referenced: http://stackoverflow.com/questions/298510/how-to-get-the-current-directory-in-a-c-program
//    -For how to get working directory in C program
//================
void executeCommand(int client_file_descriptor_P, char* storeCommand, char* flipLoc, int port_main_running)
{
    int new_socket_file_d;
    int client_file_descriptor_Q;  //Will be creating new socket by calling 'establishDataConnection'
    int port_data_conn;  //Data port num given by client
    char fileName[30];
    char portNum[10];
    char server_message[BUFSIZE];
    char message_valid[10] = "ValidFile";
    char message_invalidCommand[7] = "Error2";  //Error 1 is registered as bad file
    //Parse out the command sent by client with strtok, save in local car parsedCommand
    FILE *fp = NULL;

    memset(server_message,0,BUFSIZE);
    memset(portNum,0,10);
    memset(fileName,0,30);




    //Recieve DataPort number form server and cast into int variable
    recv(client_file_descriptor_P,&portNum,sizeof(portNum),0);
    
    port_data_conn = atoi(portNum);

    if(strcmp(storeCommand,"-g")==0)  //If -g is command, get file name out of data
    {
        //Get file name from client
        recv(client_file_descriptor_P,&fileName,sizeof(fileName),0);

        printf("File \"%s\" requested on port %i.\n", fileName, port_data_conn);
        fflush(stdout);
       
        int fd = open(fileName, O_RDWR);  //Attempt to open file
        fp = fdopen(fd,"w+");
        
        if(fd==-1)  //File couldn't open.  Send error message on data port.
        {
            printf("File not found.  Sending error message to %s:%i\n\n", flipLoc,port_main_running);
            fflush(stdout);
            send(client_file_descriptor_P,message_invalidCommand, sizeof(message_invalidCommand), 0);
            close(client_file_descriptor_P);
            
        }

        else{
            //Send validation message on server port to verify to open data port
        send(client_file_descriptor_P,message_valid, sizeof(message_valid), 0);
        
        printf("Sending \"%s\" %s:%i\n\n", fileName, flipLoc,port_data_conn);

        new_socket_file_d = createSocket(port_data_conn);  //Create data socket
        
        client_file_descriptor_Q = accept(new_socket_file_d,NULL,NULL);
        sendContents(client_file_descriptor_Q,fp);  //Pass file to sendContents function for trasfer
        close(client_file_descriptor_P);  //close server socket before exiting
        }
    }

    else  //Command must be -l since we already validated it was either -g or -l
    {

        new_socket_file_d = createSocket(port_data_conn);  //make data socket
        client_file_descriptor_Q = accept(new_socket_file_d,NULL,NULL);

        printf("List directory requested on port %i\n", port_data_conn);
        system("/bin/ls > temp.txt");  //place ls output into temp.txt, is removed after sending
        fp = fopen("temp.txt","ab+");

        printf("Sending directory contents to %s:%i\n\n", flipLoc,port_data_conn);
        fflush(stdout);
        sendContents(client_file_descriptor_Q,fp);
        remove("temp.txt");  //Remove Temp file
        close(client_file_descriptor_P);
    }
}


//================
//Function: void sendContents(int client_file_descriptor_Q,FILE* fp)
//Description: Send full file contents of files over data port, both input to function.
//              Checks for EOF flag to stop sending, then send end of response flag that
//              Client is waiting for to know comms are done.  Close data port at end.  
//================     
void sendContents(int client_file_descriptor_Q,FILE* fp)
{
    char endOfResponseFlag[6] = "-END-";
    int sending=1;
    char buffer[BUFSIZE];
    int lup;
    char get_char;
    ssize_t send_status;
    
    
    memset(buffer,0,BUFSIZE);
    do
    {  
        for(lup=0; lup < BUFSIZE-1; lup++)
        {
            get_char = fgetc(fp);  //Get each character at a time to check for EOF
            
            if(get_char==EOF)  //if EOR, set sending to 0 to break out of do/while loop
            {
                sending=0;
                break;
            }
            else
            {
                buffer[lup]=get_char;  //Place character in buffer until BUFSIZE-1
            }
        }
        
            fflush(stdout);
            //Send full buffer to client
            send_status=send(client_file_descriptor_Q,buffer,sizeof(buffer),0);
            if(send_status <= 0)
            {
                fprintf(stderr,"Error sending.\n");
            }
            memset(buffer,0,BUFSIZE);  //Clear contents of buffer
        
        
        
    }while(sending==1);
    //Send end of response flag for client to check
    send_status=send(client_file_descriptor_Q,endOfResponseFlag,sizeof(endOfResponseFlag),0);
    fclose(fp);  //Close file.
    close(client_file_descriptor_Q);  //Done with data port, close!

    return;
}





