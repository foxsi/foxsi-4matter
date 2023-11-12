import socket, struct, sys, math

housekeeping_filename = "log/gse/housekeeping.log";
housekeeping_file = open(housekeeping_filename, "wb");

cdte1_filename = "log/gse/cdte1.log";
cdte1_file = open(cdte1_filename, "wb");
cdte2_filename = "log/gse/cdte2.log";
cdte2_file = open(cdte2_filename, "wb");
cdte3_filename = "log/gse/cdte3.log";
cdte3_file = open(cdte3_filename, "wb");
cdte4_filename = "log/gse/cdte4.log";
cdte4_file = open(cdte4_filename, "wb");

cmos_pc_filename = "log/gse/cmos_pc.log";
cmos_pc_file = open(cmos_pc_filename, "wb");
cmos_ql_filename = "log/gse/cmos_ql.log";
cmos_ql_file = open(cmos_ql_filename, "wb");

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

local_addr = '192.168.1.118'
local_port = 9999

sock.bind((local_addr, local_port))

cdte1_queue_len = 32780
cdte1_payload_len = 1992
cdte1_queue = bytearray(cdte1_queue_len)
cdte1_queued = [0]*17
cdte1_queued_full = [1]*17

cdte2_queue_len = 32780
cdte2_payload_len = 1992
cdte2_queue = bytearray(cdte2_queue_len)
cdte2_queued = [0]*17
cdte2_queued_full = [1]*17

cdte3_queue_len = 32780
cdte3_payload_len = 1992
cdte3_queue = bytearray(cdte3_queue_len)
cdte3_queued = [0]*17
cdte3_queued_full = [1]*17

cdte4_queue_len = 32780
cdte4_payload_len = 1992
cdte4_queue = bytearray(cdte4_queue_len)
cdte4_queued = [0]*17
cdte4_queued_full = [1]*17

cmos_pc_queue_len = 590848 # for PC
cmos_pc_payload_len = 1992
cmos_pc_packets_per_frame = math.ceil(cmos_pc_queue_len/cmos_pc_payload_len)
cmos_pc_queue = bytearray(cmos_pc_queue_len)
cmos_pc_queued = [0]*297
cmos_pc_queued_full = [1]*297

cmos_ql_queue_len = 492544 # for QL
cmos_ql_payload_len = 1992
cmos_ql_packets_per_frame = math.ceil(cmos_ql_queue_len/cmos_ql_payload_len)
cmos_ql_queue = bytearray(cmos_ql_queue_len)
cmos_ql_queued = [0]*248
cmos_ql_queued_full = [1]*248

done = False

# hopefully this is no longer detector-specific
def reframe(data, payload_len, queue, queued):
	# [sys] [npackets MSB, packindex LSB] [packindex MSB, packindex LSB] [data type] [0x00 x 2] [payload]
	
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


# hopefully this is no longer detector-specific
def try_write(queue, queue_done, outfile):
	
	# if len(queue) >= 32780:
	if queue_done:
		outfile.write(queue)
		print("wrote " + str(len(queue)) + " bytes to log file") #. Surplus " + str(len(queue) - 32780))
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
			print("wrote " + str(len(data[8:])) + " bytes to log file")
		elif data[0] == 0x09:
			# cdte1 system
			done = reframe(data, cdte1_payload_len, cdte1_queue, cdte1_queued)
			if try_write(cdte1_queue, done, cdte1_file):
				cdte1_queue = bytearray(cdte1_queue_len)
				cdte1_queued = [0]*len(cdte1_queued)
				done = False
		elif data[0] == 0x0a:
			# cdte1 system
			done = reframe(data, cdte2_payload_len, cdte2_queue, cdte2_queued)
			if try_write(cdte2_queue, done, cdte2_file):
				cdte2_queue = bytearray(cdte2_queue_len)
				cdte2_queued = [0]*len(cdte2_queued)
				done = False
		elif data[0] == 0x0b:
			# cdte1 system
			done = reframe(data, cdte3_payload_len, cdte3_queue, cdte3_queued)
			if try_write(cdte3_queue, done, cdte3_file):
				cdte3_queue = bytearray(cdte3_queue_len)
				cdte3_queued = [0]*len(cdte3_queued)
				done = False
		elif data[0] == 0x0c:
			# cdte1 system
			done = reframe(data, cdte4_payload_len, cdte4_queue, cdte4_queued)
			if try_write(cdte4_queue, done, cdte4_file):
				cdte4_queue = bytearray(cdte4_queue_len)
				cdte4_queued = [0]*len(cdte4_queued)
				done = False
				
		elif data[0] == 0x0e:
			if data[5] == 0x00:
				done = reframe(data, cmos_pc_payload_len, cmos_pc_queue, cmos_pc_queued)
				if try_write(cmos_pc_queue, done, cmos_pc_file):
					cmos_pc_queue = bytearray(cmos_pc_queue_len)
					cmos_pc_queued = [0]*len(cmos_pc_queued)
					done = False
			elif data[5] == 0x01:
				done = reframe(data, cmos_ql_payload_len, cmos_ql_queue, cmos_ql_queued)
				if try_write(cmos_ql_queue, done, cmos_ql_file):
					cmos_ql_queue = bytearray(cmos_ql_queue_len)
					cmos_ql_queued = [0]*len(cmos_ql_queued)
					done = False
			else:
				print("got unidentifiable datatype in cmos packet!")
				print(data[0:8])
		else:
			print("system fell through!")

	except KeyboardInterrupt:
		sock.close()
		sys.exit()
