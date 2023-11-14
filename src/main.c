/*******************************************************************************
* This file is part of sqirt: Serial Query Interface Response Tool
* A simple program to send a request to a Serial Device, and return the response
* string back to the user
* Specifically designed for desktop and embedded environments
*
* See the GitHub for more information: https://github.com/ADBeta/sqirt
*
* ADBeta (c)    Version 1.1.2   14 Nov 2023
*******************************************************************************/
//Which sleep method to use: usleep (outdated) or nanosleep
//#define SLEEP_MODE_USLEEP
#define SLEEP_MODE_NANOSLEEP

#if defined(SLEEP_MODE_USLEEP) && defined(SLEEP_MODE_NANOSLEEP)
#error You must not have both usleep and nanosleep methods active at once
#endif

/******************************************************************************/
#ifdef SLEEP_MODE_NANOSLEEP
#include <time.h>
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "serial.h"
#include "args.h"

#define ARG_COUNT 10

///TODO Add file input thingy

/*** String definitions *******************************************************/
const char *const help_prompt_str = "Try 'sqirt -h' for more information.";
const char *const help_str = "\
sqirt\tSerial Query Interface Response Tool \n\
Sends a message to a Serial PORT, then echos its reponse to stdout\n\n\
Basic Usage: sqirt -p [port] -m [message] [OPTIONAL]\n\
Example: sqirt -p /dev/ttyUSB0 -m \"Hello World!\" -nl\n\n\
Arguments:\n\
  -p\tWhich PORT to use (REQUIRED)\n\
  -m\tMessage to send. Use \"\" or \'\' for spaces or special characters (REQUIRED)\n\
\n\
  -br\tBaudrate. Valid Options: 2400, 4800, 9600, 19200, 38400, 115200 (Default: 115200)\n\
  -td\tDelay before Transmitting to PORT. Valid Options: 0-1000 (0.1 sec increments) (Default: 0)\n\
  -rd\tDelay before Receiving the response from PORT. Valid Options: 0-1000(0.1 sec increments) (Default:  5)\n\
  -to\tTimeout for the PORT to respond. Valid Options: 0-255 (0.1 sec increments) (Default: 5)\n\
  -bl\tBit Length of the PORT. Valid Options: 5, 6, 7, 8 (Default: 8)\n\
  -bs\tBuffer Size of the response string (Default: 256)\n\
  -fi\nTransmit a file instead of a message (TODO)\n\
\nFlags:\n\
  -nl\tAppends NewLine (\"\\r\\n\") to the message automatically\n\
  -h\tShow this help message\n\
\n\nSee the GitHub for more Information. <https://github.com/ADBeta/sqirt>\n\
sqirt Version TODO    (c) ADBeta Nov 2023\n"; //TODO

const char *const out_of_range = "Value is out of range:";
const char *const invalid_num_str = "String is not a valid Numeric String:";

/*** Function pre-declarations ************************************************/
//Wait for a specified amount of Increments (0.1 seconds)
//An old usleep implimentation and a newer nanosleep version are included for
//devices that lack nanosleep.
void WaitIncrement(const size_t inc);

//Takes a clam_arg string and pointer to a long; sets the ptr to the numeric 
//value of the string. Returns int error codes:
//0    No Error
//-1   String is not a valid numeric string
//-2   Value exceeded the limit value
int GetNumericLimitedFromArg(const char *str, long *val, const long limit);

//Prints an error message to stderr, then exits the program. Pass function the
//error happened in, and the reason it happened.
void PrintErrorAndExit(const char *pri, const char *sec, const char *ter);

/*** Main Program *************************************************************/
int main(int argc, char *argv[])
{
	/*** Serial Device User Configurable Parameter Pre-definition *************/
	unsigned int conf_baud = B115200;
	unsigned int conf_txdelay = 0;
	unsigned int conf_rxdelay = 5;
	uint8_t conf_timeout = 5;
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
	ArgDef_t *tdel_ptr = Clam_AddDefinition(CLAM_TSTRING, "-td");
	ArgDef_t *rdel_ptr = Clam_AddDefinition(CLAM_TSTRING, "-rd");
	ArgDef_t *time_ptr = Clam_AddDefinition(CLAM_TSTRING, "-to");
	ArgDef_t *bits_ptr = Clam_AddDefinition(CLAM_TSTRING, "-bl");
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
	int errval = Clam_ScanArgs(argc-1, argv+1);
	if(errval != 0)
	{
		PrintErrorAndExit(strclamerr(clamerr), argv[errval], "");
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
		PrintErrorAndExit("You must specify a port with -p", "", "");
	}
	
	if(mesg_ptr->detected == false)
	{
		PrintErrorAndExit("You must specify a message with -m", "", "");
	}
	
	/*** Detect and handle User Configurable Parameters ***********************/
	//Badrate
	if(baud_ptr->detected)
	{
		long strval = 0;
		int ret = GetNumericLimitedFromArg(baud_ptr->arg_str, &strval, 115200);
		
		if(ret == -1) PrintErrorAndExit("Baudrate", baud_ptr->arg_str,
		                                 invalid_num_str);
	
		//Check input matches supported Baudrate values
		switch(strval)
		{
			case 2400:
				conf_baud = B2400;
				break;
			case 4800:
				conf_baud = B4800;
				break;
			case 9600:
				conf_baud = B9600;
				break;
			case 19200:
				conf_baud = B19200;
				break;
			case 38400:
				conf_baud = B38400;
				break;
			case 115200:
				conf_baud = B115200;
				break;
			
			default:
				PrintErrorAndExit("Baudrate", baud_ptr->arg_str, 
				"Not a valid Baudrate");
		}
	}
	
	//Transmit Delay
	if(tdel_ptr->detected)
	{
		long strval = 0;
		const char *arg = tdel_ptr->arg_str;
		int ret = GetNumericLimitedFromArg(arg, &strval, 1000);
		
		//If any error has occured, print a message and exit (also check if 
		//value is negative)
		if(strval < 0) ret = -2;
		if(ret != 0)
		{
			if(ret == -1) PrintErrorAndExit("TX Delay", arg, invalid_num_str);
			if(ret == -2) PrintErrorAndExit("TX Delay", arg, out_of_range);
		}
		
		//Otherwsie set the configuration value. Cast to unsigned int
		conf_txdelay = (unsigned int)strval;
	}
	
	//Receive Delay
	if(rdel_ptr->detected)
	{
		long strval = 0;
		const char *arg = rdel_ptr->arg_str;
		int ret = GetNumericLimitedFromArg(arg, &strval, 1000);
		
		//If any error has occured, print a message and exit (also check if 
		//value is negative)
		if(strval < 0) ret = -2;
		if(ret != 0)
		{
			if(ret == -1) PrintErrorAndExit("RX Delay", arg, invalid_num_str);
			if(ret == -2) PrintErrorAndExit("RX Delay", arg, out_of_range);
		}
		
		//Otherwsie set the configuration value. Cast to unsigned int
		conf_rxdelay = (unsigned int)strval;
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
			if(ret == -1) PrintErrorAndExit("Timeout", arg, invalid_num_str);
			if(ret == -2) PrintErrorAndExit("Timeout", arg, out_of_range);
		}
		
		//Otherwsie set the configuration value. Cast to uint8_t
		conf_timeout = (uint8_t)strval;
	}
	
	//Bit Length
	if(bits_ptr->detected)
	{
		long strval = 0;
		int ret = GetNumericLimitedFromArg(bits_ptr->arg_str, &strval, 8);
		
		if(ret == -1) PrintErrorAndExit("Bit Length", bits_ptr->arg_str,
		                                 invalid_num_str);
	
		//Check input matches supported Baudrate values
		switch(strval)
		{
			case 5:
				conf_bitlength = CS5;
				break;
			case 6:
				conf_bitlength = CS6;
				break;
			case 7:
				conf_bitlength = CS7;
				break;
			case 8:
				conf_bitlength = CS8;
				break;
			default:
				PrintErrorAndExit("Bit Length", bits_ptr->arg_str, 
				                  "Not a valid Bit Length");
		}
	}
	
	//Buffer Size
	if(buff_ptr->detected)
	{
		long strval = 0;
		const char *arg = buff_ptr->arg_str;
		int ret = GetNumericLimitedFromArg(arg, &strval, 1<<16); //64KiB
		
		//If any error has occured, print a message and exit (also check if 
		//value is negative)
		if(strval < 0) ret = -2;
		if(ret != 0)
		{
			if(ret == -1) PrintErrorAndExit("Buffer Size", arg, invalid_num_str);
			if(ret == -2) PrintErrorAndExit("Buffer Size", arg, out_of_range);
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
		PrintErrorAndExit("Cannot open Port", port_ptr->arg_str, 
		                  strerror(ser_err));
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
	
	//Wait for an amount of time specified by Transmit Delay before sending data
	WaitIncrement(conf_txdelay);
	
	/*** Write/Read from the Serial Device ************************************/
	//Write the message given to the PORT. Append newline if -nl is detected	
	ser_err = Ser_WriteBuffer(mesg_ptr->arg_str, strlen(mesg_ptr->arg_str), &dev);
	if(nlin_ptr->detected) Ser_WriteBuffer("\r\n", 2, &dev);
	
	if(ser_err != 0)
	{
		PrintErrorAndExit("Cannot Write to Port", port_ptr->arg_str, 
		                  strerror(ser_err));
	}
	
	//Wait for an amount of time specified in rxdelay, this is to the PORT can
	//Compute the transmitted message and be ready to transmit back
	WaitIncrement(conf_rxdelay);
	
	//Read the response from the Serial Port into a buffer of specified size
	char resp_buffer[conf_buffersize];
	//Read from the PORT into the buffer
	ssize_t byte_count = Ser_ReadBuffer(resp_buffer, conf_buffersize, &dev);
	if(byte_count < 0)
	{
		PrintErrorAndExit("Cannot Read from Port:", port_ptr->arg_str,
			strerror(ser_err));
	}
	
	printf("Read %li bytes\n", byte_count); //TODO
		
	//Implant a '\0' into the string so further operations can be done.
	//If the chars read is less than the buffer, add it one past the string end
	//If the string has filled the buffer, put it at the last char of the buffer
	if(byte_count < (ssize_t)conf_buffersize)
	{
		resp_buffer[byte_count] = '\0';
	} else 
	{
		resp_buffer[conf_buffersize] = '\0';
	}
	
	//Print the buffer received to stdout
	fprintf(stdout, "%s", resp_buffer);

	//Done
	Ser_CloseDevice(&dev);	
	return 0;
}

/*** Function Definitions *****************************************************/
void WaitIncrement(const size_t inc)
{
	if(inc == 0) return;

	static size_t last_inc = 0;
	
	//(obsolute) usleep implimentation for old/badly supported devices (Onion)
	#ifdef SLEEP_MODE_USLEEP
	static useconds_t usec = 0;
	if(inc != last_inc)
	{
		usec = (useconds_t)inc * 100000;
		last_inc = inc;
	}
	usleep(usec);
	#endif
	
	//nanosleep implimentation for modern systems (recommended)
	#ifdef SLEEP_MODE_NANOSLEEP
	static struct timespec time, rem;
	if(inc != last_inc)
	{
		//Split seconds and nanoseconds
		time.tv_sec = (long)inc / 10;
		time.tv_nsec = ((long)inc % 10) * 100000000;
		last_inc = inc;
	}
	nanosleep(&time, &rem);
	#endif
}

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

void PrintErrorAndExit(const char *pri, const char *sec, const char *ter)
{
	//Always print the Primary string
	fprintf(stderr, "Error: %s ", pri);
	
	//Only print secondary and tertiary if given. Secondary is inside quotes
	if(sec[0] != '\0') fprintf(stderr, "\'%s\' ", sec);
	if(ter[0] != '\0') fprintf(stderr, "%s ", ter);
	
	fprintf(stderr, "\n%s\n", help_prompt_str);
	
	exit(EXIT_FAILURE);
}
