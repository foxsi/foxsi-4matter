import socket, time, random

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

address = "192.168.1.8"
port = 9999

endpoint = (address, port)
sock.bind(endpoint)

while True:
    # make some random data to transmit
    data = random.randbytes(1000)
    print("sending data to " + address + ":" + str(port))
    sock.sendto(data, endpoint)

    # approximate 18 Mbps downlink rate
    time.sleep(55.0/1000)