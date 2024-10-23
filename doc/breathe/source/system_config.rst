Configuration data
==================

Overview
--------
You should feed ``foxsi4-commands/systems.json``` into the software by calling like this:

.. code-block:: bash

    ./bin/formatter --config foxsi4-commands/systems.json

The command line arguments are ingested into the software by ``LineInterface``.

If you are using the ``foxsimile`` simulator, pass the foxsimile configuration instead:

.. code-block:: bash
    ./bin/formatter --config foxsi4-commands/foxsimile_systems.json


`foxsi4-commands`
-----------------
There is a whole repository of configuration data, `foxsi4-commands <https://www.github.com/foxsi/foxsi4-commands>`_. This repository contains instructions for modifying, validating, and building configurations for the FOXSI-4 system.

`Parameters.h`
--------------
.. doxygenfile:: Parameters.h
   :project: foxsi-4matter