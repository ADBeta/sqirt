/*******************************************************************************
* clam - Command Line Argument Manager
* Extrememly low footprint Argument Manager, specifically designed to support
* both desktop and embedded environments
*
* Please see the GitHub for more information:
*
* (c) ADBeta 2023

Example defining both arrays TODO
ArgDef_t def_args[ARG_COUNT];
const char *undef_args[ARG_COUNT];
Clam_InitDefinedArray(def_args, ARG_COUNT);
Clam_InitUndefinedArray(undef_args, ARG_COUNT);
*******************************************************************************/
#include <stdbool.h>
#include <stddef.h>

#ifndef CLAM_T
#define CLAM_T

/*** Configuration Compile-time flags *****************************************/
//#define CONF_VERBOSE

//Global error code
typedef enum {
	CLAM_ENONE = 0,
	CLAM_EMAXDEFS,        //No indexes left in defined argument array
	CLAM_EMAXUNDEFS,       //No indexes left in undefined argument array
	CLAM_ENODEFMEM,       //No memory has been allocated for defined args
	CLAM_EUNKNOWNARG,     //Passed argument is unknown/unrecognised
	CLAM_ENOSUBSTR,       //No substring exists for the current arg
	CLAM_ENOMATCH,        //No Matching defined argument was found
} ClamError_e;

extern ClamError_e clamerr;


typedef enum {
	CLAM_TUNDEF, CLAM_TFLAG, CLAM_TSTRING
} ArgType_e;

typedef struct
{
	ArgType_e arg_type;          //See ArgType enum
	const char *flag_str;        //String that flag the argument given. e.g -h
	const char *arg_str;         //String returned from command line
	bool detected;               //Flag if this argument has been found
} ArgDef_t;

//Returns a string based on the value of clamerr
const char *strclamerr(const ClamError_e err);

//Initialises the defined argument array from passed char array
void Clam_InitDefinedArray(ArgDef_t *def_arr, const size_t def_count);
//Initialises the undefined array from passed char array (Array of char[])
void Clam_InitUndefinedArray(const char **undef_arr, const size_t undef_count);
//Returns pointer to added definition, or NULL if error
ArgDef_t *Clam_AddDefinition(const ArgType_e type, const char *flag);

//Scan all args passed to the program.
//Returns the argument index that caused the error (+1 to align it to the actual
//argv array)
//Returns -1 if no memory was allocated
//Returns 0 for no fault
//Sets clamerr to a CLAM_E Error
int Clam_ScanArgs(int argc, char *argv[]);

//Scans through all args looking for one with a flag_str matching passed char *
ArgDef_t *Clam_SearchForFlag(const char *flag);

//Returns if the string contains only numeric chars (must be null terminated)
bool Clam_IsArgStrNumeric(const char *string);

#endif
