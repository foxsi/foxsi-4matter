import socket, time, random, struct

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# multicast group address and port
mcast_grp = '224.1.1.0'
mcast_port = 3000
  
# I have to bind the socket to all multicast groups instead of the specific mcast_grp to get it to work. Not sure why.
sock.bind(('',mcast_port))
# convert multicast address to binary form, INADDR_ANY means any interface
mreq = struct.pack("4sl", socket.inet_aton(mcast_grp), socket.INADDR_ANY)
# join the multicast group
sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

while True:
	# receive data from source
	data, sender_endpoint = sock.recvfrom(1500)
	# data will be redirected to the same multicast address but with a port number incremented by 1
	addr_redirect = (mcast_grp, mcast_port+1)
	print("Redirecting data from "+ str(sender_endpoint) + " to " + str(addr_redirect))
	# forward data
	sent = sock.sendto(data, addr_redirect)