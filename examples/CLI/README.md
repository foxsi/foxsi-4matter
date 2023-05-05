# CLI

This example instantiates some `Subsystem` objects using config files passed in over a command line interface (**CLI**).

To use, can run the compiled example like this:

```bash
% ./bin/cli_example --file examples/CLI/example_config.ini --verbose
```

to use the provided example config, and run with verbose output. Note that the roles of the Formatter and GSE are flipped here for development ease (like in the [forwarding example](../forwarding/README.md)). 

## Using

Refer to the remote machine setup (SSH, open terminals, use `netcat`) in the [forwarding example](../forwarding/README.md).