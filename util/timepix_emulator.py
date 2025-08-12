
from time import sleep
from datetime import datetime
import socket 
import time
import serial
import binascii 
import sys
import string 
import numpy as np
import os 
from pathlib import Path
# import RPi.GPIO as GPIO 
from shutil import disk_usage

# GPIO.setwarnings(False)

# Set up Classes 									# for readrates packet 
class ReadRatesPacket:
    def __init__(self, mean_tot: int = 0, flx_rate: int = 0):
        self.mean_tot = mean_tot  					# 2-byte integer (0-1022)
        self.flx_rate = flx_rate  					# 2-byte integer (four digits)


class ReadALLHKPacket:
    def __init__(self, board_t1: int = 0, board_t2: int = 0, asic_voltages: list = [0, 0, 0, 0],
                 asic_currents: list = [0, 0, 0, 0], fpga_values: list = [0, 0, 0], rpi_storage_fill: int = 0):
        self.board_t1 = board_t1  # 3-digit integer
        self.board_t2 = board_t2  # 3-digit integer
        self.asic_voltages = asic_voltages  # List of 4 integers (each with a max of 5 digits)
        self.asic_currents = asic_currents  # List of 4 integers (each with a max of 4 digits)
        self.fpga_values = fpga_values  # List of 3 integers (each with a max of 4 digits)
        self.rpi_storage_fill = rpi_storage_fill  # 3-digit integer

class ReadTempPacket:								# for read temp packets 
    def __init__(self, fpgat: int = 0, board_t1: int = 0, board_t2: int = 0):
        self.fpgat = fpgat  						# 3 digits
        self.board_t1 = board_t1  					# maximum 3 digits
        self.board_t2 = board_t2 

class FlagByte:
    def __init__(self):
        self.flags = 0         								# Initialize with all flags set to 0
    def raise_flag(self, flag_index):
        self.flags |= (1 << flag_index)         			# Set the bit at the specified index to 1
    def clear_flag(self, flag_index):         				# Set the bit at the specified index to 0
        self.flags &= ~(1 << flag_index)
    def is_flag_set(self, flag_index):				        # Check if the bit at the specified index is 1
        return bool(self.flags & (1 << flag_index))
    def get_flags(self):					        		# Return the entire byte representing flags
        return self.flags
                                                    #functions for packet building 
def create_read_all_hk_packet(data: ReadALLHKPacket):
    packet = [0] * 27  # Define a 27-byte packet
    packet[0] = data.board_t1 % 256
    packet[1] = data.board_t1 // 256
    packet[2] = data.board_t2 % 256
    packet[3] = data.board_t2 // 256

    for i in range(4):  # Packing ASIC voltages and currents (combined)
        voltage = data.asic_voltages[i] % 65536  # Combine voltage and current into 2 bytes
        current = data.asic_currents[i] % 10000
        packet[4 + i * 4] = voltage % 256
        packet[5 + i * 4] = voltage // 256
        packet[6 + i * 4] = current % 256
        packet[7 + i * 4] = current // 256

    for i in range(3):  # Packing FPGA voltages (2 bytes each)
        packet[20 + i * 2] = data.fpga_values[i] % 256
        packet[21 + i * 2] = data.fpga_values[i] // 256

    packet[26] = data.rpi_storage_fill  # Packing RPI storage fill (1 byte)

    return packet

def create_read_rates_packet(data: ReadRatesPacket):
    packet = [0] * 4  								# Define a 4-byte packet
    packet[0] = data.mean_tot & 0xFF
    packet[1] = (data.mean_tot >> 8) & 0xFF
    packet[2] = data.flx_rate & 0xFF
    packet[3] = (data.flx_rate >> 8) & 0xFF
    return packet


def create_read_temp_packet(data: ReadTempPacket):
    packet = [0] * 9  # Define a 9-byte packet
    packet[0] = data.fpgat % 256 			    	# Packing FPGA T (3 digits)
    packet[1] = data.fpgat // 256
    packet[2] = data.board_t1 % 256 				# Packing Board T1 (3 digits)
    packet[3] = data.board_t1 // 256
    packet[4] = data.board_t2 % 256			    	# Packing Board T2 (3 digits)	
    packet[5] = data.board_t2 // 256
    return packet

NUM_PCAPS = 4

def pack_pcap_size(size_mb):
    """Pack a single PCAP size (uint16) into 2 bytes."""
    size_int = int(size_mb)  # convert numpy.uint16 to int
    if not (0 <= size_int <= 0xFFFF):
        raise ValueError("PCAP size out of range for uint16")
    return size_int.to_bytes(2, byteorder="big")

def pack_pcap_status_packet(sizes_mb):
    """Pack list/array of 4 PCAP sizes into an 8-byte packet."""
    if len(sizes_mb) != NUM_PCAPS:
        raise ValueError(f"Need exactly {NUM_PCAPS} PCAP sizes")
    packet = bytearray()
    for size in sizes_mb:
        packet.extend(pack_pcap_size(size))
    return bytes(packet)

NUM_PHOTONS = 360

def pack_photon(x, y, tot, spare):
    packed = ((int(x) & 0x1FF) << 23) | ((int(y) & 0x1FF) << 14) | ((int(tot) & 0x3FF) << 4) | (int(spare) & 0xF)
    return packed.to_bytes(4, byteorder="big")

def pack_image_packet(xs, ys, tots, spares):
    if len(xs) != NUM_PHOTONS:
        raise ValueError("Need exactly 360 photons")
    packet = bytearray()
    for x, y, tot, spare in zip(xs, ys, tots, spares):
        packet.extend(pack_photon(x, y, tot, spare))
    return bytes(packet)

##crease instance of flag_byte for flags 
flag_byte = FlagByte()

## Set up UART Service 
usbdevice = '/dev/tty.usbserial-FT9O910G'
ser = serial.Serial(
    usbdevice,
    115200,
    serial.EIGHTBITS,
    serial.PARITY_NONE,
    serial.STOPBITS_ONE
)

## Set up GPIO on raspberry pi 
# GPIO.setmode(GPIO.BCM)
# GPIO.setup(17, GPIO.OUT) 
# GPIO.output(17,0)
#files 

tot_flipflop = 525
flux_flipflop = 2.0

now = datetime.now()

hkfn = "util/mock/tpx_hk.npz"
telefn = "util/mock/tpx_telemetry.npz"
logfile = "log/timepix_log_{}-{}-{}_{}-{}-{}".format(now.year, now.month, now.day, now.hour, now.minute, now.second)
pcfile = "util/mock/tpx_pc.npz"

pc_raw_data = np.load(pcfile)
x_npz, y_npz, tot_npz, spare_npz = pc_raw_data['x'], pc_raw_data['y'], pc_raw_data['tot'], pc_raw_data['spare']

print("starting listen...")

while True:																        ### Step One: Check FORMATTER for commands via UART
    received_bytes = ser.read()
    # print(f"Received command: ", received_bytes.hex())
    # received_bytes = b'\x80'
    # time.sleep(0.25)

    #Check if Files Too Large (slows down code)
    #
    #
    # now = datetime.now()
    now = time.time()

    ser.reset_input_buffer()
    #print("received:{}".format(received_bytes))
    if received_bytes == b'\x80':   						# PING COMMAND
        print("Received command: 0x80 (PING)")
        pingback = b'0x00'
        try:
            ser.write(pingback)
        except:
            flag_byte.raise_flag(2) #software error

    elif received_bytes == b'\x88':   						# READ ALL HOUSEKEEPING COMMAND
        print("Received command: 0x88 (Read Housekeeping)")
        hk_readback_start = time.time()
        try:
            hk = np.load(hkfn)
            temp1 = hk['temp1']
            temp2 = hk['temp2']
            ftemp = hk['ftemp']
            V33 = hk['V33']
            V25 = hk['V25']
            # V18 = hk['V18']
            V0A = hk['V0A']
            V0D = hk['V0D']
            V1A = hk['V1A']
            V1D = hk['V1D']
            V2A = hk['V2A']
            V2D = hk['V2D']
            V3A = hk['V3A']
            V3D = hk['V3D']
            I0A = hk['I0A']
            I0D = hk['I0D']
            I1A = hk['I1A']
            I1D = hk['I1D']
            I2A = hk['I2A']
            I2D = hk['I2D']
            I3A = hk['I3A']
            I3D = hk['I3D']
        except:
            flag_byte.raise_flag(2) #software error
            temp1 = 0
            temp2 = 0
            ftemp = 0
            V33 = 0
            V25 = 0
            # V18 = 0
            V0A = 0
            V0D = 0
            V1A = 0
            V1D = 0
            V2A = 0
            V2D = 0
            V3A = 0
            V3D = 0
            I0A = 0
            I0D = 0
            I1A = 0
            I1D = 0
            I2A = 0
            I2D = 0
            I3A = 0
            I3D = 0
        try:
            rpi = disk_usage("/")
            #below returns percentage left, max 100 
            per_left = round(min((rpi.free / rpi.total) * 100,100))
            if per_left < 50: #if less that 1/2 storage space left 
                flag_byte.raise_flag(3) #flag for storage warn
        except:
            per_left = 111
            flag_byte.raise_flag(2) #flag for software error 

        read_all_hk_data = ReadALLHKPacket(
        board_t1=int(temp1) ,
        board_t2=int(temp2),
        asic_voltages=[int(V0A), int(V1A), int(V2A), int(V3A)], #analog 
        asic_currents=[int(I0A), int(I1A), int(I2A), int(I3A)], #analog 
        fpga_values=[int(V33), int(V25), int(ftemp)], #fpga v33, fpga v18, fpga temperature 
        rpi_storage_fill=per_left)

        if temp1 or temp2 > 50: 
                flag_byte.raise_flag(4)
        elif ftemp > 50:
            flag_byte.raise_flag(5)
        
        read_all_hk_packet = create_read_all_hk_packet(read_all_hk_data)
        
        try: 
            ser.write(bytes(read_all_hk_packet))
            delta = time.time() - now
            print("\tHK response in ", delta, "s")
        except:
            flag_byte.raise_flag(2) #software error


    elif  received_bytes == b'\x89':   						# READ TEMPERATURES COMMAND
#		print("Received command: 0x89 (Read Temperatures")
        #pull from/populate recent hk data 
        try:
            hk = np.load(hkfn)
            temp1 = hk['temp1']
            temp2 = hk['temp2']
            ftemp = hk['ftemp']
        except:
            flag_byte.raise_flag(2) #software error
            temp1 = 0
            temp2 = 0
            ftemp = 0

        if temp1 or temp2 > 50:
            flag_byte.raise_flag(4)
        elif ftemp > 50:
            flag_byte.raise_flag(5)
        
        read_temp_data = ReadTempPacket(fpgat=int(ftemp), board_t1=int(temp1), board_t2=int(temp2))
        read_temp_packet = create_read_temp_packet(read_temp_data)
        
        try:
            ser.write(bytes(read_temp_packet))
        except:
            flag_byte.raise_flag(2) #software error


    elif received_bytes == b'\x8A':  						# READ SOFTWARE STATUS COMMAND
        print("Received command: 0x8A (Read Software Status)")

        boold = flag_byte.is_flag_set(3) #software flag is 3 
        
        if boold == True:
            try:
                ser.write(b'\x01') #flagged
            except:
                flag_byte.raise_flag(2) #software error
        else:
            try:
                ser.write(b'\x00') #not flagged - status healthy
            except:
                flag_byte.raise_flag(2) #software error
        

    elif received_bytes == b'\x81': #READ RATES COMMAND
        print("Received command: 0x81 (Read Rates)")
        #pull from/populate recent pixel data 
        try:
            tele = np.load(telefn)
            meanToT = tele['meanToT']
            flxrate = tele['flxrate']
            if tot_flipflop == 525:
                tot_flipflop = 560
            else:
                tot_flipflop = 525
            if flux_flipflop == 2.0:
                flux_flipflop = 3.0
            else:
                flux_flipflop = 2.0
        except:
            flag_byte.raise_flag(2)
            # meanToT = 0
            # flxrate = 0
            tot_flipflop = 0
            flux_flipflop = 0


        # read_rates_data = ReadRatesPacket(mean_tot = int(meanToT),flx_rate=int(flxrate))
        read_rates_data = ReadRatesPacket(mean_tot = int(tot_flipflop),flx_rate=int(flux_flipflop))
        read_rates_packet = create_read_rates_packet(read_rates_data)
        try:
            ser.write(bytes(read_rates_packet))
        except:
            flag_byte.raise_flag(2) #software error

    elif received_bytes == b'\xA0': # READ PHOTON EVENT DATA
        print("Received command: 0xA0 (READ PHOTON EVENTS)")
        packet = pack_image_packet(x_npz, y_npz, tot_npz, spare_npz)
        ser.write(bytes(packet))

    elif received_bytes == b'\xA5':
        print("Received command: 0xA5 (READ PCAP HEALTH)")
        fake_pcap_sizes = [0, 1, 20000, 50001]
        packet = pack_pcap_status_packet(fake_pcap_sizes)
        ser.write(bytes(packet))

    elif received_bytes == b'\x8B':  #READ ERROR FLAG COMMAND
        print("Received command: 0x8B (READ ERROR FLAG)")
        #check flag status 
        ##  Pull from  text file
        all_flags = flag_byte.get_flags()
        try:
            ser.write(bytes([all_flags])) #changed from: bytes(all_flags)
        except:
            flag_byte.raise_flag(2)

    elif received_bytes == b'\xA4':  #READ CLOCK COUNTER COMMAND
        print("Received command: 0xA4 (READ CLOCK COUNTER)")
        # QQQQQ # 
        # Untested, Currently unused 


    elif received_bytes == b'\x62':  #HV RAMP DOWN START COMMAND
        print("Received command: 0x62 (HV RAMP DOWN START)")
        try:
            with open(logfile,'w') as f: 
                f.write("\n HV RAMP DOWN START ~ TimeStamp: {} \n".format(now))
                f.flush()
                f.close()
        except:
            flag_byte.raise_flag(2)


    elif received_bytes == b'\x63':  #HV RAMP UP START COMMAND
        print("Received command: 0x63 (HV RAMP UP START)")
        # GPIO.output(17,1)
        #Turn on HV on flag (1)
        flag_byte.raise_flag(1)
        try:
            with open(logfile,'w') as f: 
                f.write("\n HV RAMP UP START ~ TimeStamp: {} \n".format(now))
                f.flush()
                f.close()
        except: 
            flag_byte.raise_flag(2)


    elif received_bytes == b'\x70':  #NotifY PRE LAUNCH COMMAND
        print("Received command: 0x70 (Notify Pre Launch)")
        
        try:
            with open(logfile,'w') as f: 
                f.write("\n Pre Launch Command ~ TimeStamp: {} \n".format(now))
                f.flush()
                f.close()
        except: 
            flag_byte.raise_flag(2)


    elif received_bytes == b'\x71':  #NOTelifY LAUNCH HOLD COMMAND
        print("Received command: 0x71 (Notify Launch Hold)")
        try:
            with open(logfile,'w') as f: 
                f.write("\n Launch Hold ~ TimeStamp: {} \n".format(now))
                f.flush()
                f.close()
        except:
            flag_byte.raise_flag(2)

    elif received_bytes == b'\x22': #Erase Storage 
        print("Received command 0x22 (Erase Storage)") 
        try:
            with open(logfile,'w') as f: 
                f.write("\n Deleted Storage ~ TimeStamp: {} \n".format(now))
                f.flush()
                f.close()
        except:
            flag_byte.raise_flag(2)

    elif received_bytes == b'\x79': #reboot pi 
        print("Received command 0x79 (reboot)")

    else:
        print("Received unknown command:", received_bytes)
        try: 
            with open(logfile,'w') as f: 
                f.write("\n Wrong/Corrupt Command Received: {}~ TimeStamp: {} \n".format(received_bytes,now))
                f.flush()
                f.close()
        except:
            flag_byte.raise_flag(2)
