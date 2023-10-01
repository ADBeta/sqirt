/*******************************************************************************
* Argument handling library, Prototype for improved version of CLIah for c
*
* ADBeta (c)2023
*******************************************************************************/
#include <args.h>


#include <stdio.h>
#include <string.h>

int scan_args(int argc, char *argv[]) 
{
	int argi = 0; //Arg index

	while(argi < argc) {
		char *args = argv[argi]; //Current argument string
	
		printf("Arg %i: %s\n", argi, args);
		
		if(strcmp(args, "-help") == 0) {
			printf("Help found\n");
		}
		
		
		
		
		
		++argi;
	}
	
	return 0;
}
