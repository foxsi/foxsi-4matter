# foxsi4-formatter
Code for the FOXSI-4 formatter.

This formatter replaces legacy FOXSI formatters with a [Raspberry Pi](https://www.raspberrypi.org) processor unit. A peripheral SPMU-001 board (based on the Xilinx Spartan 7) for [SpaceWire](https://www.star-dundee.com/wp-content/star_uploads/general/SpaceWire-Users-Guide.pdf) communication support.

See [PISETUP.md](PISETUP.md) for instructions on booting the Raspberry Pi.

# Dependencies
- [Boost](https://www.boost.org/)
    - boost::asio
    - boost::bind
    - boost::statechart
    - boost::interprocess
- [pigpio](https://abyz.me.uk/rpi/pigpio/download.html)

