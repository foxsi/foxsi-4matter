import serial

s = serial.Serial("/dev/ttyAMA1",  9600)
s.write(r"\x88")
reply = s.read()
print(reply)