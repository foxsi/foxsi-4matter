# forwarding

This example instantiates some `Subsystem` objects and demonstrates packet **forwarding** from a port to the ground.

This can be used to demonstrate unidirectional asynchronous listening on a TCP port, and forwarding of those received packets over UDP. Note that you can receive and forward to the same remote (for demonstration purposes). The flight use-case here is the Formatter listening to the SPMU-001, and forwarding those received packets to the ground over UDP.