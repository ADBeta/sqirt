# sqirt [Serial Query Interface Response Tool]
sqirt is a light and simple program, intended for embedded applications, that
takes a `message` and transmits it to the `port` given, it then receives a
response from the serial device and prints it to the stdout stream.

## Tested Platforms
x86
OpenWRT_MIPSel

## Usage
Standard RS232/Serial hardware controls are available, such as Baudrate `-br`,
Bit Length `-bl`, and Timeout `-to`. To add to this I have built in the 
following controls:

To adjust the received string buffer size, use `-bs` to change the buffer size.

To pass a string with a newline (`\r\n`) you can either use `-nl` as an argument
or use Unix Shell escaping, `-m $'Hello World\r\n'`  

Some devices reset when their Serial Port is accessed. For devices like this,
use the `-td (Transmit Delay)` flag to add a delay from accessing the Ports 
file descriptor, and Transmitting data to it.  

Similarly, some devices take longer than others to compute a response to the 
query, use the `-rd (Receive Delay)` flag to add a delay between Transmitting
and Receving a response.  


## TODO
* Add parity, hardware/software control stop bits and break flags

----
<b> 2023 ADBeta </b>  
This software is under the GPL 2.0 Licence, please see LICENSE for information
