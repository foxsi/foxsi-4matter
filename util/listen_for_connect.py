import socket, time, sys

spmu_loopback_endpoint = ("127.0.0.100", 10030)
hk_loopback_endpoint = ("127.0.0.16", 7777)

spmu_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
hk_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

spmu_sock.bind(spmu_loopback_endpoint)
hk_sock.bind(hk_loopback_endpoint)

spmu_sock.listen()
hk_sock.listen()

print("waiting for connections...")
spmu_sock.accept()
print(spmu_loopback_endpoint, "connected")
hk_sock.accept()
print(hk_loopback_endpoint, "connected")

while True:
    pass