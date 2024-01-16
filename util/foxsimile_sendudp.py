import socket, sys

local_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
local_sock.bind(("127.0.0.118", 9999))
local_sock.settimeout(1.0)		# timeout receive after 1 second.

remote_endpoint = ("127.0.0.8", 9999)

response_len = 4

while True:
	
	# get user input
	bytes_to_send_str = input("enter bytes to send > ")

	# handle exit
	if bytes_to_send_str == "q":
		print("exiting")
		local_sock.close()
		sys.exit()

	# convert input string to command
	bytes_str_arr = bytes_to_send_str.split()
	bytes_to_send = [int(bs, 16) for bs in bytes_str_arr]
	print("\tsending " + str(bytes_to_send))

	# send command
	local_sock.sendto(bytes(bytes_to_send), remote_endpoint)
	print("\tsent")
