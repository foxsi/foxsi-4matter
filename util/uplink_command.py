import socket, sys

local_endpoint = ("127.0.0.118", 9999)
formatter_endpoint = ("127.0.0.8", 9999)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(local_endpoint)
sock.connect(formatter_endpoint)

print('to use, input uplink commands like this, with bytes separated by a space:\n\t > 0x0e 0x0a')
while True:
    try:
        input_str = input('enter a command string:\n\t> ')
        result = input_str.split(' ')
        if len(result) > 2:
            print("too many bytes found, can't send it!")
            continue

        system = int(result[0], base=16)
        command = int(result[1], base=16)
        sock.send(bytes([system,command]))
        print("sent!")
    except KeyboardInterrupt:
        socket.close()
        print("exiting.")
        break
