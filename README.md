[![build](https://github.com/foxsi/foxsi-4matter/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/foxsi/foxsi-4matter/actions/workflows/c-cpp.yml)
# foxsi-4matter
Code for the FOXSI-4 formatter.

This formatter replaces older FOXSI formatters with a [Raspberry Pi](https://www.raspberrypi.org) processor unit. There's also an SPMU-001 board (based on the Xilinx Spartan 7) for [SpaceWire](https://www.star-dundee.com/wp-content/star_uploads/general/SpaceWire-Users-Guide.pdf) communication support, and a [Housekeeping board](https://github.com/foxsi/foxsi4-hk) for commanding the onboard power supply.

## Setting up
If you are setting up a fresh Raspbeery Pi, see [PISETUP.md](PISETUP.md) for instructions. If you are just trying to get this software running, stay where you are (this README).

## Network configuration
The physical system is laid out like this:

![The Formatter is connected to many systems. Sorry, an image would be handy.](doc/assets/formatter_layout.svg "Formatter physical interfaces")

There are a lot of things that plug into the Formatter. The role of this software is to pipe data between all these interfaces. That involves 
1. configuring the software to have permission to access all these ports on startup, 
2. then handling all the idiosyncrasies of packet formatting for all the connected systems, so that the actual data can pass between interfaces without extraneous headers/wrappers coming along for the ride.

This software owes its life to the [`foxsi4-commands`](https://github.com/foxsi/foxsi4-commands/tree/main) repository, which serves tells the code about the diagram above. Specifically, the top-level file [`foxsi4-commands/systems.json`](https://github.com/foxsi/foxsi4-commands/blob/main/systems.json) defines which interfaces are available to the Formatter and how they are constrained—what IP addresses and port numbers and maximum packet sizes and data rates and timeouts should be used for some given interface. For downlink and uplink commands, `systems.json` is the "contract" between the GSE and Formatter that defines their common interface. 

> [!NOTE]
> As of [v1.2.1](https://github.com/foxsi/foxsi-4matter/releases/tag/v1.2.1), there is a nuisance in [foxsi4-commands/systems.json](https://github.com/foxsi/foxsi4-commands/blob/main/systems.json) for Formater use. The Formatter will run and transmit downlink data to the Ethernet endpoint defined by [`gse.ethernet_interface.address`](https://github.com/foxsi/foxsi4-commands/blob/77d61f94183432e3a4fdff0bf0356e8361575445/systems.json#L5-L11). But the GSE will listen to [`gse.ethernet_interface.mcast_group`](https://github.com/foxsi/foxsi4-commands/blob/77d61f94183432e3a4fdff0bf0356e8361575445/systems.json#L5-L11). 
>
> So you'll need to replace the **Formatter's** GSE configuration in `systems.json` with this:
> ```json
>   "ethernet_interface": {
>        "protocol": "udp",
>        "address": "224.1.1.118",
>        "port": 9999,
>        "max_payload_bytes": 2000
>    }, 
> ```

## How to build
Here's how you can build this software from scratch to run on the Raspberry Pi. 

In the following, "your computer" is a laptop or desktop connected to the Raspberry Pi, which actually runs the software.

Decide whether you need to build the software on your computer *in addition* to building on the Raspberry Pi. If you make local edits and want to check that everything compiles, or if you want to run certain tests, or if you want to use the `foxsimile` emulator, you should build on your machine. If all you need to do is forward code to the Raspberry Pi and build it there, skip to [building on the Raspberry Pi to run](#building-on-the-raspberry-pi-to-run).

### Installing dependencies
If this is your first time building on your computer, you may need to install two dependencies. If you are setting up a new Raspberry Pi, see [PISETUP.md](PISETUP.md) for instructions on installing dependencies.

#### macOS
```bash
brew install cmake
brew install nlohmann-json
brew install boost
brew install doxygen
brew install googletest
```
(if you want to check if they are already installed, you can run `brew info <package name>` to check).

I have used `boost` v1.83.0 and v1.80.0, and `nlohmann-hson` v3.11.3. I expect other `boost` versions 1.7x and up, and other 3.1x versions of `nlohmann-json` to also work, but have not tested.

### Building on the Raspberry Pi to run
The flight Raspberry Pi already has dependencies installed. If you are setting up a brand new Raspberry Pi, see [PISETUP.md](PISETUP.md). Otherwise, choose instructions for either
1. [setting up from](#set-up-from-a-release) a [tagged release](https://github.com/foxsi/foxsi-4matter/tags), or
2. [setting up from the latest version of the `main` branch available](#set-up-from-latest-main).

#### Set up from a release
Select the release you want to build on the [`foxsi-4matter` tags page](https://github.com/foxsi/foxsi-4matter/tags). On the page for a specific release ([v0.0.8, as an example](https://github.com/foxsi/foxsi-4matter/releases/tag/v0.0.8)) download the .zip or tar.gz archive to your computer. 

Unzip the archive you downloaded, then open a terminal *inside* that folder (which should be named `foxsi-4matter-x.y.z`, where `x.y.z` is the version number you downloaded). From that terminal, run:
```bash
git clone https://github.com/foxsi/foxsi4-commands.git
```
to add the command repository to your release. If you want to try building software on your computer first, run this sequence of commands inside the folder `foxsi-4matter-x.y.z`:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```
You can omit this local build step if you just want to build on the Raspberry Pi.

#### Set up from latest `main`
I try to keep the latest `main` on GitHub versioned anyway. But this way you can use `git clone` instead of downloading zips.

Open a terminal in a folder where you want this code, and run
```bash
git clone --recursive https://github.com/foxsi/foxsi-4matter.git
```

If you want to try building on your computer first, run this sequence of commands inside the folder `foxsi-4matter` that was just created:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

#### Building on Raspberry Pi
Now that you have code stored on your computer, you can push the code to the Raspberry Pi over an Ethernet connection. First, `ssh` into the Raspberry Pi:
```
ssh foxsi@192.168.1.8
password:...
```
The password for the flight Pi is `four`.

If you have issues `ssh`ing, make sure you can `ping` the IP `192.168.1.8` and that your local subnetwork includes that address.

Now you can prepare to build new code on the Formatter. Before building any new code, make sure to stop and power off any detector systems onboard. Then stop running the Formatter service:
```bash
sudo systemctl stop formatter.service
```

The Formatter service will resume running after a reboot.

> [!NOTE] This will overwrite the existing `formatter` binary in the Formatter. 
> If you want to save a copy of it first, do the following to save a copy of the binary in whatever directory you are currently in on your machine:
> ```bash
>  scp foxsi@192.168.1.8:foxsi-4matter/bin/formatter .
>  ```


From another terminal on your computer, navigate to the downloaded `foxsi-4matter` code, which should be in a folder called either `foxsi-4matter` (for a build from `main`) or `foxsi-4matter-x.y.z` (for a build from a release). Then run this command (it's long) to push the code into `your-folder-name` in the Raspberry Pi:
```bash
rsync -av --exclude=build --exclude=bin --exclude=doc --exclude=log --exclude=foxsi4-commands ../foxsi-4matter-x.y.z foxsi@192.168.1.8:/home/foxsi
```
but replace `-x.y.z` with the version number, or omit it entirely for a build from `main`.

Now, go back to your other terminal that is still `ssh`'d inside the Raspberry Pi, and create a new build folder:
```bash
cd foxsi-4matter
rm -r build/*
cd build
cmake ..
cmake --build . -j4
```
The Raspberry Pi build process takes a couple minutes. This will create several binaries in `foxsi-4matter/bin/` that you can run. The main program that runs in flight is `foxsi-4matter/bin/formatter`. 

You should have stopped executing `formatter` via `systemctl` above. The new binary you just built will be run after a reboot. Or, if you want to see all the `cout` output as it happens, you can run it now like this:
```bash
./bin/formatter --verbose --config foxsi4-commands/systems.json
```

## How to use
Once the Formatter software starts running (either on boot, or because you have manually started running something) it will try to poll all the detector systems for housekeeping data. These systems may or may not be connected or powered on.

You can use the GSE to command onboard systems on/off. Then start software on each detector readout systems. Refer to the FOXSI-4 

For detail on the CdTe DE software, go to the [DE github repository](https://github.com/foxsi/CdTe_DE).

### Detector mapping
The DE identifies connected canisters (in the raw data recordings) on the SpaceWire port they are connected to. The power system requires a unique byte to be sent to turn on/off each system. For the FOXSI-4 flight configuration, this is the DE nomenclature for each detector and power code:

| System (by focal plane position) | Detector  | DE number | Power board byte |
| -------------------------------- | --------- | --------- | ---------------- |
| Canister 2                       | no2021_06 | `det4`    | `0x06`           |
| Canister 3                       | no2021_07 | `det2`    | `0x04`           |
| Canister 4                       | no2021_05 | `det3`    | `0x05`           |
| Canister 5                       | no2022_01 | `det1`    | `0x03`           |
| DE                               | ---       | ---       | `0x00`           |
| CMOS 1                           | `0010`    | ---       | `0x08`           |
| CMOS 2                           | `0002`    | ---       | `0x07`           |
| Timepix                          | ---       | ---       | `0x01`           |
| SAAS                             | ---       | ---       | `0x02`           |

### Starting the Formatter software
The Formatter will try to run the binary `/home/foxsi/foxsi4-matter/bin/formatter` after boot, and again every 10 seconds after that binary stops running. This is managed by [[`systemctl`](https://www.freedesktop.org/software/systemd/man/latest/systemctl.html)], which is a powerful Linux utility for running *services*. You can interact with the Formatter service using these commands:

```bash
sudo systemctl stop formatter.service          # stop running the Formatter service
sudo systemctl start formatter.service         # start running the Formatter service
sudo systemctl disable formatter.service       # disable the Formatter service from starting after boot
sudo systemctl enable formatter.service        # enable the Formatter service to run after boot
sudo systemctl status formatter.service        # report the current status of the Formatter service
```

The last command, querying the `status` of the running Formatter service, will tell you if it is running still or has stopped. It will also print some of the Formatter's recent output. Note that while the Formatter service may still be running, the main loop may have effectively stopped due to subsystem disconnects. If you query the status multiple times and always see identical printout, the service may no longer be running correctly. 

When the service is running, if you need to debug you will need to find the correct `~/foxsi-4matter/log/` file to collect evidence. This can be inconvenient to track down since Unixtime changes with every reboot. 

A helpful debugging workflow is to stop the Formatter service, then launch it manually so you can see the stdout output:
```bash
sudo systemctl stop formatter.service
./bin/formatter --verbose --config foxsi4-commands/systems.json
```
You can stop the Formatter with ctrl-C.

## Downlink data

The Formatter transmits data over the UDP interface defined in `foxsi4-commands/systems.json`'s `gse` field. A complete raw data frame from an onboard system may be larger than the maximum downlink packet size, in which case it will be fragmented. A given downlink packet has the following header:

| Index | Size    | Name       | Description                                               |
| ----- | ------- | ---------- | --------------------------------------------------------- |
| `0`   | 1 byte  | `system`   | Onboard system ID which produced data in packet.          |
| `1-2` | 2 bytes | `n`        | Number of packets in this frame.                          |
| `3-4` | 2 bytes | `i`        | Index of this packet in the frame. **This is 1-indexed.** |
| `5`   | 1 bytes | `data`     | An identifier of data type stored in the packet.          |
| `6-7` | 2 bytes | `frame_counter` | Identifier used to associate packets with the complete frame they originally came from. Planned addition in v1.3.0.                                                 |

The raw data payload is concatenated after the 8-byte header.

For the `system` field, the ID value is taken from `foxsi4-commands/systems.json` in the `hex` field for each system entry. For the `data` field, the following map is used:

| Value  | Name   | Description          |
| :----: | ------ | -------------------- |
| `0x00` | `pc`   | Photon-counting data |
| `0x01` | `ql`   | Quick-look data      |
| `0x02` | `tpx`  | Timepix data         |
| `0x10` | `hk`   | Housekeeping data    |
| `0x11` | `pow`  | Power data           |
| `0x12` | `temp` | Temperature data     |
| `0x13` | `stat` | System status data   |
| `0x14` | `err`  | System error data    |
| `0x20` | `ping`  | Software status (added in v1.3.0) |
| `0x30` | `reply`  | Forwarded reply to command |
| `0xff` | `none` | No data type         |

### Logging frame data with simple logger

This package includes a simple script to log downlinked frame data to `log/gse/`. The log file name has this format: `<source system name>_<data_type>.log`. The logger is at `util/logudp.py`, and can be run like this:

```bash
cd ~/Documents/FOXSI/foxsi-4matter/
python3 util/logudpy.py
```

This will save raw data to `~/foxsi-4matter/log/gse/` under a filename describing the source system for the data (for example, `cmos1_ql.log` is the raw quicklook data output of `cmos1`). **Note that these log files are overwritten on each successive run.** After collecting data you want ot save, copy the file out to a different folder. You can stop logging data with ctrl-C. For the CdTe system, the raw data files can be parsed using the GSE parser.

### Logging all downlinked data with GSE

You can also log downlinked data using the [GSE software](https://github.com/foxsi/GSE-FOXSI-4). The GSE package includes a utility in the `Listener` class to reconstruct frames and log packets to file. This object is designed to be used in a background process from the main GSE, but can also be run directly from the command line (from within the GSE software directory):

```bash
python3 FoGSE/listening.py foxsi4-commands/systems.json
```

On startup, this `Listener` application creates a set of log file at this location in the GSE directory: `logs/received/<date_time>/`. Within this folder, a file is created for each system and downlink datatype from the above table, with this format: `<system>_<data>.log`. 

Additionally, an ASCII-formatted file called `catch.log` is produced to log any received packets that can't be parsed into a system-specific file. Each entry in this file is newline-separated, and tagged at the start of the file with an offset timestamp from the creation time of the file.

## Common issues
1. If you run the formatter software, kill it, then try to run it immediately again, you will get an error containing `connect: Address already in use`. This is because the kernel retains control of TCP sockets for a while (~1 minute) after you close them to allow any messages still on the wire to come through. Just wait a moment and try running again. Of course, it is possible (depending on your configuration) that another process on the same machine actually is using the IP address you want as well.
2. Unix time resets to a similar value on every reboot. Do not trust any timestamps you get out of this software.
3. If you never clear the `~/foxsi-4matter/log/` folder, you can completely fill the disk and get a read-only filesystem. Then you will need to edit `fstab` manually and with a racing heart  while the whole flare campaign waits for you. This happened at PFRR during the FOXSI-4 launch campaign. 

## Documentation
If you enjoyed this measly little README, you're going to *love* the rest of the documentation! You can view these docs in a web browser (they are HTML), and until they are hosted somewhere, you will need these instructions to build them:

```bash
git clone https://github.com/foxsi/foxsi-4matter.git    # clone this repository
cd foxsi-4matter/doc            # navigate into the repository
python3 -m venv env/            # create a python virtual environment
source env/bin/activate         # activate the virtual environment
pip install sphinx              # install packages for the documentation... 
pip install breathe
pip install furo
cd ..
source util/build_doc.sh        # script for building all docs
```

Documentation should open automatically after running the last shell script, but if not, you should be able to enter at `foxsi-4matter/doc/breathe/build/index.html`.

I hope you enjoy.
