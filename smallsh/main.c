// Program 3:: CS_344_400  -- smallsh
// Operating Systems:: Oregon State University
// Author: Danny McMurrough
// Program Description: This program is intended to replicate some functionalities
// of a standard shell, but on a smaller scale.  It has three built in functions
// (exit,status,cd).  It allows for redirection and background processes.
// Core concepts:  I/O redirections, processes, signals.
//  ENJOY!

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>


int MAXCHARINPUT=2048;  //Lets user input max of 2048, and allows for one spot to use '\0' null terminate to be instered to end string.
int MAXARGS=512+1;  //Max of 512 arguments + 1 for NULL
int MAXCHARPERARG=30;  //Max length of 30 per argument.
char STATUSV[30];

void settings();
void startSmallSh();
char** allocateStringArr();
void userInputAndValidate(char[], char[], int);
int parseInput(char*,char**);
void clearContents(char*,char**);
void inputHandler(int,char**, int*);
void builtIncd(char**,int);
char** makeExeArr_findSpeialChars(char**, int,int*,int*,char*);
void execIt(char**,int,char*);
void sigHand(int);
void runChild(char**,int,struct sigaction,char*);
void catchInt(int);
int checkStatus(int);
void callStatus();

//===================
//======= MAIN ======
//===================
int main(int argc, const char * argv[])
{
    settings();  //Any presets needed
    startSmallSh();  //Main driver function for shell functionality
    
    return 0;
}
//===================
//===================




//================
//Function: settings()
//Description: Only settings is to not allow stdout to sit unwanted in buffer.
//================
void settings()
{
    setbuf(stdout, NULL);  //Allows for the use of printf because now there is no buffer allowed.
}



//================
//Function: startSmallSh()
//Description: Only settings is to not allow stdout to sit unwanted in buffer.
//================
void startSmallSh()
{
    int actualArgs=0;  //Tracks # of arguments, clear string arr based on this(actual).
    int callExit = 0;

    //Title of shell (smallsh)
    printf("\n");
    printf("-------smallsh-------\n");
    fflush(stdout);
    
    //Set up char* and char** for memory based on max variables (uses function allocateStringArr)
    char getUserInput[MAXCHARINPUT];
    char copyGetUserInput[MAXCHARINPUT];
    char** eachArgument= allocateStringArr(MAXARGS,MAXCHARPERARG);

    
    //While loop should continue to drive shell until user enters exit
    do{
        userInputAndValidate(getUserInput,copyGetUserInput, MAXCHARINPUT);   //Check for valid input
        actualArgs=parseInput(getUserInput, eachArgument);
        inputHandler(actualArgs,eachArgument, &callExit);
        clearContents(getUserInput, eachArgument);
    }while(callExit==0);
    
}


//================
//Function: allocateStringArr(int , int)
//Description: Generic function to allocate string array of demensions (size*bufit)
//================
char** allocateStringArr(int size, int bufit)
{
    char** genStrArr;
    int lup;
    genStrArr= malloc(size * sizeof(char*));
    
    for(lup=0; lup < size; lup++)
    {
        genStrArr[lup] = malloc(bufit * sizeof(char));
    }
    
    return genStrArr;
}

//================
//Function: userInputAndValidate(char [], char [], int)
//Description: First, this user checks for any uncaught zombie process
// and waits on them if found.  Also, pid/exit values are inserted.
// Then user input is extracted with fgets, only allowing for max size of input
// stored in global var MAXCHARINPUT (2048)
//================
void userInputAndValidate(char input[], char copyInput[], int bufsize)
{
    int status,pid,signal;
    while ( (pid = waitpid(-1, &status, WNOHANG)) > 0)  //Without blocking, call waitpid for all any uncaught backgorund process (zombie).
    {
       signal= checkStatus(status);  //Grab return value and print it to stdout
        printf("background pid %i is done: exit value %i\n",pid,signal);
        fflush(stdout);
    };
    printf(": ");   //Set : to get user input
    fflush(stdout);
    
    fgets(input,bufsize,stdin);  //gets input from user up to maxium of 2048
    strcpy(copyInput,input);
    //fflush(stdin);
}

//================
//Function: parseInput(char* , char** )
//Description: From two input variables, this function tokenizes the raw string
// of user data from usrStr into the tokenizedArr string array (already allocated mem)
// Also, counts how many arguemnts are parsed out, and returned in int numOfArgs.
// Note we parse by filters both 1)' ' (space) and 2)'\n' newline char.
//================
int parseInput(char* usrStr, char** tokenizedArr)
{
    int numOfArgs=0;
    char * tokens;
    
    tokens=strtok(usrStr," \n");
    tokenizedArr[numOfArgs]=tokens;
    if(tokens!=NULL)numOfArgs++;
    
    while(tokens !=NULL && numOfArgs <= 512)  //Make it so no more than 512 args are taken, for furture error handling.
    {
        tokens=strtok(NULL," \n");
        if(tokens!=NULL){
            tokenizedArr[numOfArgs]=tokens;
            numOfArgs++;}
    }
    
    return numOfArgs;  //Will be acurate number of arguments (not refering to index)
}

//================
//Function: clearContents(char* , char** )
//Description: Zero out our string array using memset on each index for
// re-use in next cycle of input.
//================
void clearContents(char* inp, char** args)
{
    memset(inp,0,MAXCHARINPUT);
    int lup;
    
    for(lup=0; lup < MAXARGS; lup++)  //Clear contents of each index
    {
        args[lup] = memset(inp,0,MAXCHARPERARG);
    }
}

//================
//Function: inputHandler(int ,char** , int* )
//Description: Here we decided what to do now that user input is parsed.
// Allocate memory for two int points to pass around special char info
// May decide to run built in functions (exit/cd/status/#/and (no input))
// If not passes parsed string to makeExeArr_findSpeialChars to form a string arr
// capable of being input into execution statement.
// Finally, send off to execuation function:
//================
void inputHandler(int numArg,char** arrOfArgs, int* setExit)
{
    char** exeString;
    int* placeOfSpecialChar=malloc(sizeof(int));
    int* symbolSwitchs=malloc(sizeof(int));
    *placeOfSpecialChar=99;
    
    //printf("Arg[0] is currently: %s\n",arrOfArgs[0]);
    //Handel '' no entry
    if(numArg==0)
    {
        return;//do nothing, handles no entry (blank line)
    }
    
    //Handel #
    if(arrOfArgs[0][0]=='#')
    {
        return;  //Lets line be comment
    }
    
    //Handel exit
    if(strcmp(arrOfArgs[0],"exit") ==0)
    {
        *setExit=1;
        return;
    }
    
    //Handel cd
    if(strcmp(arrOfArgs[0],"cd")==0)
    {
        builtIncd(arrOfArgs,numArg);
        return;
    }
    
    //Handel status
    if( strcmp(arrOfArgs[0],"status") ==0)
    {
        printf("%s",STATUSV);
        fflush(stdout);
        return;
    }
    
    char fileName[20];
    
    if(numArg>0)  //Error check for arguments (must be something)
    {//Makes string array to insert into execute statement, also, gets the index and type of any special characters
    exeString=makeExeArr_findSpeialChars(arrOfArgs,numArg,symbolSwitchs,placeOfSpecialChar, fileName);
    }
    
    //Rules for symbolSwitches
    // No redirection, just &, then ==3
    // If >+& == 4
    // If <+& ==5
    //else ==99  (means no special symbols)
        int checkSymb= *symbolSwitchs;
        execIt(exeString,checkSymb,fileName);
    
    
    free(placeOfSpecialChar);  //Free up memory for int*
    free(symbolSwitchs);       //Free up memory for int*
    memset(fileName,0,20);
}



//================
//Function: void builtIncd(char** , int)
//Description: As the title suggest, this function is intended to work like
// the "CD-change directory" built in *unix function.
// This will change the working directory, and user will return to the location
// they ran this program from prior.  Uses environmental var HOME
// to navigate home.
//================
void builtIncd(char** arrOfArgs, int numOfArgs)
{
    int check;
    
    if( numOfArgs==1)  //Only CD call (go home)
    {
        const char* home = getenv("HOME");
         check = chdir(home);
        if(check<0)
        {
            perror("Could not go home.\n(cd failed)\n");
        }
    }
    
    else{  //change directory to second variable input (cd xxx) <- the xxx location
        check = chdir(arrOfArgs[1]);
        if(check<0)
        {
            perror("Bad path\n");
            
        }

    }
    
    memset(STATUSV,0,30);  //clears status variable prior to insertion of exit value for cd function.
    
    if(check <0)  //Sets exit value based on chdir call return in check (if -1, exit value will be set to 1).
    {
    sprintf(STATUSV, "exit value %i\n",1);
    }
    else{
        sprintf(STATUSV, "exit value %i\n",0);
    }
}





//================
//Function: char** (char** arrOfArgs, int , int* , int* , char*)
//Description: Creates a string array with null value at end.
// Also, locates tracks down special characters and keeps track of their
// array index and the type of special character.  See the following table
// for usage of tracking special characters in the int pointer 'special':
// TABLE FOR INT* SPECIAL TRACKING:
//      If < == 1
//      If > == 2
//      No redirection, then ==3
//      If >+& == 4
//      If <+& ==5
// Returns execute array with NULL inserted at the end.  Also info stored in int*'s
//================
char** makeExeArr_findSpeialChars(char** arrOfArgs, int num, int* special, int* place, char* fileName)
{
    char** eachArgument= allocateStringArr(MAXARGS,MAXCHARPERARG);
    //fileName[0]='/';
    int lup;
    
    for(lup=0; lup < num; lup++)
    {
        if( strcmp("<",arrOfArgs[lup])==0)
        {
            eachArgument[lup]=NULL;
            *special=1;
            *place=lup;
            
            if(arrOfArgs[lup+1]!=NULL)
            {
                strcpy(fileName,arrOfArgs[lup+1]);
            }
           
        }
        
        else if( strcmp(">",arrOfArgs[lup])==0)
        {
            eachArgument[lup]=NULL;
            *special=2;
            *place=lup;
            
            if(arrOfArgs[lup+1]!=NULL)
            {
                strcpy(fileName,arrOfArgs[lup+1]);
            }
            
        }
        
        else if( strcmp("&",arrOfArgs[lup])==0)  //Here we create a sort of switch
            //based interger for combinations of < > and &
            // No redirection, then ==3
            // If >+& == 4
            // If <+& ==5
        {
            eachArgument[lup]=NULL;
            *place=lup;
            if(*special !=1 && *special != 2)
            {
                *special=3;
            }
            else if( *special ==1)
            {
                *special= 5;
            }
            else if( *special ==2)
            {
                *special= 4;
            }
            break;  //& must be last element, so exit loop after finding
        }
        else if(*place==99)  //No special symbols were found
        {
           eachArgument[lup]=arrOfArgs[lup];
            if(lup==num-1)  //Last run through and no special symbols
            {
                eachArgument[num]=NULL;  //Sets last element to NULL
            }
        }
    }
    
   
    return eachArgument;
}


//================
//Function: void execIt(char** , int , char*)
//Description: Forking/Signal Handeling, and executing.
// Child is split into new function, parent handling is internal.
// Parent calls waitpid on input without & inserted
// Parent also in charge of ignoring SIGINT calls to foreground procs
// as well as printing pid of background processes.
//================
void execIt(char** arr, int backGroundProc, char* fileName)
{
   struct sigaction sigSIGINT;  //set up sigaction struct for SINGINT signal handeling
                                // passed to child, used by both child and parent.
    
    pid_t pid=fork();  //-----FORK------ (Start child process)
    int status=0;
   
    if(pid < 0)
    {
        printf("Error");
        fflush(stdout);
    }
 
    //-------------
    //Child Process : calls runChild which does all handeling for child proc
    //-------------
    else if(pid==0)
    {
    runChild(arr,backGroundProc,sigSIGINT,fileName);
    }
    
    
    //-------------
    //Parent Process
    //-------------
    else
    {
        int sig;
        //Ignore SIGINT call
        sigSIGINT.sa_handler=SIG_IGN;  //By default, ignore
        sigSIGINT.sa_flags=0;
        sigfillset(&(sigSIGINT.sa_mask));
        sigaction(SIGINT, &sigSIGINT, NULL);
        
        
        if(backGroundProc<3)  //No background process initation
        {
            waitpid(pid,&status,WUNTRACED | WCONTINUED);
            //WUNTRACED:  Return if child has stopped
            //WCONTINUED: Return if child has stopped->continued
             sig=checkStatus(status);  //set status variable, 0 for don't print
        }
        
        else{
            printf("background pid is %i\n",pid);
            fflush(stdout);
        }
        
        
    }
}


//================
//Function: void runChild(char**,int , struct sigaction ,char* )
//Description:Handels child process.Checks for redirection and background proccess calls.
// Also calls signal handler for proccess that are not background and sets to default.
// Execute array based upon input type, with redirection being done prior.
// Here is a reminder of table used for backGroundProc to determine special charaters:
// TABLE FOR INT* SPECIAL TRACKING:
//      If < == 1
//      If > == 2
//      No redirection, then ==3
//      If >+& == 4
//      If <+& ==5
//================
void runChild(char**arr,int backGroundProc, struct sigaction sigStruct,char* fileName)
{
    //Set default sigint signal handler for foreground process
    if(backGroundProc<3)  //No background prcess initation, set default sigint signal
    {
        sigStruct.sa_handler= SIG_DFL;
        sigStruct.sa_flags=0;
        sigfillset(&(sigStruct.sa_mask));
        sigaction(SIGINT, &sigStruct, NULL);
    }
    
    //For input that calls a background process (&), redirect stdin and stdout to
    // file /dev/null
    else{
        int filed;
        const char* defaultoutp="/dev/null";
      
        filed = open(defaultoutp, O_WRONLY | O_CREAT | O_TRUNC,0644);
          fcntl(filed, F_SETFD,FD_CLOEXEC);  //Close upon execute
        
        if(filed==-1)
        {
            printf("Failed to open/create redirect file\n");
            fflush(stdout);
            exit(1);
        }
        
        dup2(filed,0);  //Stdin into default file
        dup2(filed,1);  //Stdout into default file
        
    }
    
    //--------
    //Handle > output redirection
    //--------
    if(backGroundProc==2)
    {
       
        int fd;  //Open write only, append to end, create if not exist
        fd = open(fileName, O_WRONLY | O_CREAT | O_APPEND,0644);
        if(fd==-1)
        {
            printf("Failed to open file\n");
            fflush(stdout);
            exit(1);
        }
        
        
        dup2(fd,1);  //Stdout into named-pipe file
        dup2(fd,2);  //Stdout into named-pipe file
        
        fcntl(fd, F_SETFD,FD_CLOEXEC);  //Close upon execution
        
        int checkStat=execvp(arr[0],arr);
        if(checkStat==-1)//Error handeling for bad execution!
        {
            printf("%s: no such file or directory.\n",arr[0]);
            fflush(stdout);
            exit(1);//Exit with one means something went wrong
        }
        
        exit(0);;//Exit with 0, worked!
    
    }
    
    //--------
    //Handle < input redirection
    //--------
    if(backGroundProc==1)
    {
        
        int fdIn;  //Open read only for input redirection
        fdIn = open(fileName, O_RDONLY ,0644);
        if(fdIn==-1)
        {
            printf("Failed to open file\n");
            fflush(stdout);
            exit(1);
        }
        
        
        dup2(fdIn,0);  //Stdin redirected to file
        fcntl(fdIn, F_SETFD,FD_CLOEXEC);  //Close upon execution
        
        
        //Call execvp, which searches for command, and uses array of strings as arguments,
        // terminated by a NULL at end of array.
        int checkStat=execvp(arr[0],arr);
        
        if(checkStat==-1) //Error handeling for bad execution!
        {
            printf("%s: no such file or directory.\n",arr[0]);
            fflush(stdout);
            exit(1);//Exit with one means something went wrong
        }
        
        exit(0);//Exit with 0, worked!
        
    }
    
    //--------
    //Execute!
    //--------
    
    //Call execvp, which searches for command, and uses array of strings as arguments,
    // terminated by a NULL at end of array.
    int checkStat=execvp(arr[0],arr);
    //Error handeling for bad execution!
    if(checkStat==-1)
    {
        printf("%s: no such file or directory.\n",arr[0]);
        fflush(stdout);
        exit(1);  //Exit with one means something went wrong
    }

    exit(0);  //Exit with 0, worked!
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







