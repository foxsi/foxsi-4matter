import socket, struct, sys

housekeeping_filename = "log/gse/housekeeping.log";
housekeeping_file = open(housekeeping_filename, "wb");

cdte_filename = "log/gse/cdte.log";
cdte_file = open(cdte_filename, "wb");

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

local_addr = '192.168.1.118'
local_port = 9999

sock.bind((local_addr, local_port))

queue_len = 32780
payload_len = 1992
cdte_queue = bytearray(queue_len)
cdte_queued = [0]*17
cdte_queued_full = [1]*17

done = False

def reframe(data, queue, queued):
	# [sys] [npackets MSB, packindex LSB] [packindex MSB, packindex LSB] [0x00 x 3] [payload]
	
	npackets = int.from_bytes(data[1:3], byteorder='big')
	ipacket = int.from_bytes(data[3:5], byteorder='big')

	print("npackets: " + str(npackets) + ", ipacket: " + str(ipacket))
	
	if ipacket <= npackets:
		# queue.extend(data[8:])
		this_index = (ipacket - 1)*payload_len
		distance = len(data[8:])
		# print("packet end: " + str(data[-1]) + " distance: " + str(distance) + " index: " + str(this_index))
		queue[this_index:(this_index + distance)] = data[8:]
		# print("queue end of packet: " + str(queue[this_index+distance - 1]))
		queued[ipacket - 1] = 1

		if all(item == 1 for item in queued):
			print("finished packet")
			return True
	else:
		print("error! bad packet number")

def try_write_cdte(queue, queue_done):
	
	# if len(queue) >= 32780:
	if queue_done:
		cdte_file.write(queue)
		print("wrote " + str(len(queue)) + " bytes to cdte log. Surplus " + str(len(queue) - 32780))
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
			done = reframe(data, cdte_queue, cdte_queued)
			if try_write_cdte(cdte_queue, done):
				cdte_queue = bytearray(32780)
				cdte_queued = [0]*17
				done = False
			# cdte_file.write(data)
			# cdte_file.write(b"\x0a\x0a\x0a\x0a\x0a\x0a\x0a\x0a")
		else:
			print("system fell through!")

	except KeyboardInterrupt:
		sock.close()
		sys.exit()
