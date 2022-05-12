# Setting up the Raspberry Pi boot image
## Supplies
1. Rapsberry Pi SBC (versions 3 or 4 will work, as will a Compute Module with carrier board)
2. microSD card with capacity ≥ 8GB
3. Laptop
4. Ethernet cable
5. Power cable (USB C for Raspberry Pi 4, USB micro B for Raspberry Pi 3)

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

## Talking to the Raspberry Pi
**From here I assume macOS**. You can translate the shell commands if needed.
1. Open a Terminal window.
2. SSH into the Raspberry Pi with the following command, replacing the `<username>` with the username you chose when making the boot image:
```
	% ssh <username>@raspberrypi.local
```
3. Enter the password you chose when building the boot image.
4. *End the SSH session by typing* `exit`.
