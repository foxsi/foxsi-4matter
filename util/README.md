# util

This folder includes some high-level utilities for Formatter actions.

## Contents:

1. [spipower.py](spipower.py) is a Python script to control (via command line) the MAX7317 serial-to-parallel chip on the power distribution board. You can use this to enable/disable power output from the board to specific subsystems.
2. [monitortemp.py](monitortemp.py) is a Python script that prints the Raspberry Pi's CPU temperature to the command line at 5-second intervals.
3. [udpecho.py](udpecho.py) is a Python script that echoes received UDP packets back to the sender.
4. [udpgarbage.py](udpgarbage.py) is a Python script that continuously transmits roughly 18 Mbps of garbage data to the ground station.

## [spipower.py](spipower.py)

This script is intended to be used with the Power Distribution board developed at UMN. To use, connect Raspberry Pi SPI0 (GPIOs 8-11, pins 24, 21, 19, 23) to the Power Distribution board. Run this script and follow the command prompt to turn systems on and off.

## [monitortemp.py](monitortemp.py)

This script prints out the CPU temperature every 5 seconds.