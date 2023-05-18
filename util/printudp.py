import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

address = "192.168.1.8"
port = 9999

endpoint = (address, port)
sock.bind(endpoint)

while True:
	data, sender_endpoint = sock.recvfrom(1500)
	print(str(sender_endpoint[0]) + ":" + str(sender_endpoint[1]) + " sent 0x" + data.hex())