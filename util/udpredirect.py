import socket, time, random

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

address = "192.168.1.118"
port = 9999

endpoint = (address, port)
sock.bind(endpoint)

while True:
	data, sender_endpoint = sock.recvfrom(1500)
	sender_endpoint_redirect = (sender_endpoint[0], sender_endpoint[1]+1)
	print("Redirecting data from "+ str(sender_endpoint) + " back to " + str(sender_endpoint_redirect))
	sent = sock.sendto(data, sender_endpoint_redirect)

	time.sleep(55.0/1000)