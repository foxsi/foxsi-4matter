import socket, time, random

local_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
local_sock.bind(("192.168.1.118", 9999))

remote_endpoint = ("192.168.1.8", 9999)

# set up the CMOS:
# 0x0e 0x18
# 0x0e 0x1f
# 0x0e 0x10
# 0x0e 0x12

local_sock.sendto([0x0e, 0x18], remote_endpoint)	# start_cmos_init
local_sock.sendto([0x0e, 0x1f], remote_endpoint)	# start_cmos_training
local_sock.sendto([0x0e, 0x10], remote_endpoint)	# set_cmos_params
local_sock.sendto([0x0e, 0x12], remote_endpoint)	# start_cmos_exposure

# readout period:
T = 125/1000.0

while True:
	start_time = time.time()
	
	cmos1_pps_read_pps_linetime = [0x0e, 0xa1]
	
	# send command to read the PPS
	local_sock.sendto(cmos1_pps_read_pps_linetime, remote_endpoint)
	print("sent linetime request")

	# receive any response
	response = local_sock.recvfrom(4096)
	print("received response of length " + str(len(response)) + ": " + str(response))
	
	time.sleep(T - (time.time() - start_time)%T)
