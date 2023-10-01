/*******************************************************************************
* Serial handler - Manages and configures the termios terminal device
* ADBeta (c)2023
*******************************************************************************/
#include <termios.h>
#include <fcntl.h> 
#include <unistd.h>

#include <errno.h>
#include <string.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "serial.h"

int ser_open_device(char *filename, serial_device_t *dev)
{
	dev->filename = filename;
	//Open the given filename (Serial Port name) as read/write tty, with sync
	dev->filedesc = open(dev->filename, O_RDWR | O_NOCTTY | O_SYNC);
	
	//Make sure the file opened correctly
	if(dev->filedesc < 0)
	{
		printf("Error Opening %s: %s\n", dev->filename, strerror(errno));
		return 1;
	}
	
	return ser_get_attr(dev);
}

int ser_write_buffer(const char *buff, const unsigned int len,
                     serial_device_t *dev)
{
	int ret = write(dev->filedesc, buff, len);
	if(ret < 0)
	{
		printf("Error Writing to Serial Device: %s\n", strerror(errno));
	}
	
	return ret;
}

int ser_read_buffer(char *buff, const unsigned int len, serial_device_t *dev)
{
	int ret = read(dev->filedesc, buff, len);
	if(ret < 0)
	{
		printf("Error Reading from Serial Device: %s\n", strerror(errno));
	}
	
	return ret;
}

/*** Serial Setings & variable handling ***************************************/
int ser_get_attr(serial_device_t *dev)
{
	int ret = tcgetattr(dev->filedesc, &dev->terminal); 
	if(ret != 0) 
	{
		printf("Error getting terminal attributes: %s\n", strerror(errno));
	}
	
	return ret;
}

int ser_set_attr(serial_device_t *dev)
{
	//Force an attribute update now
	int ret = tcsetattr(dev->filedesc, TCSANOW, &dev->terminal);
	if(ret != 0)
	{
		printf("Error setting terminal attributes: %s\n", strerror(errno));
	}
	
	return ret;
}

int ser_set_baud(const int baud, serial_device_t *dev)
{
	if(cfsetospeed(&dev->terminal, baud) != 0)
	{
		printf("Error setting output baudrate: %s\n", strerror(errno));
		return -1;
	}
	
	if(cfsetispeed(&dev->terminal, baud) != 0)
	{
		printf("Error setting input baudrate: %s\n", strerror(errno));
		return -1;
	}
	
	return ser_set_attr(dev);
}

int ser_set_bits(const int bits, serial_device_t *dev)
{
	//Unset all bitlength flags
	dev->terminal.c_cflag &= ~(CS5 | CS6 | CS7 | CS8);

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
			printf("Error in ser_set_bits: Input not a valid baud speed\n");
			return -2;
	}
	
	return ser_set_attr(dev);
}

int ser_set_vmin(const uint8_t bytes, serial_device_t *dev)
{
	dev->terminal.c_cc[VMIN] = bytes;
	return ser_set_attr(dev);	
}

int ser_set_vtime(const uint8_t time, serial_device_t *dev)
{
	dev->terminal.c_cc[VTIME] = time;
	return ser_set_attr(dev);
}

int ser_enable_software_control(const bool en, serial_device_t *dev)
{
	if(en == true)
	{
		dev->terminal.c_iflag |= (IXON | IXOFF | IXANY);
	} else {
		dev->terminal.c_iflag &= ~(IXON | IXOFF | IXANY);
	}
	
	return ser_set_attr(dev);
}

int ser_enable_hardware_control(const bool en, serial_device_t *dev)
{
	if(en == true)
	{
		dev->terminal.c_cflag |= CRTSCTS;
		dev->terminal.c_cflag &= ~CLOCAL;
	} else {
		dev->terminal.c_cflag &= ~CRTSCTS;
		dev->terminal.c_cflag |= CLOCAL;
	}
	
	return ser_set_attr(dev);
}

int ser_two_stop_bit(const bool en, serial_device_t *dev)
{
	if(en == true)
	{
		dev->terminal.c_cflag |= CSTOPB;
	} else {
		dev->terminal.c_cflag &= ~CSTOPB;
	}
	
	return ser_set_attr(dev);
}

int ser_enable_read(const bool en, serial_device_t *dev)
{
	if(en == true)
	{
		dev->terminal.c_cflag |= CREAD;
	} else {
		dev->terminal.c_cflag &= ~CREAD;
	}
	
	return ser_set_attr(dev);
}

int ser_ignore_break(const bool en, serial_device_t *dev)
{
	if(en == true)
	{
		dev->terminal.c_iflag |= IGNBRK;
	} else {
		dev->terminal.c_iflag &= ~IGNBRK;
	}
	
	return ser_set_attr(dev);
}

int ser_set_parity(const bool odd, const bool even, serial_device_t *dev)
{
	dev->terminal.c_cflag &= ~(PARODD  | PARENB);
	
	dev->terminal.c_cflag |= (PARODD * odd);
	dev->terminal.c_cflag |= (PARENB * even);
	
	return ser_set_attr(dev);
}
