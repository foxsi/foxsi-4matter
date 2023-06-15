import spidev
spi=spidev.SpiDev()
# see MAX7317 datasheet
# for MAX7317, CPOL=0, CPHA=1
# spi.max_speed_hz = 1/(38.4 * 1e-9)

# Phil's board:
#   MAX7317 port    Output
#   P0              CdTe-5V5
#   P1              Timepix-5V and Timepix-12V
#   P2              SAAS-5V and SAAS-12V
#   P3              CdTe can 2-28V
#   P4              CdTe can 3-28V
#   P5              CdTe can 4-28V
#   P6              CdTe can 5-28V
#   P7              CMOS1-28V
#   P8              CMOS2-28V
#   P9              unused
# on Phil's board, MAX7317 outputs are connected to /EN pins. 
# pull HIGH to disable channel, pull LOW to enable.
# Note: canisters are numbered by focal plane position.

# can use Rpi SPI0:
#   GPIO8:  P24    CS
#   GPIO9:  P21    MISO
#   GPIO10: P19    MOSI
#   GPIO11: P23    SCLK
# jumper out of ether switch board

# convenience function to print status:
def print_status(state):
    print("    CdTe-DE:\t" + ("on" if not (state & 0b0000000001) else "off")) 
    print("    Timepix:\t" + ("on" if not (state & 0b0000000010) else "off")) 
    print("    SAAS:\t" + ("on" if not (state & 0b0000000100) else "off")) 
    print("    CdTe can 2:\t" + ("on" if not (state & 0b0000001000) else "off")) 
    print("    CdTe can 3:\t" + ("on" if not (state & 0b0000010000) else "off")) 
    print("    CdTe can 4:\t" + ("on" if not (state & 0b0000100000) else "off")) 
    print("    CdTe can 5:\t" + ("on" if not (state & 0b0001000000) else "off")) 
    print("    CMOS 1:\t" + ("on" if not (state & 0b0010000000) else "off")) 
    print("    CMOS 2:\t" + ("on" if not (state & 0b0100000000) else "off")) 
    print("\n")

spi.open(0,0)
spi.max_speed_hz = 10000   # 10 kHz
spi.mode = 0b01
# spi.bits_per_word = 16

rd = 0b10000000
wr = 0b00000000

hi = 0x00
lo = 0x01

# user prompt string:
prompt = """User, enter:
    [0]: toggle CdTe-DE
    [1]: toggle Timepix
    [2]: toggle SAAS
    [3]: toggle CdTe canister 2
    [4]: toggle CdTe canister 3
    [5]: toggle CdTe canister 4
    [6]: toggle CdTe canister 5
    [7]: toggle CMOS 1
    [8]: toggle CMOS 2

    [a]: everything on
    [o]: everything off

    [q]: exit 
""" 

# set all outputs to zero:
# spi.xfer2([wr|0x0A, 0x01])
spi.xfer([0x0A, lo])
print("all outputs should be low.")

state = 0b1111111111
key = "o"

while key != "q":
    key = input(prompt)
    if key.isdigit():
        sys = int(key)
        state = state ^ (1 << sys)
        level = (state >> sys) & 1
        spi.xfer2([sys, level])

    else:
        if key == "a":
            state = 0b0000000000 
            spi.xfer2([0x0A, hi])
        elif key == "o":
            state = 0b1111111111
            spi.xfer2([0x0A, lo])

    #print("state: " + format(state, '010b')) 
    print_status(state)

spi.close()
