import socket, struct, sys

housekeeping_filename = "util/log/housekeeping.log";
housekeeping_file = open(housekeeping_filename, "wb+");

cdte_filename = "util/log/cdte.log";
cdte_file = open(housekeeping_filename, "wb+");

cdte_queue = bytearray()

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# multicast group address and port
local_addr = '192.168.1.118'
local_port = 9999

# bind socket to mcast_grp on mcast_port
sock.bind((local_addr, local_port))

def reframe(data):
	# [sys] [npackets MSB, packindex LSB] [packindex MSB, packindex LSB] [0x00 x 3] [payload]
	# npackets = (data[1] << 8) + data[2]
	# ipacket = (data[3] << 8) + data[4]
	global cdte_queue

	print("typepof data: " + str(type(data)))
	print("just one little byte: " + str(int.from_bytes(data[0:1]), byteorder='little'))
	
	npackets = (int.from_bytes(data[1:2], byteorder='little') & 0xffff)
	ipacket = (int.from_bytes(data[3:4], byteorder='little') & 0xffff)

	print("npackets: " + str(npackets) + ", ipacket: " + str(ipacket))
	
	if ipacket < npackets - 1:
		cdte_queue.append(data[8:])
	else:
		print("error! bad packet number")

def try_write_cdte():
	global cdte_queue
	
	if len(cdte_queue) >= 32780:
		cdte_file.write(cdte_queue)
		print("wrote " + len(cdte_queue) + " bytes to cdte log")
		cdte_queue = []

while True:
	try:
		data, sender_endpoint = sock.recvfrom(2048)  #receive data
		print(str(sender_endpoint[0]) + ":" + str(sender_endpoint[1]) + " sent " + data.hex())
		if data[0] == 0x02:
			# housekeeping system
			# print(str(sender_endpoint[0]) + ":" + str(sender_endpoint[1]) + " sent " + data[0:7].hex())
			housekeeping_file.write(data[8:])
		if data[0] == 0x09:
			# cdte1 system
			# reframe(data)
			# try_write_cdte()
			cdte_file.write(data)
			cdte_file.write("\n\n\n\n\n\n\n\n")

	except KeyboardInterrupt:
		sock.close()
		sys.exit()
