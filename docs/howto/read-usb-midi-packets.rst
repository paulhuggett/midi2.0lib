Read USB-MIDI 1.0 Event Packets
===============================

A USB-MIDI 1.0 device sends and receives 32-bit Event Packets, as described by the USB
Device Class Definition for MIDI Devices, release 1.0. Each packet carries a virtual
cable number, a Code Index Number (CIN), and up to three bytes of MIDI.
:cpp:class:`midi2::bytestream::usbm1_to_bytestream` extracts the bytes for one cable.
Pass them to a :cpp:class:`midi2::bytestream::to_ump` to get UMP:

.. literalinclude:: ../examples/howto_usbm1.cpp
   :language: cpp
   :start-after: SPDX-License-Identifier: MIT

Output:

.. code-block:: text

   20903C40
   20803C00

How it works
------------

* The constructor's argument is the virtual cable number (0 to 15). Packets for other
  cables are ignored, like the second packet here. ``set_cable()`` selects a different
  cable and ``get_cable()`` returns the current one.
* The CIN of each packet determines how many of its three bytes are MIDI data. Packets
  with a reserved CIN (0 or 1) produce nothing.
* To read several cables, use one translator for each, and give each cable's bytes to
  its own :cpp:class:`midi2::bytestream::to_ump`, perhaps with a different group for
  each.
* :cpp:func:`reset() <midi2::bytestream::usbm1_to_bytestream::reset>` discards any
  waiting output and also sets the cable number back to 0.
