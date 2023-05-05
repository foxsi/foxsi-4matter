# forwarding

This example instantiates some `Subsystem` objects and demonstrates packet **forwarding** from a port to the ground.

This can be used to demonstrate bi-directional asynchronous packet forwarding through the Formatter. Received TCP packets will be forwarded over UDP, and received UDP packets will be forward over TCP.

## Using

Edit [forwarding_example.cpp](forwarding_example.cpp) to have the local machine IP address in the variable `local_ip`, and the remote machine address in the variable `ground_ip`. Set desired port numbers for these machines/systems as well in the following lines. 

Call the local machine "machine A" and the remote machine "machine B". Then, in two separate (possibly SSH'd) terminals on the remote machine, do:

```bash
% netcat -l <the-ip-of-remote-machine-B> <the-port-on-which-machine-A-listens-for-TCP>
```

and 

```bash
% netcat -ul <the-ip-of-remote-machine-B> <the-port-on-which-machine-A-listens-for-UDP>
```

Now, you can type at either terminal on machine B, hit `Enter`, and you should see your entry appear at the other machine B terminal. Your text has been routed to the local machine (machine A) and forwarded back to machine B in a new protocol.