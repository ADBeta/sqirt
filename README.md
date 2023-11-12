# sqirt [Serial Query TODO Response Tool]
sqrt is a light and simple program, intended for embedded applications, that
takes a `message` and transmits it to the `port` given, it then receives a
response from the serial device and prints it to the stdout stream.

## Tested Platforms
x86
OpenWRT_MIPSEL



To pass a string with a newline (`\r\n`) you can either use `-nl` as an argument
or use Linux/Bash escaping, `-m $'Hello World\r\n'`

## TODO
* Add parity, hardware/software control stop bits and break flags
* add a "file in" (-fi) flag to write the content of a file to the serial port

----
<b> 2023 ADBeta </b>  
This software is under the GPL 2.0 Licence, please see LICENSE for information
