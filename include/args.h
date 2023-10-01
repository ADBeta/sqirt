/*******************************************************************************
* Argument handling library, Prototype for improved version of CLIah for c
*
* ADBeta (c)2023
*******************************************************************************/
#include <stdbool.h>

typedef struct
{
	char *flag_str;       //String that flag the argument given. e.g -h
	
	
	
	//Private members
	
	bool detected;
} arg_vars_t;


//Scan all args passed to the program.
int scan_args(int argc, char *argv[]);
