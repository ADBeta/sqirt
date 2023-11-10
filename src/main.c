/*******************************************************************************
* This file is part of sqrt: Serial Query Response Tool
* A simple program to send a request to a Serial Device, and return the response
* string back to the user
* Specifically designed for desktop and embedded environments
*
* See the GitHub for more information:
*
* ADBeta (c)    Version 0.4.2    11 Nov 2023
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "serial.h"
#include "args.h"

#define ARG_COUNT 10

/*** String definitions *******************************************************/
const char *const help_prompt_str = "Try 'sqrt -h' for more information.";
const char *const help_str = "\
sqrt\tSerial Query Response Tool \n\
Sends a message to a Serial PORT, then echos its reponse to stdout\n\n\
Basic Usage: sqrt -p [port] -m [message] [OPTIONAL]\n\
Example: sqrt -p /dev/ttyUSB0 -m \"Hello World!\"\n\n\
Arguments:\n\
  -p\tWhich PORT to communicate with (REQUIRED)\n\
  -m\tMessage to send to PORT. Use \"\" for messages with spaces or \'\' for special characters (REQUIRED)\n\
\n\
  -br\tBaudrate to use. Valid Options: 2400, 4800, 9600, 19200, 38400, 115200 (Default: 115200)\n\
  -wt\tHow long to wait before writing to PORT (0.1 sec increments. Some devices reset when PORT is opened. Valid Options: 0-1000) (Default: 0)\n\
  -to\tTimeout for the PORT to respond. Valid Options: 0-255 (0.1 sec increments) (Default: 10)\n\
  -bl\tBit Length the PORT uses to communicate. Valid Options: 5, 6, 7, 8 (Default: 8)\n\
  -bs\tBuffer Size of the response string (Default: 256)\n\
\nFlags:\n\
  -nl\tAppends NewLine (\"\\r\\n\") to the message automatically\n\
  -sl\tOnly capture a Single Line from the PORT during the response\n\
  -h\tShow this help message\
\n\nSee the GitHub for More Information and bug fixes <https://github.com/ADBeta/sqrt>\n\
sqrt Version TODO    (c) ADBeta TODO month 2023\n";


const char *const out_of_range = "Value is out of range";
const char *const invalid_num_str = "String is not a valid Numeric String";

/*** Function pre-declarations ************************************************/
//Takes a clam_arg string and pointer to a long; sets the ptr to the numeric 
//value of the string. Returns int error codes:
//0    No Error
//-1   String is not a valid numeric string
//-2   Value exceeded the limit value
int GetNumericLimitedFromArg(const char *str, long *val, const long limit);

//Prints an error message to stderr, then exits the program. Pass function the
//error happened in, and the reason it happened.
void PrintErrorAndExit(const char *funct, const char *reason, const char *aux);

/*** Main Program *************************************************************/
int main(int argc, char *argv[])
{
	/*** Serial Device User Configurable Parameter Pre-definition *************/
	unsigned int conf_baud = B115200;
	unsigned int conf_wait = 0;
	uint8_t conf_timeout = 10;
	unsigned int conf_bitlength = CS8;
	size_t conf_buffersize = 256;
	
	/*** Initialise the Argument Array ****************************************/
	ArgDef_t def_args[ARG_COUNT];
	Clam_InitDefinedArray(def_args, ARG_COUNT);
	
	/*** Define arguments and Scan Arguments **********************************/
	//Help Message Special Case
	ArgDef_t *help_ptr = Clam_AddDefinition(CLAM_TFLAG, "-h");
	
	//Arguments that have a return string
	ArgDef_t *port_ptr = Clam_AddDefinition(CLAM_TSTRING, "-p");
	ArgDef_t *mesg_ptr = Clam_AddDefinition(CLAM_TSTRING, "-m");
	////
	ArgDef_t *baud_ptr = Clam_AddDefinition(CLAM_TSTRING, "-br");
	ArgDef_t *wait_ptr = Clam_AddDefinition(CLAM_TSTRING, "-wt");
	ArgDef_t *time_ptr = Clam_AddDefinition(CLAM_TSTRING, "-to");
	
	
	ArgDef_t *buff_ptr = Clam_AddDefinition(CLAM_TSTRING, "-bs");
	
	//Arguments that set a detected flag
	ArgDef_t *nlin_ptr = Clam_AddDefinition(CLAM_TFLAG, "-nl");
	
	//Check the clamerr value to ensure all definitions were added
	if(clamerr != CLAM_ENONE)
	{
		//TODO
		fprintf(stderr, "Error while adding Definitions: %i:\n", (int)clamerr);
		exit(EXIT_FAILURE);
	}
	
	//Scan Arguments and check for errors
	Clam_ScanArgs(argc-1, argv+1);
	if(clamerr != CLAM_ENONE)
	{
		fprintf(stderr, "Error while parsing arguments: %i:\n", (int)clamerr);
		exit(EXIT_FAILURE);
		//TODO handle errors. print msg and leave
	}
		
	/*** If help was requested, print the help message ************************/
	if(help_ptr->detected)
	{
		fprintf(stdout, "%s", help_str);
		exit(EXIT_SUCCESS);
	}
	
	/*** Failsafe checks. Port and Message Must be defined ********************/
	if(port_ptr->detected == false)
	{
		PrintErrorAndExit("Usage", "You must specify a port with -p", "");
	}
	
	if(mesg_ptr->detected == false)
	{
		PrintErrorAndExit("Usage", "You must specify a message with -m", "");
	}
	
	/*** Detect and handle User Configurable Parameters ***********************/
	//Badrate
	if(baud_ptr->detected)
	{
		//Check input matches specific supported values
		
		
		
		//If not, print an error message and eixt
		fprintf(stderr, "Error: Baudrate given is not valid\n%s\n", 
		                 help_prompt_str);
		exit(EXIT_FAILURE);
	}
	
	//Wait time
	if(wait_ptr->detected)
	{
		long strval = 0;
		const char *arg = wait_ptr->arg_str;
		int ret = GetNumericLimitedFromArg(arg, &strval, 1000);
		
		//If any error has occured, print a message and exit (also check if 
		//value is negative)
		if(strval < 0) ret = -2;
		if(ret != 0)
		{
			if(ret == -1) PrintErrorAndExit("Wait Time", invalid_num_str, arg);
			if(ret == -2) PrintErrorAndExit("Wait Time", out_of_range, arg);
		}
		
		//Otherwsie set the configuration value. Cast to unsigned int
		conf_wait = (unsigned int)strval;
	}
	
	//Timeout
	if(time_ptr->detected)
	{
		long strval = 0;
		const char *arg = time_ptr->arg_str;
		int ret = GetNumericLimitedFromArg(arg, &strval, 255);
		
		//If any error has occured, print a message and exit (also check if 
		//value is negative)
		if(strval < 0) ret = -2;
		if(ret != 0)
		{
			if(ret == -1) PrintErrorAndExit("Timeout", invalid_num_str, arg);
			if(ret == -2) PrintErrorAndExit("Timeout", out_of_range, arg);
		}
		
		//Otherwsie set the configuration value. Cast to uint8_t
		conf_timeout = (uint8_t)strval;
	}
	
	
	//Bit Length
	
	//Buffer Size
	if(buff_ptr->detected)
	{
		long strval = 0;
		const char *arg = buff_ptr->arg_str;
		int ret = GetNumericLimitedFromArg(arg, &strval, 255);
		
		//If any error has occured, print a message and exit (also check if 
		//value is negative)
		if(strval < 0) ret = -2;
		if(ret != 0)
		{
			if(ret == -1) PrintErrorAndExit("Buffer Size", invalid_num_str, arg);
			if(ret == -2) PrintErrorAndExit("Buffer Size", out_of_range, arg);
		}
		
		//Otherwsie set the configuration value. Cast to uint8_t
		conf_buffersize = (uint8_t)strval;
	}
	
	/*** Communicate with Termios library *************************************/
	int ser_err;
	
	//Create and open a device. Error and exit if device didn't open
	SerialDevice dev;
	ser_err = Ser_OpenDevice(port_ptr->arg_str, &dev);
	
	if(ser_err != 0)
	{
		PrintErrorAndExit("Cannot open Port", strerror(ser_err),
		                   port_ptr->arg_str);
	}
	
	//Set some known parameters of the serial device
	dev.terminal.c_oflag = 0;            //Disable remapping, delays, etc
	dev.terminal.c_lflag = 0;            //Disable signaling chars, echo, etc
	Ser_SetAttr(&dev);
	
	Ser_EnableRead(true, &dev);
	Ser_IgnoreBreak(false, &dev);
	Ser_SetParity(false, false, &dev);
	Ser_TwoStopBit(false, &dev);
	Ser_EnableSoftwareControl(false, &dev);
	Ser_EnableHardwareControl(false, &dev);
	Ser_SetVmin(0, &dev);
	
	//Set the user configured parameters
	Ser_SetBaud(conf_baud, &dev);
	Ser_SetBits(conf_bitlength, &dev);
	Ser_SetVtime(conf_timeout, &dev);
	
	//Adaptive wait for the device to be ready
	//TODO
	printf("Waiting for %i\n", conf_wait);
	
	sleep(2);
	
	/*** Write/Read from the Serial Device ************************************/
	//Write the message given to the PORT. Append newline if -nl is detected
	//TODO check err state, auto redirect on !0 maybe 
	ser_err = Ser_WriteBuffer(mesg_ptr->arg_str, strlen(mesg_ptr->arg_str), &dev);
	if(nlin_ptr->detected) Ser_WriteBuffer("\r\n", 2, &dev);
	
	//TODO wait for serial to finish writing & end device to catch up with life
	sleep(1);
	
	
	
	
	char read_buffer[conf_buffersize];	
	
	//TODO
	Ser_ReadBuffer(read_buffer, conf_buffersize, &dev);
	
	
	
	printf("%s", read_buffer);

	//TODO put this in a catch block for signals
	Ser_CloseDevice(&dev);
	
	return 0;
}



/*** Function Definitions *****************************************************/
int GetNumericLimitedFromArg(const char *str, long *val, const long limit)
{
	//Check String is valid numeric
	if(Clam_IsArgStrNumeric(str) == false) return -1;
	
	//NOTE: Passing NULL to end_ptr, we wont be looking for multiple values
	long strint = strtol(str, NULL, 10);
	
	//Check if the value was out of range
	if(strint > limit) return -2;
	
	//Set the pointer to the received value
	*val = strint;
	return 0;
}

void PrintErrorAndExit(const char *funct, const char *reason, const char *aux)
{
	fprintf(stderr, "Error: %s: %s", funct, reason);
	
	//Print the aux string in " ", only if it is not empty
	if(aux[0] != '\0') fprintf(stderr, " \'%s\'", aux);
	
	fprintf(stderr, ". %s\n", help_prompt_str);
	
	exit(EXIT_FAILURE);
}
