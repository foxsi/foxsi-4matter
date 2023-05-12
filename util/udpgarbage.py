import socket, time, random

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

local_address = "192.168.1.8"
local_port = 9999

remote_address = "192.168.1.108"
remote_port = 10000

local_endpoint = (local_address, local_port)
remote_endpoint = (remote_address, remote_port)
sock.bind(local_endpoint)

while True:
    # make some random data to transmit
    data = random.randbytes(1000)
    print("sending data to " + local_address + ":" + str(local_port))
    sock.sendto(data, remote_endpoint)

    # approximate 18 Mbps downlink rate
    time.sleep(55.0/1000)