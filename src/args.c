/*******************************************************************************
* clam - Command Line Argument Manager
* Extrememly low footprint Argument Manager, specifically designed to support
* both desktop and embedded environments
*
* Please see the GitHub for more information:
*
* (c) ADBeta 2023    Version 1.6.8    16 Nov 2023
*******************************************************************************/
#include "args.h"

#ifdef CONF_VERBOSE
#include <stdio.h>
#endif

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

/*** Predefined Error Strings *************************************************/
const char *const _clam_str_emaxdefs = "Reached Maximum Index of Defined Args";
const char *const _clam_str_emaxundefs = "Reached Maximum Index of Undefined Args";
const char *const _clam_str_enodefmem = "No  Memory Allocated for Defined Args";
const char *const _clam_str_eunknownarg = "Unknown Argument";
const char *const _clam_str_enosubstr = "Argument Requires a Substring";
const char *const _clam_str_enomatch = "No Matching Argument Found";

/*** Functions ****************************************************************/
const char *strclamerr(const ClamError_e err)
{	
	switch(err)
	{
		case CLAM_ENONE:
			return NULL;
		case CLAM_EMAXDEFS:
			return _clam_str_emaxdefs;
		case CLAM_EMAXUNDEFS:
			return _clam_str_emaxundefs;
		case CLAM_ENODEFMEM:
			return _clam_str_enodefmem;
		case CLAM_EUNKNOWNARG:
			return _clam_str_eunknownarg;	
		case CLAM_ENOSUBSTR:
			return _clam_str_enosubstr;
		case CLAM_ENOMATCH:
			return _clam_str_enomatch;
	}
	
	//If nothing has matched yet, return generic "unknown error" message
	return "Unknown Error Type";
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
		clamerr = CLAM_EMAXDEFS;
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
static const char *Clam_AddUndefined(const char *ptr)
{
	static size_t crnt_indx = 0;

	//If too many strings have been added, exit with failure
	if(crnt_indx >= _undefined_arg_count)
	{
		clamerr = CLAM_EMAXUNDEFS;
		return NULL;
	}
	
	_undefined_arg_arr[crnt_indx] = ptr;
	++crnt_indx;
	
	clamerr = CLAM_ENONE;
	return ptr;
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
						
						//NOTE Because argi was already incrimented, this is the
						//correct index to map to argv[]
						return argi;
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
		//return the position of the unknown arg.
		if(!match_found)
		{
			#ifdef CONF_VERBOSE
			printf("\"%s\" doesn't match any known definition\n", args);
			#endif
			
			if(_undefined_arg_arr == NULL)
			{
				clamerr = CLAM_EUNKNOWNARG;
				return argi + 1;
			} else 
			{
				const char *ret = Clam_AddUndefined(args);
				if(ret == NULL) return -2;
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
