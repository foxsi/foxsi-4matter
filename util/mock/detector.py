import sys, time, socket, detector_response

local_port = 10030
local_address = "127.0.0.3"

remote_port = 10030
remote_address = "127.0.0.1"

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind((local_address, local_port))
sock.connect((remote_address, remote_port))

# a key for each input message, a val for each response.

try:
    while True:
        data = sock.recv(4096)
        print(type(data))
        for key in detector_response.keys():
            if key in data:
                # add some delay, like you're thinking hard
                sock.send(detector_response[key])
            
except KeyboardInterrupt:
    sock.shutdown(socket.SHUT_RDWR)
    sock.close()
    sys.exit()



    



