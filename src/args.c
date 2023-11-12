/*******************************************************************************
* clam - Command Line Argument Manager
* Extrememly low footprint Argument Manager, specifically designed to support
* both desktop and embedded environments
*
* Please see the GitHub for more information:
*
* (c) ADBeta 2023    Version 1.3.2    05 Nov 2023
*******************************************************************************/
#include "args.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

ClamError_e clamerr = CLAM_ENONE;

static ArgDef_t *_defined_arg_arr      = NULL;
static const char **_undefined_arg_arr = NULL;

static size_t _defined_arg_count       = 0;
static size_t _undefined_arg_count     = 0;

/*** Functions ****************************************************************/
//TODO fix this
char *strclamerr(ClamError_e err)
{
	
	if(err == CLAM_ENONE) return "hello";
	
	return "world";
}

void Clam_InitDefinedArray(ArgDef_t *def_arr, const size_t def_count)
{
	_defined_arg_count = def_count;
	_defined_arg_arr = def_arr;
	
	//Initialise the array values
	for(size_t crnt_def = 0; crnt_def < _defined_arg_count; crnt_def++)
	{
		_defined_arg_arr[crnt_def].arg_type = CLAM_TUNDEF;
		_defined_arg_arr[crnt_def].flag_str = NULL;
		_defined_arg_arr[crnt_def].arg_str = NULL;
		_defined_arg_arr[crnt_def].detected = false;
	}
}

void Clam_InitUndefinedArray(const char **undef_arr, const size_t undef_count)
{
	_undefined_arg_count = undef_count;
	_undefined_arg_arr = undef_arr;
	
	//Init the char array to NULLs
	for(size_t crnt_undef = 0; crnt_undef < _undefined_arg_count; crnt_undef++)
	{
		_undefined_arg_arr[crnt_undef] = NULL;
	}
}

ArgDef_t *Clam_AddDefinition(const ArgType_e type, const char *flag)
{
	static size_t crnt_indx = 0;
	
	//If too many args have been added, exit with failure
	if(crnt_indx >= _defined_arg_count) {
		clamerr = CLAM_EMAXINDEX;
		return NULL;
	}
	
	ArgDef_t *arg_ptr = &_defined_arg_arr[crnt_indx];
	arg_ptr->arg_type = type;
	arg_ptr->flag_str = flag; 
	
	++crnt_indx;
	
	clamerr = CLAM_ENONE;
	return arg_ptr;
}

//Private function for library only
static int Clam_AddUndefined(const char *ptr)
{
	static size_t crnt_indx = 0;

	//If too many strings have been added, exit with failure
	if(crnt_indx >= _undefined_arg_count) return CLAM_EMAXINDEX;
	
	_undefined_arg_arr[crnt_indx] = ptr;
	++crnt_indx;
	
	clamerr = CLAM_ENONE;
	return 0;
}

int Clam_ScanArgs(int argc, char *argv[]) 
{
	if(_defined_arg_arr == NULL)
	{
		clamerr = CLAM_ENODEFMEM;
		return -1;
	}
	
	for(int argi = 0; argi < argc; argi++)
	{
		char *args = argv[argi]; //Current argument string
		bool match_found = false;
		
		//Scan through all defined arguments
		for(size_t crnt_def = 0; crnt_def < _defined_arg_count; crnt_def++)
		{
			ArgDef_t *def_obj = &_defined_arg_arr[crnt_def];
			
			if(def_obj->flag_str == NULL) continue;
			
			//If a match is found, set the detected bool, return string if
			//applicable, then continue to the next argument
			if(strcmp(args, def_obj->flag_str) == 0)
			{
				match_found = true;
				def_obj->detected = true;
				
				if(def_obj->arg_type == CLAM_TSTRING)
				{
					//Incriment argi to get the arg_string, detect overflow
					if(++argi >= argc)
					{
						#ifdef CONF_VERBOSE
						printf("Cannot find substring: end of inputs\n");
						#endif
						clamerr = CLAM_ENOSUBSTR;
						return -3;
					}
					
					def_obj->arg_str = argv[argi];
				}
				
				#ifdef CONF_VERBOSE
				printf("Found match for \"%s\". arg_str = %s\n", 
				       args, def_obj->arg_str);				
				#endif
				
				break;
			}
		}
		
		//If no match was found, either put it in the undefined array, or
		//return an error if the undefined array was not initialised 
		if(!match_found)
		{
			#ifdef CONF_VERBOSE
			printf("\"%s\" doesn't match any known definition\n", args);
			#endif
			
			if(_undefined_arg_arr == NULL)
			{
				clamerr = CLAM_EUNKNOWNARG;
				return -2;
			} else 
			{
				//TODO check return
				Clam_AddUndefined(args);
			}
		}		 
	}
	
	//All good
	clamerr = CLAM_ENONE;
	return 0;
}

ArgDef_t *Clam_SearchForFlag(const char *flag)
{
	//Scan through all defined arguments
	for(size_t crnt_def = 0; crnt_def < _defined_arg_count; crnt_def++)
	{
		ArgDef_t *def_obj = &_defined_arg_arr[crnt_def];
			
		//If a match is found, return its address. Skip any NULL string
		if(def_obj->flag_str == NULL) continue;
		
		if(strcmp(def_obj->flag_str, flag) == 0) {
			clamerr = CLAM_ENONE;
			return def_obj;
		}
	}
	
	//If no match is found, set clamerr and return NULL
	clamerr = CLAM_ENOMATCH;
	return NULL;
}

bool Clam_IsArgStrNumeric(const char *string)
{
	//Get length of string, then search each char. If any non-numeral chars
	//are matched, return false, if all chars are numeric, return true
	size_t len = strlen(string);
	
	size_t c_char = 0;
	//Allow first char to be '-'
	if(string[c_char] == '-') ++c_char;
	
	for( ; c_char < len; c_char++)
	{
		char test_char = string[c_char];
		if((test_char < '0' || test_char > '9')) return false;
	}
	
	return true;
}
