// Program 2:: CS_344_400
// Operating Systems:: Oregon State University
// Author: Danny McMurrough
// Program Description:  Working with 'raw C', design a Colossal Cave Adventure
// type game.  This game generates 7 or 10 possible rooms randomly, and assigns
// 3-6 connections between room.  I have a LOTR flavor of this program.
//  ENJOY!

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>  //For random number generator



//I referenced this LOTR Dynamic Map, to find out the path of Gandalf the White!
//http://lotrproject.com/map/#zoom=3&lat=-1802.5&lon=1551&layers=BTTTTTTTT


//Function prototypes
char* makeMyDir();
char**  generateRoomFiles(char*);
void readFromFile(char**, char*);
void middleEarthAdventure(char*);
int getIndexOfName(char*,int);
void poemForTheSoul(int currentLocation);

//Struct varaible used to hold rooms
typedef struct rooms {
    int nameIndex;
    int typeIndex;  //Game type can be: START_ROOM || MID_ROOM || END_ROOM (0,1,2)
    int connections[6];  //Max of 6 connections
}rooms;

//Array of structs to represent each room (7 rooms).
rooms gandalfTheWhite[7];

//Hardcoded names of rooms, to be chosen at random (7of10)
char* roomnames[] = {"Lothlorien", "Fangorn", "Edoras", "Helm's Deep",
    "Orthanc", "Minas Tirith", "Black Gate", "Orodruin", "Shire", "Aman" };


//===================
//======= MAIN ======
//===================
int main(int argc, const char * argv[]) {
    //Create seed for  rand() "random" num generation
    srand((unsigned int)time(NULL));
    
    //Calls function to generate new directory, return name of dir
    char* myDirName = makeMyDir();
    //Returns a 'string array' of file names, pass in directory name.
    char** roomNames= generateRoomFiles(myDirName);
    //Input game info from file, pass in file names and directory name.
    readFromFile(roomNames,myDirName);
    //Main driver of game, after game information extracted
    middleEarthAdventure(myDirName);
    
    
    
    
    //FREE UP ALLOCATED MEMORY
    int freeIt;
    for(freeIt=0;freeIt<7; freeIt++)
    {free(roomNames[freeIt]);}
    free(roomNames);
    free(myDirName);
    //---

    return 0;
}
//===================
//===================


//================
//Function: makeMyDir()
//Description: This function makes a directory in current working directoy with the
//        format <username>.rooms.<pid>  Gets the current user name and pid and cats them
//        together.  Also, allocates memoery and returns the (const char*) name of
//        the directory generates for future handeling.
//================
char* makeMyDir(){
    
    int maxBuf = 25;
    char myid[maxBuf];
    char pidarr[maxBuf];
    const char* id = getlogin();  //Returns a pointer to a string containing name of user logged in
    int intpid = getpid();
    sprintf(pidarr,"%d",intpid); //convert int to char[]
    // const char* constpid = pidarr;
    strcpy(myid, id);
    strcat(myid,".rooms.");
    strcat(myid, pidarr);
    mkdir(myid, 0700);
    
    //Allocate memory to send char* back
    char* sendDirBack = malloc(sizeof(char)*sizeof(myid));
    strcpy(sendDirBack, myid);
    return sendDirBack;
}

//================
//Function: generateRoomFiles(char*)
//Description: Creates random rooms, generates connections, and writes to file.
//  Sets up a 2D array, where first column is the current room, and following 6 numbers
//  in each row behind current room number is potential connections.  The value 99
//  is filled in array to check for empty values.
//================
char** generateRoomFiles( char* dir)
{
    int roomindex[7];
    //In order to choose random file names, this will get 7 random numbers out
    // of 0-9 and store those access numbers in an interger array, which will represent
    // the corresponding index to roomname.  Also, as this ordering will be random, I
    // will choose the first two indexes that are chosen to be the starting room and
    // ending room correspondingly.
    int i;
    for(i=0; i<7; i++)
    {
        int check=1;
        roomindex[i] = rand() %10;
        
        if(i!=0){ do{
            check=0;
            int z;
            for(z=i-1; z >= 0; z--)
            {
                if(roomindex[z]==roomindex[i])
                {
                    check=1;
                    break;
                }
            }
            if(check==1)
            {
                roomindex[i] = rand() %10;
            }
        }while(check==1);}
        //printf("%i\n", roomindex[i]);  //sanity check
    }
    
    
    //++++++  SETUP 2D ARRAY
    //Generate a 2D array, for storage of fileName(Column 0) and connections(Col 1-5)
    //Max # of connections is 6
    //Fills unused slots with value 99 for checking
    //++++++
    int fileConnections [7][7];
    int ii;
    for(ii=0; ii < 7; ii++)
    {
        int zz;
        for( zz=0; zz<7; zz++)
        {
            if(i==0)
            {
                fileConnections[zz][ii] =roomindex[zz];
            }
            else{
                fileConnections[zz][ii] = 99;
            }
            // printf("%i\n", fileConnections[zz][ii]); //sanity check
        }
        
    }
    
    //++++++THE FUN PART: MAKE CONNECTIONS!
    //----
    int makeCons;
    for(makeCons=0; makeCons < 7; makeCons++)  //Loop through each room, first check how many
        //Connection currently have.
    {
        
        //Here was decrease the chance the random number of rooms attempted to assign
        // is not 5 or 6.  Rooms will grow at times if other rooms have increasing number
        // of connections.  This is a way to keep connections somewhat minimized, yet
        // they still have the potentially to be assigned any number.
        int decreaseOdds = rand() %50;  //0-49
        int randNumOfConnToGen = rand() %4 +3;  //Randomly choose how many
        //connections to generate for room (3-6)...NOTE: IF room already has as many
        //or more connections than the # generated here, prog will do nothing.
        if(decreaseOdds>1 && (randNumOfConnToGen==5 || randNumOfConnToGen==6))
        {
            randNumOfConnToGen= randNumOfConnToGen -2;  //Turn a 6 or 5 into 5 or 4
        }

        
        //Check how many current connections held
        int howManyCurrentConn=0;
        if(makeCons > 0)  //Check for how many connections held if past the first index
        {
            int howmcon;
            for(howmcon=1; howmcon<7; howmcon++)
            {int checkForVal= fileConnections[makeCons][howmcon];
                if(checkForVal!=99)
                    howManyCurrentConn++;
                else
                    break;}
        }
        
        //Get the difference between randconnections and current connections.
        //If difference is 0 or less, do nothing
        int diffConnToGen = randNumOfConnToGen - howManyCurrentConn;
        
        if(diffConnToGen > 0)  //Only generate new connections for current room if
            // it doesn't have as many connections as our random
            //variable randNumOfConnToGen has chosen
        {
            //Loop though as many times as new connections need be made
            int newCons;
            for(newCons=0; newCons < diffConnToGen; newCons++)
            {
                int checkValidConn =0;
                int found=0;
                //Get new connection in do/while loop
                do{  //Here it would be ideal to use %7 to get result (0-6); however,
                    //this doesn't work with rand(), always return 0...so new
                    //solution, to keep it random, screen out 7;
                    int grabIndex = rand() % 8;
                    
                    //Bypass an index >6 or equal to current index (i)
                    if(grabIndex!=7 && grabIndex!=makeCons)
                    {
                        if(howManyCurrentConn>0)  //If >0, we need to compare against
                            //the current connection held
                        {
                            //Runs through and checks random connection against all
                            //Previous Connections
                            int checkValid=0;
                            int c;
                            for(c=1; c<=howManyCurrentConn; c++)
                            {
                                if(roomindex[grabIndex]==fileConnections[makeCons][c])
                                {
                                    checkValid=1;
                                    break;
                                }
                            }
                            if(checkValid==0)  //if connection is found,valid, set found to 1
                            { found=1;}
                            
                            
                        }
                        else{found=1;}  //If no current connections, valid, found=1
                        
                        if(found==1)
                        {
                            //add valid connection to next slot in array
                            fileConnections[makeCons][howManyCurrentConn+1]=roomindex[grabIndex];
                            howManyCurrentConn++;  //Increment current connections
                            checkValidConn=1;  //Signal to exit do/while loop
                            
                            int inc=1;
                            int foundEmpty=0;
                            int findEmpty;
                            
                            //Set connection in other direction!
                            do{
                                findEmpty=fileConnections[grabIndex][inc];
                                if(findEmpty==99)
                                {
                                    fileConnections[grabIndex][inc]=roomindex[makeCons];
                                    foundEmpty=1;
                                }
                                inc++;
                            }while(foundEmpty==0);
                        }
                        
                    }
                }while(checkValidConn==0);
            }
        }
    }
 
    
    FILE * fp = NULL;
    //++++++
    //CREATE AND WRITE TO FILES
    //++++++
    int create;
    for(create=0; create<7; create++)
    {
        //---
        //This section creates the 7 files based on the random algorith from above.
        char file[50];
        int roomNumber = roomindex[create];
        char room[30];
        strcpy(room,roomnames[roomNumber]);
        strcpy(file,dir);
        strcat(file,"/");
        strcat(file,room);
        fp = fopen(file,"w+");  //Creates file for reading and writing
        //int file_descriptor = open(file, O_RDWR | O_CREAT, 0777);
        //---
        
        //---
        //Now we will write room name to each file.
        fputs("ROOM NAME: ",fp);
        fputs(room,fp);
        fputs("\n",fp);
        
        
        //++++++WRITE CONNECTIONS TO FILE
        int connIndex;
        int con;
        char* startRoom = "START_ROOM";
        char* midRoom = "MID_ROOM";
        char* endRoom = "END_ROOM";
        for(con=1; con<7; con++)
        {
            char connNum[2];
            sprintf(connNum,"%d",con); //convert int to char[]
            connIndex=fileConnections[create][con];
            if(connIndex!=99)  //As long as we are not at empty spot in array, write connection.
            {
                fputs("CONNECTION ",fp);
                fputs(connNum,fp);
                fputs(": ",fp);
                fputs(roomnames[connIndex],fp);
                fputs("\n",fp);
            }
            //Set to input room type for two different conditions:
            //Cond1:  Reached the element after last filled connection(99), when connections are less than 6
            //Cond2:  All connections are filled, but last iteration of loop
            if(connIndex==99 || (connIndex!=99 && con==6)){ //---
                //Now we will write room type to each file.
                //As stated, index 0 will be start, 1 end, and 2-6 middle rooms
                fputs("ROOM TYPE: ",fp);
                if(create==0)
                {
                    fputs(startRoom,fp);
                }
                else if(create==1)
                {
                    fputs(endRoom,fp);
                }
                else{
                    fputs(midRoom,fp);
                    //fputs("What",fp);
                }
                fputs("\n",fp);
                break;
            }
            
        }
        fclose(fp);  //Close file
        //---
        memset(room,0,30);  //Resets the file array to all null terminated chars
        memset(file,0,50);  //Resets the file array to all null terminated chars
    }
    
    //Allocate memory to pass array of names back to main.
    char** roomArr = malloc(7 * sizeof(char*));
    int lpmal;
    for(lpmal=0; lpmal < 7; lpmal++)
    {
        roomArr[lpmal]= malloc(15 * sizeof(char));
        
    }
    int rooms;
    int roomIndex;
    //Assign variable roomArr the names of files (could have sent back ints arr as well).
    for(rooms=0; rooms < 7; rooms++)
    {
        roomIndex=roomindex[rooms];
        strcpy(roomArr[rooms], roomnames[roomIndex]);
    }
    return roomArr;
}

//================

//================
//Function: readFromFile()
//Description:  Start from scratch again, and read in rooms from file.
//        Pass in array of file names, and directory name to access.
//        Parses the information from file, and inputs game information into program.
//================
void readFromFile(char** rooms, char* dir)
{
    char roomFile[50];
    char currentLIne[50];
    char room[30];
    FILE * fp = NULL;
    
    int x,y;

    //Generate Structs
    for(x=0;x<7;x++)
        
    {
        strcpy(room,rooms[x]);
        strcpy(roomFile,dir);
        strcat(roomFile,"/");
        strcat(roomFile,room);
        fp = fopen(roomFile,"r+");
        
        if(fp==NULL)
        {
            printf("File Doesn't Exist");
            exit(0);
        }
        rewind(fp);  //Take "file pointer" back to beginning
        
        //Make array of ints in struct filled with 99 to signify empty slot
        for(y=0; y<6; y++)
        {
            gandalfTheWhite[x].connections[y]= 99;
        }
        
        
        int checkLine=0;
        int connections=0;
        //Read in file line by line
         while(fgets(currentLIne, 50, fp) !=NULL)
         {
         char * tokens;
         int lookup,i;
         char noSpaceName[20];
         tokens=strtok(currentLIne,":");  //Here we seperate out names past colon
         while( (tokens=strtok(NULL, "\n")) !=NULL)
         {
             memset(noSpaceName,0,20);
             //Get rid of leading white space in string
             for(i=1; i< strlen(tokens); i++)
             {
                 noSpaceName[i-1]=tokens[i];
             }
             //The first line will be the current room name, get it!
             if(checkLine==0)
             {
                 for(lookup=0;lookup<10; lookup++)
                 {
                     //Compare string to roomnames[] array position to get index,
                     //assign index to each structure nameIndex variable (int)
                     if(strcmp(noSpaceName,roomnames[lookup])==0)
                     {
                         gandalfTheWhite[x].nameIndex= lookup;
                         break;
                     }
                   
                 }
                 
             }
             //Input room type.  Will really only end up checking for
             // End Room (value of 1)
             else if(strcmp(noSpaceName,"START_ROOM")==0||strcmp(noSpaceName,"MID_ROOM")==0||strcmp(noSpaceName,"END_ROOM")==0)
             {
                 if(strcmp(noSpaceName,"START_ROOM")==0)
                 {
                     gandalfTheWhite[x].typeIndex= 0;
                 }
                 else if(strcmp(noSpaceName,"END_ROOM")==0)
                 {
                     gandalfTheWhite[x].typeIndex= 1;
                 }
                 else{
                     gandalfTheWhite[x].typeIndex= 2;
                 }
                 
                 
             }
             //Assign connections
             else{
                 for(lookup=0;lookup<10; lookup++)
                 {
                     if(strcmp(noSpaceName,roomnames[lookup])==0)
                     {
                         gandalfTheWhite[x].connections[connections]= lookup;
                         break;
                     }
                 }
                 connections++;  //Keeps track of how many connections for input purposes
             }
             checkLine++; //Var used to check if needs to assign names.
             
         }
             
          
 
         memset(currentLIne,0,50);
         }
        
        
        
        
        
        memset(roomFile,0,50);  //Resets the file array to all null terminated chars
    }
    
}

//================
//Function: middleEarthAdventure(char*)
//Description:  Main game driver, uses information previously read from files, stored in struct array.
//        Loops through rooms until end room is found.  Validates input. Calls poemForTheSoul with victory.
//================
void middleEarthAdventure(char* dir)
{
    printf("Welcome to a recreation of Colossal Cave Adventure!\n");
    printf("You will travel a path of locations Gandalf the White visited in the Tolkein land of Middle Earth.\n");
    printf("There will be a random set of 7 locations assigned, and you will have to make it to the end.\n");
    printf("The thing is, you don't know where the end lies. \n");
    printf("\n");
    //printf("\"Not all those who wander are lost\".\n");
 
    
     printf("\n");
    printf("===================================\n");
    printf("\"The board is set. The pieces are moving.\"  ~Gandalf the White\n");
    printf("===================================\n");
    printf("\n");
    printf("\n");
    
    
    //Create new file to write the path taken to.
    FILE * fp = NULL;
    char fileForUserPath[50];
    strcpy(fileForUserPath,dir);
    strcat(fileForUserPath,"/userPath");
    fp = fopen(fileForUserPath,"w+");
    
    
    
    int endIsFound=0;
    int currentLocation=0;
    char nameOfLocation[20];
    int currentLocIndx, connIndx;
    int loop1;
    int isValid, userIndex;  //Used to verify user input correct value
    int counter=0;

    //Loop driver for the game
    do{
        isValid=0;  //used to check user input
        currentLocIndx = gandalfTheWhite[currentLocation].nameIndex;  //Index of room we are at (stats at 0, as 0 is also assigned to the starting room)
        strcpy(nameOfLocation,roomnames[currentLocIndx]);
        //Print name of current room
        printf("CURRENT LOCATION: %s\n",nameOfLocation);
        printf("POSSIBLE CONNECTIONS:");
        //Print, with specific formatting, list of connections from current froom
        for(loop1=0; loop1<6; loop1++)
        {
            connIndx=gandalfTheWhite[currentLocation].connections[loop1];
            if(connIndx!=99)
            {
                printf(" %s",roomnames[connIndx]);
                
                //period or comma syntax
                if(loop1==5)//If at the end, period
                {
                    printf(".\n");
                    break;
                }
                //If next connection is empty, period
                else if(gandalfTheWhite[currentLocation].connections[loop1+1]==99)
                {
                    printf(".\n");
                    break;
                }
                else
                {
                    printf(",");
                }
            }
            else{
                break;
            }
            
        }
        
        char insertName[20];
        int rmv, loop2;
        do{
            printf("WHERE TO? >");
            fgets(insertName,20,stdin);
            //Try and remove new line char.
            for(rmv=0; rmv < 20; rmv++)
            {
                if(insertName[rmv]=='\n')
                {
                    insertName[rmv]='\0';
                    break;
                }
            }
            
            //Call function, check for valid index
            userIndex= getIndexOfName(insertName,currentLocation);
            if(userIndex==99)
                //99 Represents invalid index, get new input from user
            {
                printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
            }
            else
            {
                printf("\n");
                counter++;
                isValid=1;
            }
        }while(isValid==0);
        
        //Write the Name of Location Traveled to in file to document path
        // Uses unserIndex, index of input name extracted from previous loop
        fputs(roomnames[userIndex],fp);
        fputs("\n",fp);
        //Find out which array element connection is based off index of the name
        //User input, which was stored in userIndex
        for(loop2=0; loop2 <7; loop2++)
        {
            
            if(gandalfTheWhite[loop2].nameIndex==userIndex)
            {
                currentLocation=loop2;
                break;
            }
        }
        
        //You got to the end room!  Display # of steps, and path stored in temp file.
        if(gandalfTheWhite[currentLocation].typeIndex==1)
        {
            printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
            printf("YOU TOOK %i STEPS.  YOUR PATH TO VICTORY WAS:\n",counter);
            endIsFound=1;
        }
        
        
     memset(nameOfLocation,0,20);
    }while(endIsFound==0);
    
    rewind(fp);
    char currentLine[50];
    while(fgets(currentLine, 50, fp) !=NULL)
    {
        printf("%s",currentLine);
        memset(currentLine,0,50);
    }
    printf("\n");
    printf("\n");
    
    //Remove temp file for user path
    fclose(fp);
    remove(fileForUserPath);
    
    //Output Tolkein Poem for victory!
    poemForTheSoul(currentLocation);
    
}


//================
//Function: getIndexOfName(char*,int)
//Description:  Validates user input, check input name and looping though connections.
//        Either returns valid index or 99 for no valid found.
//================
int getIndexOfName(char* name, int currLoc)
{
    int connIndx;
    int loop1;
    
    
    for(loop1=0; loop1<6; loop1++)
    {
        connIndx=gandalfTheWhite[currLoc].connections[loop1];
        if(connIndx==99)
        {
            break;  //Invalid input, break then exit
        }
        if(strcmp(name,roomnames[connIndx])==0)
        {
            return connIndx;
        }
        
        
    }
    connIndx=99;
    return connIndx;
}


//================
//Function: poemForTheSoul(int)
//Description:  Rewards user with poem upon victory.
//        If ending room is Lothlorien, output specific poem.
//        Else, choose from random 3 other poems.
//================
void poemForTheSoul(int currentLocation)
{
    int yourWinnings= rand() % 3;
    
    //TOLKIEN POEM's!
    // Utalized Tolkein Gateway : http://tolkiengateway.net/wiki/Main_Page
    
    //If Lothlorien was the ending room
    if(gandalfTheWhite[currentLocation].nameIndex==0)
    {
        printf("In Dwimordene, in Lórien\n");
        printf("Seldom have walked the feet of Men,\n");
        printf("Few mortal eyes have seen the light\n");
        printf("That lies there ever, long and bright.\n");
        printf("Galadriel! Galadriel!\n");
        printf("Clear is the water of your well\n");
        printf("White is the star in your white hand\n");
        printf("Renewed shall be blade that was broken,\n");
        printf("Unmarred, unstained is leaf and land\n");
        printf("In Dwimordene, in Lórien\n");
        printf("More fair than thoughts of Mortal Men.\n");
        printf("\n");
        printf("~Gandalf\n");
        printf("\n");
    }
    
    else if(yourWinnings==0)
    {
        printf("Ho! Ho! Ho! to the bottle I go\n");
        printf("To heal my heart and drown my woe.\n");
        printf("Rain may fall and wind may blow,\n");
        printf("And many miles be still to go,\n");
        printf("But under a tall tree I will lie,\n");
        printf("And let the clouds go sailing by.\n");
        printf("\n");
    }
    else if(yourWinnings==1)
    {
        printf("All that is gold does not glitter\n");
        printf("Not all those who wander are lost\n");
        printf("The old that is strong does not wither,\n");
        printf("Deep roots are not reached by the frost.\n");
        printf("\n");
        printf("From the ashes a fire shall be woken,\n");
        printf("A light from the shadows shall spring\n");
        printf("Renewed shall be blade that was broken,\n");
        printf("Renewed shall be blade that was broken,\n");
        printf("The crownless again shall be king.\n");
        printf("\n");
        printf("~Bilbo\n");
        printf("\n");
    }
    else if(yourWinnings==2)
    {
        printf("Still round the corner there may wait\n");
        printf("A new road or a secret gate\n");
        printf("And though I oft have passed them by,\n");
        printf("A day will come at last when I\n");
        printf("Shall take the hidden paths that run\n");
        printf("West of the Moon, East of the Sun.\n");
        printf("\n");
        printf("~Frodo's adaptation of Bilbo's song\n");
        printf("\n");
    }
    
     
}




