//
//  main.c
//  Giggity
//
//  Created by Danny McMurrough on 4/23/16.
//  Copyright © 2016 Danbamboo. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>  //For random number generator
//#include <fcntl.h>  //For opening files...

//Free Malloc Memory!

//LOTR Dynamic Map!
//http://lotrproject.com/map/#zoom=3&lat=-1802.5&lon=1551&layers=BTTTTTTTT

const char* makeMyDir();
void generateRoomFiles(const char*);

int main(int argc, const char * argv[]) {
    //Create seed for  rand() "random" num generation
    srand((unsigned int)time(NULL));
    
    //Struct varaible used to hold rooms
    struct rooms{
        const char* name;
        const char* type;  //Game type can be: START_ROOM || MID_ROOM || END_ROOM
        char* connections[6];  //Max of 6 connections
    };
    
    //Array of structs to represent each room (7 rooms).
    struct rooms* roomsArr[7];
    
    
    
    
    //int random= rand() % 8;
    //printf("%i", random);
    
    
    const char* myDirName = makeMyDir(); //Calls function to generate new directory, return name of dir
    generateRoomFiles(myDirName);
    
    
    
    
    
    
    
    
    //Remove directory, must be empty.
    //rmdir(myDirName);
    return 0;
}

//================
//Function: makeMyDir()
//Description: This function makes a directory in current working directoy with the
//        format <username>.rooms.<pid>  Gets the current user name and pid and cats them
//        together.  Also, allocates memoery and returns the (const char*) name of
//        the directory generates for future handeling.
//================
const char* makeMyDir(){
    
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


void generateRoomFiles(const char* dir)
{
    //Hardcoded names of rooms, to be chosen at random (7of10)
    char* roomnames[] = {"Lothlorien", "Fangorn", "Edoras", "Helm's Deep",
        "Orthanc", "Minas Tirith", "Black Gate", "Orodruin", "Shire", "Aman" };
    
    int roomindex[7];
    //In order to choose random file names, this will get 7 random numbers out
    // of 0-9 and store those access numbers in an interger array, which will represent
    // the corresponding index to roomname.  Also, as this ordering will be random, I
    // will choose the first two indexes that are chosen to be the starting room and
    // ending room correspondingly.
    for(int i=0; i<7; i++)
    {
        int check=1;
        roomindex[i] = rand() %10;
        
        if(i!=0){ do{
            check=0;
            for(int z=i-1; z >= 0; z--)
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
    for(int i=0; i < 7; i++)
    {
        for(int z=0; z<7; z++)
        {
            if(i==0)
            {
                fileConnections[z][i] =roomindex[z];
            }
            else{
                fileConnections[z][i] = 99;
            }
            // printf("%i\n", fileConnections[z][i]); //sanity check
        }
        
    }
    
    
    //++++++THE FUN PART: MAKE CONNECTIONS!
    //
    for(int i=0; i < 7; i++)  //Loop through each room, first check how many
        //Connection currently have.
    {
       
       
       
        int decreaseOdds = rand() %50;
        int randNumOfConnToGen = rand() %4 +3;  //Randomly choose how many
        //connections to generate for room (3-6)...NOTE: IF room already has as many
        //or more connections than the # generated here, do nothing.
            if(decreaseOdds>1 && (randNumOfConnToGen==5 || randNumOfConnToGen==6))
               {
                   randNumOfConnToGen= randNumOfConnToGen -2;
               }
        
       
            
            
            
            
        //Check how many current connections held
        int howManyCurrentConn=0;
        
        if(i > 0)  //Check for how many connections held if past the first index
        {
            for(int x=1; x<7; x++)
            {int checkForVal= fileConnections[i][x];
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
            for(int y=0; y < diffConnToGen; y++)
            {
                int checkValidConn =0;
                int found=0;
                //Get new connection in do/while loop
                do{  //Here it would be ideal to use %7 to get result (0-6); however,
                    //this doesn't work with rand(), always return 0...so new
                    //solution, to keep it random, screen out 7;
                    int grabIndex = rand() % 8;
                    
                    //Bypass an index >6 or equal to current index (i)
                    if(grabIndex!=7 && grabIndex!=i)
                    {
                        if(howManyCurrentConn>0)  //If >0, we need to compare against
                            //the current connection held
                        {
                            //Runs through and checks random connection against all
                            //Previous Connections
                            int checkValid=0;
                            for(int c=1; c<=howManyCurrentConn; c++)
                            {
                                if(roomindex[grabIndex]==fileConnections[i][c])
                                {
                                    checkValid=1;
                                    break;
                                }
                            }
                            if(checkValid==0)
                            { found=1;}
                            
                            
                        }
                        else{found=1;}
                        
                        if(found==1)
                        {
                            fileConnections[i][howManyCurrentConn+1]=roomindex[grabIndex];
                            howManyCurrentConn++;
                            checkValidConn=1;
                            
                            int inc=1;
                            int foundEmpty=0;
                            int findEmpty;
                            do{
                                findEmpty=fileConnections[grabIndex][inc];
                                if(findEmpty==99)
                                {
                                    fileConnections[grabIndex][inc]=roomindex[i];
                                    foundEmpty=1;
                                }
                                inc++;
                            }while(foundEmpty==0);
                        }
                        
                    }
                }while(checkValidConn==0);
            }
        }
        

    
    
    
    FILE * fp = NULL;
    //++++++
    //CREATE AND WRITE TO FILES
    //++++++
    for(int i=0; i<7; i++)
    {
        //---
        //This section creates the 7 files based on the random algorith from above.
        char file[50];
        int roomNumber = roomindex[i];
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
        for(int con=1; con<7; con++)
        {
            char connNum[2];
            sprintf(connNum,"%d",con); //convert int to char[]
            connIndex=fileConnections[i][con];
            if(connIndex!=99)
            {
               fputs("CONNECTION ",fp);
               fputs(connNum,fp);
               fputs(":",fp);
               fputs(roomnames[connIndex],fp);
               fputs("\n",fp);
            }
            
        }
         
         
       
        
        
                //---
                //Now we will write room type to each file.
                //As stated, index 0 will be start, 1 end, and 2-6 middle rooms
                char* startRoom = "START_ROOM";
                char* midRoom = "MID_ROOM";
                char* endRoom = "END_ROOM";
                
                fputs("ROOM TYPE: ",fp);
                if(i==0)
                {
                    fputs(startRoom,fp);
                }
                else if(i==1)
                {
                    fputs(endRoom,fp);
                }
                else{
                    fputs(midRoom,fp);
                }
                //---
                
                
        
        
        
      
        //---
        memset(room,0,30);  //Resets the file array to all null terminated chars
        memset(file,0,50);  //Resets the file array to all null terminated chars
    }
       
    }
    
    
}















