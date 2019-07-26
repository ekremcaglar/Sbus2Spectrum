# Sbus2Spectrum

(sbus) to spectrum rc protokol converter

desertrotor is a ground control station producer.
It has two outputs for remotely controling drones. (PPM and somehow encapsulated serial Sbus)

Whenever you try to control a drone having spectrum type rc input (intel aero in my case)
you need a protocol converter.

Since I am unable to get ready one I wrote my own.

An ardunio board receives desertrotor serial output with parameters 115200 kbps, 8N1.
Desertrotor outputs 31 bytes in each frame.
Frame structrure is in the format
0x55 0xAA 0x1D 0x1D 0xXX SBUS 2byte CRC
SBUS is well defined 25 bytes long frame structure

spectrum1024 protocol consist of 16 bytes.

Ardunio program converts desertrotor to spectrum1014 protocol. 


