#include <stdio.h>

#include<unistd.h>

#include "serial.h"
#include "args.h"

serial_device_t dev;

int main(int argc, char *argv[])
{	

	/*
	ser_open_device("/dev/ttyACM0", &dev);
	
	
	//Manually change some of the termios devices variables
	dev.terminal.c_oflag = 0;            //Disable remapping, delays, etc
	dev.terminal.c_lflag = 0;            //Disable signaling chars, echo, etc
	ser_set_attr(&dev);
	
	//Use the API functions to modify the rest of the termios variables
	ser_set_baud(B9600, &dev);
	ser_set_bits(CS8, &dev);
	ser_set_vmin(0, &dev);
	ser_set_vtime(10, &dev);
	ser_enable_software_control(false, &dev);
	ser_enable_hardware_control(false, &dev);
	ser_two_stop_bit(false, &dev);
	ser_enable_read(true, &dev);
	ser_ignore_break(false, &dev);
	ser_set_parity(false, false, &dev);
	
	
	sleep(2);
	
	ser_write_buffer("Hello! :)\n", 32, &dev);
	
	sleep(1);
	
	char buffer[256];	
	ser_read_buffer(buffer, 256, &dev);
	
	printf("%s\n", buffer);
	*/
	
	
	
	
	
	
	
	
	//Pass arguments to function, excluding exec path
	scan_args(argc-1, argv+1);
	
	return 0;
}

