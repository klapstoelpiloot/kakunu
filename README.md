KlikAanKlikUit RF Communication Tools
=====================================
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


