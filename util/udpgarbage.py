import socket, time, random

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)

# multicast group address and port
mcast_grp = '224.1.1.118'
mcast_port = 9999

while True:
	# make some random data to transmit
	data = random.randbytes(1000)
	print("sending data to multicast group " + mcast_grp + ":" + str(mcast_port))
	sock.sendto(data, (mcast_grp, mcast_port))
	time.sleep(55.0/1000) 
