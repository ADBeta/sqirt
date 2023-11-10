/*******************************************************************************
* Serial handler - Manages and configures the termios terminal device
* This library is intended for use in desktop, embedded and many other projects
* Please see the GitHub for more information:
*
* PLEASE NOTE: Most API functions of this library return errno values.
* (c) ADBeta    Version 1.1.0    03 Nov 2023
*******************************************************************************/
#include <termios.h>
#include <fcntl.h> 
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "serial.h"

int Ser_OpenDevice(const char *filename, SerialDevice *dev)
{
	dev->filename = filename;
	//Open the given filename (Serial Port name) as read/write tty, with sync
	dev->filedesc = open(dev->filename, O_RDWR | O_NOCTTY | O_SYNC);
	
	//Make sure the file opened correctly
	if(dev->filedesc < 0) return errno;
	
	//Pre-load the attributes for the device. Returns its return value
	return Ser_GetAttr(dev);
}

int Ser_CloseDevice(SerialDevice *dev)
{
	close(dev->filedesc);
	return errno;
}

int Ser_WriteBuffer(const char *buff, const size_t len, SerialDevice *dev)
{
	write(dev->filedesc, buff, len);
	return errno;
}

int Ser_ReadBuffer(char *buff, const size_t len, SerialDevice *dev)
{
	read(dev->filedesc, buff, len);
	return errno;
}

/*** Serial Setings & variable handling ***************************************/
int Ser_GetAttr(SerialDevice *dev)
{
	tcgetattr(dev->filedesc, &dev->terminal); 
	return errno;
}

int Ser_SetAttr(SerialDevice *dev)
{
	//Force an attribute update now
	tcsetattr(dev->filedesc, TCSANOW, &dev->terminal);
	return errno;
}

int Ser_SetBaud(const unsigned int baud, SerialDevice *dev)
{
	if(cfsetospeed(&dev->terminal, baud) != 0) return errno;
	if(cfsetispeed(&dev->terminal, baud) != 0) return errno;
	
	return Ser_SetAttr(dev);
}

int Ser_SetBits(const unsigned int bits, SerialDevice *dev)
{
	//Unset all bitlength flags
	dev->terminal.c_cflag &= ~(tcflag_t)(CS5 | CS6 | CS7 | CS8);

	switch(bits)
	{
		case CS5:
			dev->terminal.c_cflag |= CS5;
			break;
		case CS6:
			dev->terminal.c_cflag |= CS6;
			break;
		case CS7:
			dev->terminal.c_cflag |= CS7;
			break;
		case CS8:
			dev->terminal.c_cflag |= CS8;
			break;
		
		default:
			return EINVAL;
	}
	
	return Ser_SetAttr(dev);
}

int Ser_SetVmin(const uint8_t bytes, SerialDevice *dev)
{
	dev->terminal.c_cc[VMIN] = bytes;
	return Ser_SetAttr(dev);	
}

int Ser_SetVtime(const uint8_t time, SerialDevice *dev)
{
	dev->terminal.c_cc[VTIME] = time;
	return Ser_SetAttr(dev);
}

int Ser_EnableSoftwareControl(const bool en, SerialDevice *dev)
{
	if(en == true)
	{
		dev->terminal.c_iflag |= (IXON | IXOFF | IXANY);
	} else {
		dev->terminal.c_iflag &= ~(tcflag_t)(IXON | IXOFF | IXANY);
	}
	
	return Ser_SetAttr(dev);
}

int Ser_EnableHardwareControl(const bool en, SerialDevice *dev)
{
	if(en == true)
	{
		dev->terminal.c_cflag |= CRTSCTS;
		dev->terminal.c_cflag &= ~(tcflag_t)CLOCAL;
	} else {
		dev->terminal.c_cflag &= ~(tcflag_t)CRTSCTS;
		dev->terminal.c_cflag |= CLOCAL;
	}
	
	return Ser_SetAttr(dev);
}

int Ser_TwoStopBit(const bool en, SerialDevice *dev)
{
	if(en == true)
	{
		dev->terminal.c_cflag |= (tcflag_t)CSTOPB;
	} else {
		dev->terminal.c_cflag &= ~(tcflag_t)CSTOPB;
	}
	
	return Ser_SetAttr(dev);
}

int Ser_EnableRead(const bool en, SerialDevice *dev)
{
	if(en == true)
	{
		dev->terminal.c_cflag |= CREAD;
	} else {
		dev->terminal.c_cflag &= ~(tcflag_t)CREAD;
	}
	
	return Ser_SetAttr(dev);
}

int Ser_IgnoreBreak(const bool en, SerialDevice *dev)
{
	if(en == true)
	{
		dev->terminal.c_iflag |= IGNBRK;
	} else {
		dev->terminal.c_iflag &= ~(tcflag_t)IGNBRK;
	}
	
	return Ser_SetAttr(dev);
}

int Ser_SetParity(const bool odd, const bool even, SerialDevice *dev)
{
	dev->terminal.c_cflag &= ~(tcflag_t)(PARODD  | PARENB);
	
	dev->terminal.c_cflag |= (PARODD * odd);
	dev->terminal.c_cflag |= (PARENB * even);
	
	return Ser_SetAttr(dev);
}
