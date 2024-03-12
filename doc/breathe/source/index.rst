.. foxsi-4matter documentation master file, created by
   sphinx-quickstart on Thu Mar  7 15:13:50 2024.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to the ``foxsi-4matter`` documentation
==============================================

Overview
--------

Hmmm. I should write something here.

"Quick" start
-----------
   
Reference PISETUP.md

Dependencies
~~~~~~~~~~~~

* `Boost::asio <https://www.boost.org/doc/libs/1_84_0/doc/html/boost_asio.html>`_
* `nlohmann::json <https://github.com/nlohmann/json>`_
* `cameron314::concurrentqueue <https://github.com/cameron314/concurrentqueue>`_
   * This is a fast, lock-free, thread-safe, multi-producer/multi-consumer queue. I use it just as a multi-producer/single-consumer queue, but it enables simultaneous safe access to the downlink queue from all systems. 
* `gabime::spdlog <https://github.com/gabime/spdlog>`_

Configuration
-------------

A healthy amount of configuration data is used to feed this software information about its physical configuration. This includes parameters on time limits and latencies, expected behavior during erros, network topology and node configuration, communication volumes and rates, etc.

Some default parameters are defined in ``Parameters.h``, and the bulk of configuration information is contained in a separate repository, ``foxsi4-commands``. 

Here is the network configuration:

.. image:: ../../assets/formatter_layout.svg

An overview of the system configuration data can be found here:

.. toctree::
   :maxdepth: 1

   system_config.rst

Class reference
---------------
.. toctree::
   :maxdepth: 2

   class_reference/circle
   class_reference/layers
   class_reference/buffers
   class_reference/commanding
   class_reference/systems
   class_reference/timing
   class_reference/line_interface

Namespace reference
-------------------
.. toctree::
   :maxdepth: 2
   
   .. namespace_reference/namespaces

Indices and tables
==================

* :ref:`genindex`
* :ref:`search`
