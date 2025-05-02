# util

This folder includes some convenient utilities for Formatter actions.

A typical workflow is to pull from GitHub, modify some code on your laptop, try to build it, and if that works, push it over an Ethernet cable to the Formatter using [update_formatter.sh](update_formatter.sh). Then you can build the code on the Formatter (Raspberry Pi) and test it out. Maybe you find Ethernet connection issues! Then you can use some of the Python scripts to send dummy packets and validate Ethernet interfaces. The files in this `util/` folder help with that development workflow.

## Shell scripts
1. [update_formatter.sh](update_formatter.sh) will push code you have on a computer over an Ethernet connection to the Formatter. A typical workflow is to write some code This script will not push build, bin, doc, or log folders over to the Formatter. It uses `rsync`.
2. [build_doc.sh](build_doc.sh) is used to build all the autodoc-type documentation (from docstrings in the code). This builds both PDF (LaTeX) and HTML docs in `foxsi-4matter/doc/breathe/build/`. 
3. [copy_remote_logs.sh](copy_remote_logs.sh) will copy all the log fils on the Formatter into the `foxsi-4matter/log/formatter/` folder on this machine.
4. [delete_logs.sh](delete_logs.sh) deletes the logs in the `log/` folder. This doesn't do anything remotely—if you want to delete logs from the Formatter instead of your laptop, you'll need to `ssh` and run it there.
5. [assign_all_loopbacks.sh](assign_all_loopbacks.sh) will add all loopback IP addresses on your laptop. So you can send loopback not just from 127.0.0.1, but up to 127.0.0.255. This is used for mock testing in the `foxsimile` emulator. If you're doing stuff with `foxsimile` you'll need to run this first.

## Python scripts
A lot of these have to do with network testing between the GSE and Formatter. They contain hardcoded IP addresses/ports. These may or may not match the configuration in foxsi4-commands/systems.json. 

1. [spipower.py](spipower.py) is a Python script to control (via command line) the MAX7317 serial-to-parallel chip on the power distribution board. You can use this to enable/disable power output from the board to specific subsystems.
2. [monitortemp.py](monitortemp.py) is a Python script that prints the Raspberry Pi's CPU temperature to the command line at 5-second intervals.
3. [udpecho.py](udpecho.py) is a Python script that echoes received UDP packets back to the sender.
4. [udpgarbage.py](udpgarbage.py) is a Python script that continuously transmits roughly 18 Mbps of garbage data to the ground station IP address.