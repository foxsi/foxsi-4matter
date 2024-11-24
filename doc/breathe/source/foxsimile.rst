``foxsimile`` test framework
============================

Overview
--------
We'd like to test some functionality of this software without needing all of the FOXSI flight hardware. Doing that's a little tricky, because the fundamental function of this software is piping data from place to place. It really does no work on its own. So to simulate the formatter's interactions with other systems, we need to simulate the systems it interacts with.

.. note::
    This mocking framework has been developed for macOS, and only tested on that platform. Everything that follows may need modification for other platforms.

The approach here framework is to use on-machine loopback IP connections and virtual serial ports to pipe data between the ``formatter`` process, and a ``foxsimile`` process that emulates detector system responses. The real system has different computers with outward-looking IP address that talk to each other; this emulation framework replaces those multiple computers with multiple processes on a single computer. So of course it is not fully representative of the real systems, but at least we can use it to test communication over real TCP/UDP/serial connections.

Scope
~~~~~
The ``foxsimile`` framework supports emulation of UART and Ethernet devices. 

Setup and Dependencies
~~~~~~~~~~~~~~~~~~~~~~
``socat`` is used to run vitual serial ports. You can install it like this:

.. code-block:: bash

    brew install socat

We will use multiple loopback IP address to emulate all systems. On macOS, these additional loopback addresses need to be enabled first. There is a shell sript in this repository's ``util/`` that will do this:

.. code-block:: bash

    source util/assign_all_loopbacks.sh

This will take a moment to run.

On macOS, you will also need to allow your loopback interface (``lo0``) to route to multicast IP addresses:

.. code-block:: bash
    
    sudo route -nv add -net 224.1.1.118 -interface lo0

On Linux there is a similar ``route`` command with slightly different flags. There are workarounds for this—you can do only point-to-point communication and hardcode a ``gse`` IP address in ``foxsimile_systems.json``—but this is closer to the flight network setup.

Running
-------
Each of the following commands is a separate process that will run throughout your test. Either detach them, or run each in its own terminal.

Open a virtual serial port with ``socat``:

.. code-block:: bash

    socat pty,raw,echo=0,link=/tmp/foxsi_serial pty,raw,echo=0,link=/tmp/foxsi_serial 

Start ``foxsimile``:

.. code-block:: bash

    ./bin/foxsimile --config foxsi4-commands/foxsimile_systems.json

If desired, you can start GSE software or a listening script for UDP packets the ``formatter`` will "downlink." A simple Python script that will just print all received packets from the ``formatter`` can be run like this:

.. code-block:: bash

    python3 util/printudp.py 127.0.0.118 9999

Now start the ``formatter`` using the ``foxsimile`` config:

.. code-block:: bash
    
    ./bin/formatter --verbose --config foxsi4-commands/foxsimile_systems.json


Detailed information
--------------------
See the documentation for the :ref:`foxsimile-namespace-doc` namespace for more information.