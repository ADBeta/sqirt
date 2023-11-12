/*******************************************************************************
* Serial handler - Manages and configures the termios terminal device
* This library is intended for use in desktop, embedded and many other projects
* Please see the GitHub for more information:
*
* PLEASE NOTE: Most API functions of this library return errno values.
* (c) ADBeta 2023
*******************************************************************************/
#include <termios.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	const char *filename;        //Filename string
	struct termios terminal;     //Serial Terminal data
	int filedesc;                //File Descriptor
} SerialDevice;

/*** High Level Serial Management *********************************************/
//Opens the termios serial bus. NOTE THIS MUST BE DONE BEFORE MODIFYING VALUES
int Ser_OpenDevice(const char *filename, SerialDevice *);
//Close the termios serial bsus device.
int Ser_CloseDevice(SerialDevice *dev);

//Write a buffer [buf] of length [len] to a Serial Device,
//Returns errno (=0 if ok)
int Ser_WriteBuffer(const char *buf, const size_t len, SerialDevice *);

//Read a buffer [buf] from a Serial Device of length [len]
//Returns bytes read, is this is -1, an error occured. See errno
ssize_t Ser_ReadBuffer(char *buf, const size_t len, SerialDevice *);

/*** Serial Setings & variable handling ***************************************/
//Manually set or get the termios variables
int Ser_GetAttr(SerialDevice *);
int Ser_SetAttr(SerialDevice *);

//Sets the baudrate to use on the serial bus
int Ser_SetBaud(const unsigned int baud, SerialDevice *);

//Sets the bit depth/length of the serial bus
//Options: CS5    CS6    CS7    CS8
int Ser_SetBits(const unsigned int bits, SerialDevice *);

//Set minimum bytes to receive before allowing a read to return (non-canonical)
//Values from 0 - 255
int Ser_SetVmin(const uint8_t bytes, SerialDevice *);

//Set timeout length on the serial bus
//Values 0 - 255    0.1 second incriments    0s - 25.5s
int Ser_SetVtime(const uint8_t time, SerialDevice *);

//Sets if software control is enabled.
//Modifies IXON    IXOFF    IXANY
int Ser_EnableSoftwareControl(const bool en, SerialDevice *);

//Sets if hardware control is enabled
//Modifies CRTSCTS    CLOCAL
int Ser_EnableHardwareControl(const bool en, SerialDevice *);

//Sets how many stop bits to use.
//One Stop Bit: false        Two Stop Bits: true
int Ser_TwoStopBit(const bool en, SerialDevice *);

//Enables the CREAD flag, which enables or disables receiving
int Ser_EnableRead(const bool en, SerialDevice *);

//Sets if the bus should ignore a BREAK signal or not (commonly false)
int Ser_IgnoreBreak(const bool en, SerialDevice *);

//Sets what parity to use on the serial bus.
int Ser_SetParity(const bool odd, const bool even, SerialDevice *);
