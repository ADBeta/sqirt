/*******************************************************************************
* Serial handler - Manages and configures the termios terminal device
* ADBeta (c)2023
*******************************************************************************/

#include <termios.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	char *filename;              //Filename string
	struct termios terminal;     //Serial Terminal data
	int filedesc;                //File Descriptor
	
} serial_device_t;

/*** High Level Serial Management *********************************************/
//Opens the termios serial bus. NOTE THIS MUST BE DONE BEFORE MODIFYING VALUES
int ser_open_device(char *filename, serial_device_t *);

int ser_write_buffer(const char *buf, const unsigned int len, serial_device_t *);

int ser_read_buffer(char *buf, const unsigned int len, serial_device_t *);

//Close the termios serial bsus device. TODO
int ser_close_device(serial_device_t *);

/*** Serial Setings & variable handling ***************************************/
//Manually set or get the termios variables
int ser_get_attr(serial_device_t *);
int ser_set_attr(serial_device_t *);

//Sets the baudrate to use on the serial bus
int ser_set_baud(const int baud, serial_device_t *);

//Sets the bit depth/length of the serial bus
//Options: CS5    CS6    CS7    CS8
int ser_set_bits(const int bits, serial_device_t *);

//Set minimum bytes to receive before allowing a read to return (non-canonical)
//Values from 0 - 255
int ser_set_vmin(const uint8_t bytes, serial_device_t *);

//Set timeout length on the serial bus
//Values 0 - 255    0.1 second incriments    0s - 25.5s
int ser_set_vtime(const uint8_t time, serial_device_t *);

//Sets if software control is enabled.
//Modifies IXON    IXOFF    IXANY
int ser_enable_software_control(const bool en, serial_device_t *);

//Sets if hardware control is enabled
//Modifies CRTSCTS    CLOCAL
int ser_enable_hardware_control(const bool en, serial_device_t *);

//Sets how many stop bits to use.
//One Stop Bit: false        Two Stop Bits: true
int ser_two_stop_bit(const bool en, serial_device_t *);

//Enables the CREAD flag, which enables or disables receiving
int ser_enable_read(const bool en, serial_device_t *);

//Sets if the bus should ignore a BREAK signal or not (commonly false)
int ser_ignore_break(const bool en, serial_device_t *);

//Sets what parity to use on the serial bus.
int ser_set_parity(const bool odd, const bool even, serial_device_t *);
