# foxsi-4matter
Code for the FOXSI-4 formatter.

This formatter replaces older FOXSI formatters with a [Raspberry Pi](https://www.raspberrypi.org) processor unit. A peripheral SPMU-001 board (based on the Xilinx Spartan 7) for [SpaceWire](https://www.star-dundee.com/wp-content/star_uploads/general/SpaceWire-Users-Guide.pdf) communication support.

## Setting up
See [PISETUP.md](PISETUP.md) for instructions on booting and configuring the Raspberry Pi.

### Dependencies
- [Boost](https://www.boost.org/)
    - boost::asio
    - boost::bind
    - boost::statechart
    - boost::interprocess
- [pigpio](https://abyz.me.uk/rpi/pigpio/download.html)
- [nlohmann JSON](https://github.com/nlohmann/json)

### Examples
You can find a few examples in the [examples folder](examples).