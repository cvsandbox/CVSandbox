The folder contains a sample Lua script demonstrating usage of the Serial Port
communication device plug-in. For this example, an Arduino board can be used
running the [Arduino_Serial.ino](Arduino_Serial.ino) sketch. Once the board is
flashed with the sketch and is up and running, the [serial.lua](serial.lua)
script can be run (provided it is updated with correct COM port number).

To run the sample script:  
cvssr.exe serial.lua

Similar script can be run from CVSandbox application. However, due to communication
delays, it is not recommended to run it as video processing step. Instead, it should
run within a scripting thread.
