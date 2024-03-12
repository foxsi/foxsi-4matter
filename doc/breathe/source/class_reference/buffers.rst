Buffers
=======

Overview
--------
These objects provide means to package messages going into and out of the Formatter over different interfaces:
* ``DownlinkBufferElement`` is a single message to be sent by the Formatter over telemetry to the ground.
* ``UplinkBufferElement`` is a single message sent over uplink to the Formatter, from the ground.
* ``PacketFramer`` provides methods for the Formatter to send large amounts of data (frames) in smaller pieces (packets) over telemetry to the ground.
* ``FramePacketizer`` provides methods for the Formatter to request and reassemble large amounts of data (frames) from smaller pieces (packets) which are requested from detector systems onboard.
* ``RingBufferParameters`` is a container for several parameters that are useful when interacting with remote memory in detector systems onboard. Often used to define components of ``FramePacketizer``.

``DownlinkBufferElement``
-------------------------
.. doxygenclass:: DownlinkBufferElement
   :project: foxsi-4matter
   :members:

``UplinkBufferElement``
-----------------------
.. doxygenclass:: UplinkBufferElement
   :project: foxsi-4matter
   :members:

``PacketFramer``
----------------
.. doxygenclass:: PacketFramer
   :project: foxsi-4matter
   :members:

``FramePacketizer``
-------------------
.. doxygenclass:: FramePacketizer
   :project: foxsi-4matter
   :members:

``RingBufferParameters``
------------------------
.. doxygenclass:: RingBufferParameters
    :project: foxsi-4matter
    :members: