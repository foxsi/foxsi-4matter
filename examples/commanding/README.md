# Commanding

This example extends [CLI](../CLI/README.md) to parse command lists stored in JSON files. These JSON command files will be used to lookup command parameters for each commanded onboard system.

## Using

This example expects an [INI file](example_config.ini) to be passed as a command line argument like this (assuming you're in the main repo directory):

```bash
$ ./bin/commanding_example --file examples/commanding/example_config.ini
```

That INI file ([example_config](example_config.ini)) should include a header section `[systems]` with field `codepath=<path-to-system-list.json>`. The `codepath` file should be a JSON file listing all onboard systems, and their hex IDs ([like this](../../foxsi4-commands/all_systems.json)). 

Commands can be included for the following subsystems:
```INI
[cdte1]
[cdte2]
[cdte3]
[cdte4]
[cdtede]
[cmos1]
[cmos2]
[timepix]
[housekeeeping]
```

CdTe and CMOS systems will expect SpaceWire command data to be provided, Timepix will expect UART, and Housekeeping will expect SPI command data. You can find a [template command file here](../../foxsi4-commands/commands.json).
