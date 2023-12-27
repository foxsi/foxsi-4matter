import serial

s = serial.Serial("/dev/ttyAMA1",  9600)
s.write("hi there")
reply = s.read()
print(reply)