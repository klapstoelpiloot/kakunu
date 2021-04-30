Klik Aan Klik Uit RF Communication Tools (software decoder/encoder)
===================================================================
By Pascal van der Heiden, [www.codeimp.com](http://www.codeimp.com)

These tools allow basic recording and control of KlikAanKlikUit devices over RF at 433 MHz. The software is developed for the Raspberry Pi and uses the [pigpio library](http://abyz.me.uk/rpi/pigpio/) as hardware interface.

Use **kakunu** to listen on an input pin. This outputs recognized codes to standard out. Use the **kakusend** tool to transmit a code on an output pin. For both tools you can use the --help parameter for more information about any options.

## Build environment
Included is a Visual Studio (2017) solution with 2 Linux projects. You must configure your Raspberry Pi in Visual Studio to build and run remotely on Linux. Add your device in Tools -> Options -> Cross Platform -> Connection Manager. On your Raspberry Pi you need to install these tools:
```
#: sudo apt install zip rsync openssh-server build-essential
```
You also need to install pigpio:
```
#: sudo apt-get install pigpio
```
Now you should be able to build the solution on your Raspberry Pi. If the projects have an invalid target specified, you can override it by going to the project properties in Visual Studio and select the correct target in the setting "Remote Build Machine".

Or you can skip all the Visual Studio stuff above and make your own build script. It is really just matter of compiling the .cpp files and linking. Make sure to specify `-pthread` and link the libraries `pigpiod_if2` and `rt` in a debug build or `pigpio` and `rt` in a release build.

Make sure you start the pigpiod service when running a debug build, because it uses the pigpiod_if2 interface which allows the tools to run without sudo.
```
#: sudo pigpiod
```

## Protocol
The KlikAanKlikUit protocol is a one-way digital signal with pulses of about 250 microseconds and multiples thereof. Because the communication is one-way, the remote control does not know the state of the devices and the devices do not send feedback to any signal, they only listen. A common KlikAanKlikUit remote control sends the same message 4 times to increase the chance of successful arrival.

The start of a message is indicated with this signal (T is 250 microseconds):
```
	 __             
	|  |________________________________

	|--|-------------------------------|
	 T              10T
```

The message ends with:
```
	 __             
	|  |________________________________ _ _ _ _____

	|--|-------------------------------- - - - ----|
	 T                      40T
```

A sub-bit '0' is encoded like this:
```
	 __ 
	|  |___

	|--|--|
	 T  T
```


A sub-bit '1' is encoded like this:
```
	 __ 
	|  |______________

	|--|-------------|
	 T       5T
```
Note that I call these "sub-bits", because every 2 sub-bits need to be combined to reconstruct the actual bit. A 0 bit consists of sub-bits 0 and 1 and a 1 bit consists of sub-bits 1 and 0. The extended version of the protocol (for dimmers) also has sub-bit combinations 0 0 and 1 1 which I combine in the software as a 2 and a 3 respectively. The extended protocol also adds more information bits, but I have not further decoded the signal into more meaningful information as (for now) I don't really need it (but your help to complete this is welcome!). The kakunu tool shows these codes when received and you can use those codes with the kakusend tool to mimic the signals that your remote control generates.

