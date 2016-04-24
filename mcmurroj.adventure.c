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

//LOTR Dynamic Map!
//http://lotrproject.com/map/#zoom=3&lat=-1802.5&lon=1551&layers=BTTTTTTTT

const char* makeMyDir();


int main(int argc, const char * argv[]) {
    
    struct rooms{
        const char* name;
        const char* type;
        char* connections[6];  //Max of 6
    };
    
    char* roomnames[] = {"Lothlorien", "Fangorn", "Edoras", "Helm's Deep",
        "Orthanc", "Minas Tirith", "Black Gate", "Orodruin", "Shire", "Aman" };
    
    
    
    const char* myDirName = makeMyDir(); //Calls function to generate new directory, return name of dir
    
   
    
    
    
    
    
    
    
    //Remove directory, must be empty.
    rmdir(myDirName);
    return 0;
}


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



