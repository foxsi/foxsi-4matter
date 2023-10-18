import socket, struct, sys

housekeeping_filename = "util/log/housekeeping.log";
housekeeping_file = open(housekeeping_filename, "wb+");

cdte_filename = "util/log/cdte.log";
cdte_file = open(cdte_filename, "wb+");

cdte_queue = bytearray()

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# multicast group address and port
local_addr = '192.168.1.118'
local_port = 9999

# bind socket to mcast_grp on mcast_port
sock.bind((local_addr, local_port))

def reframe(data, queue):
	# [sys] [npackets MSB, packindex LSB] [packindex MSB, packindex LSB] [0x00 x 3] [payload]
	# npackets = (data[1] << 8) + data[2]
	# ipacket = (data[3] << 8) + data[4]

	# print("typeof data: " + str(type(data)))
	# print("just one little byte: " + str(int.from_bytes(data[0]), byteorder='big'))
	
	npackets = int.from_bytes(data[1:3], byteorder='big')
	ipacket = int.from_bytes(data[3:5], byteorder='big')

	print("npackets: " + str(npackets) + ", ipacket: " + str(ipacket))
	
	if ipacket <= npackets:
		queue.extend(data[8:])
	else:
		print("error! bad packet number")

def try_write_cdte(queue):
	
	if len(queue) >= 32780:
		cdte_file.write(queue)
		print("wrote " + str(len(queue)) + " bytes to cdte log")
		return True
	else:
		return False

while True:
	try:
		data, sender_endpoint = sock.recvfrom(2048)  #receive data
		# print(str(sender_endpoint[0]) + ":" + str(sender_endpoint[1]) + " sent " + data.hex())
		if data[0] == 0x02:
			# housekeeping system
			# print(str(sender_endpoint[0]) + ":" + str(sender_endpoint[1]) + " sent " + data[0:7].hex())
			housekeeping_file.write(data[8:])
		elif data[0] == 0x09:
			# cdte1 system
			reframe(data, cdte_queue)
			if try_write_cdte(cdte_queue):
				cdte_queue = bytearray()
			# cdte_file.write(data)
			# cdte_file.write(b"\x0a\x0a\x0a\x0a\x0a\x0a\x0a\x0a")
		else:
			print("system fell through!")

	except KeyboardInterrupt:
		sock.close()
		sys.exit()
