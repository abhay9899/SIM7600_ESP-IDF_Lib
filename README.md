# SIM7600 Test Example

This example demostrates how to use SIM7600 LTE Module with ESP-IDF. The module works with UART and AT Commands.

## How to use example

To use this example and the library, please assign the Tx and Rx pins in the "sim7600.h" file along with the PWR Pin and RTS Pin 
if your project reqires to control the modules power and reset functionality as well.

NOTE:
	- Plese refer the begining of "sim7600.c" and "sim7600.h" files to assign required data for your proejcts application.

### Hardware Required

- This example requires only a single target (e.g., an ESP32, ESP32-S3 or ESP32-S2).
- This connection usually consists of a TX and an RX signal (PWR and RST Pins optional)
- Rx, Tx, PWR, RST Pins must be connected with SIM7600 module or your custom board consisting of SIM7600.


### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(Replace PORT with the name of the serial port to use.)

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.


## Example Breakdown

- In this example, GPIO and UART (UART PORT 2) gets initialized first for communication with SIM7600 and the chosen baud rate is 115200.
- Then, SIM7600 gets powered up and initialized and also reset sequence gets called to power it off and on again for demonstration.
- Also, LTE and cellular data mode gets initialized for internet connection and its applucation (HTTP, FTP, MQTT etc.).
- Also, GPS gets initilized and in the main loop, GPS retrival function is called and displays coordinates if GPS signal gets fixed!


## Important Notice

Please refer and go through the library file and its APIs and use it according to your application use cases. Most of the functionalities are 
not used in the example file and its purely dependent on your application. You are free to use only the library files and edit it accordingly 
in your other projects as well but be caseful with the include files and pin assignments!
