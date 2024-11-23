import sys, socket, struct

# sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# # sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR)

# # multicast group address and port
# local_addr = '192.168.1.118'
# local_port = 9999

# # bind socket to mcast_grp on mcast_port
# sock.bind((local_addr, local_port))

# while True:
# 	data, sender_endpoint = sock.recvfrom(1500)  #receive data
# 	print(str(sender_endpoint[0]) + ":" + str(sender_endpoint[1]) + " sent " + data.hex())

if __name__ == "__main__":
	if len(sys.argv) != 3:
		print("usage:\n\t>python printudp.py ip-address port")
		sys.exit()
	

	local_addr = sys.argv[1]
	local_port = int(sys.argv[2])
	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	sock.bind((local_addr, local_port))
	while True:
		data, sender_endpoint = sock.recvfrom(1500)  #receive data
		print(str(sender_endpoint[0]) + ":" + str(sender_endpoint[1]) + " sent " + data.hex())
