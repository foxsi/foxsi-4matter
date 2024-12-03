# Setting up the Raspberry Pi boot image
This guide is for setting up a _brand new_ Raspberry Pi _from scratch_ for use as the FOXSI-4 Formatter. If you are just trying to modify/build/fix/run the Formatter software, [go back here](https://github.com/foxsi/foxsi-4matter/blob/main/README.md).

## Supplies
1. Rapsberry Pi SBC (versions 4 or 5 will work, as will a Compute Module with carrier board)
2. microSD card with capacity ≥ 8GB
3. Laptop
4. Ethernet cable
5. Power cable (USB C or jumpers)

## Booting
1. On your laptop, download and install the [Raspberry Pi Imager](https://www.raspberrypi.com/software/)
2. Insert the microSD card
3. Open the Raspberry Pi Imager software
	1. Select the boot image called "Raspberry Pi OS Lite", the 32-bit Raspbian headless image.
	2. Click the gear in the bottom right for advanced options.
	3. Do not prefill the Wifi password from keychain.
	4. Enable SSH, using password authentication.
	5. Choose your SSH username and password. **The current username is `foxsi` and the current password is `four`**.
	6. Choose your location (helps with faster Wifi setup but not critical).
	7. Close the advanced settings and choose the write location to be the SD card
	8. Press `write`. Any old data on the SD will be erased, and the card will be ejected after writing.
4. Remove the microSD from the laptop and plug it into the Raspberry Pi.
5. Connect the Raspberry Pi to power. You should see two lights: red for power on, and a blinking green light after boot completes.
6. Connect the laptop to the Raspberry Pi with an Ethernet cable.

# Talking to the Raspberry Pi (SSH)
**From here I assume macOS Terminal, and knowledge of filesystem navigation (`ls`, `cd`) and file editing ([vim](https://devhints.io/vim) or [nano](https://www.nano-editor.org/dist/latest/cheatsheet.html))**. You can translate the shell commands if needed. I use the bash prompt `%` to indicate the macOS Terminal command line, and the `$` bash prompt to indicate the Linux command line via the SSH session.
1. Open a Terminal window.
2. SSH into the Raspberry Pi with the following command, replacing the `<username>` with the username you chose when making the boot image:
```bash
	% ssh <username>@raspberrypi.local
```
3. Enter the password you chose when building the boot image.
4. *End the SSH session by typing* `exit`.

# Setting up the Raspberry Pi
1. You should be in an SSH session in the Raspberry Pi.
2. Run
```bash
	$ sudo raspi-config
```
3. Enter the `Localisation Options` menu, and select your location and language.
4. Enter the `Interface Options` menu:
   1. Select `SSH` and enable `SSH` login.
   2. Select `Serial Port` and enable serial interface, but disable login shell from serial port.

## Setting up UARTs
We need to enable UARTs to communicate with Timepix and the command uplink system. Inside the Pi, edit the `/boot/config.txt`:
```bash
$ sudo nano /boot/config.txt
```
At the very bottom, add two lines: 
```
dtoverlay=uart2
dtoverlay=uart5
```

## Setting up `eduroam` WiFi
This is necessary to download packages at, e.g. the University of Minnesota, where network sharing from the host laptop to the Raspberry Pi is disallowed. If you are on a different network things will probably be easier.
1. Create a file on the Pi in the folder `/etc/wpa_supplicant` called `wpa_supplicant.conf`. You can create the file and start editing with
```bash
$ nano /etc/wpa_supplicant/wpa_supplicant.conf
```
2. Edit the file and add the following information, replacing `<your-x500>` with your UMN internet ID, and `<your-UMN-password>` with (unsurprisingly) your UMN password:
```
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
country=US

network={
        ssid="eduroam"
        key_mgmt=WPA-EAP
        auth_alg=OPEN
        eap=PEAP
        identity="<your-x500>@umn.edu"
        password="<your-UMN-password>"
        phase2="auth=MSCHAPV2"
}
```
3. Note that the file lists your login and password information verbatim. This is very insecure.
4. You can check the network connection by `ping foxsi.umn.edu` at the terminal.

Now reboot the Pi to implement your changes:
```bash
$ sudo reboot
```

### Disabling WiFi
For flight, you may not want a WiFi interface floating around. You can disable WiFi by adding `dtoverlay=disable-wifi` to your device tree:
```bash
$ sudo nano /boot/config.txt
```
and add the lines
```bash
dtoverlay=disable-wifi
```
Then you can reboot to implement the changes.

## Installing required packages
Assuming you have an internet connection, you should be able to use `apt-get` to install some required libraries for the Formatter. Some of these may take a few minutes to install.
```bash
$ sudo apt-get install cmake
$ sudo apt-get install libboost1.74-*
$ sudo apt-get install nlohmann-json3-dev
$ sudo apt-get install googletest
```

The recent v1.83.* versions of `boost` are not yet (as of March 2024) available in the Debian distribution, so I specify an earlier version for the Raspberry Pi install.

Once complete, you may need to modify the [CMakeLists.txt](CMakeLists.txt) to point `NLOHMANNJSON_ROOT` to the correct directory. The path you input should contain the file `json.hpp`.
