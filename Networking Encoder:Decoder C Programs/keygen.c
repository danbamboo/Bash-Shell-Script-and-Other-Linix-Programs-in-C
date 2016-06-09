// keygen.c:: CS_344_400  -- Program 4 - one-time-pad encryption using networks
// Operating Systems:: Oregon State University
// Author: Danny McMurrough
// Program:  This small program randomly outputs a user input number of 
// charatcters to stdout.  The characters will be capitol letters A-Z and 
// the space character.


#include <stdio.h>
#include <stdlib.h>
#include <time.h>


void errorMsgExit(const char*);

int main(int argc, char *argv[])
{
	srand(time(NULL));  //Seed random#generator

	int length_to_generate, rand_int, rand_char,lup;
	  //Screen for invalid arguments
    
    if (argc < 2) {
        errorMsgExit("ERROR, no length provided.\n");
    }

     length_to_generate = atoi(argv[1]);  //Convert inputed string of length to integer


	//65=A   to 90=Z  and 32=SPACE
     // Loops through user input number of times to generate random character
	for(lup=0;lup<length_to_generate;lup++)
	{
		rand_int = rand() % 27 + 65;  //Mod 27 to account for A-Z and space
		if(rand_int==91)  //Allow 91 to be represented by the space
		{
			rand_char=32;
		}
		else
		{
			rand_char=rand_int;
		}
		printf("%c",rand_char);  //Output to stdout

	}
	printf("\n");  //Add newline to the end

	    return 0;
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

