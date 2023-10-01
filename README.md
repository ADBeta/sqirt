# sqrt [Serial Query Response Tool]

sqrt is a light and simple program, intended for embedded applications, that
takes a `message` and transmits it to the `port` given, it then receives a
response from the serial device and prints it to the stdout stream.

-port
-baud
-message
-length
-bits
-hwcontrol?
-timeout
-break?



## TODO
* take args like this: -baud -bits etc
* take argument with spaces by looking for -message "quote

* make sure to look for second quote in the same arg, so "hello" is detected correctly
* go through all args until other " is found
* if no quote is found, throw error


----
<b> 2023 ADBeta </b>  
This software is under the GPL 2.0 Licence, please see LICENCE for information
