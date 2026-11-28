Carry MIDI-CI over UMP
======================

MIDI-CI messages are Universal System Exclusive messages, so over UMP they travel as
SysEx7 packets on the group of the function block that implements MIDI-CI. To receive
them, give the data bytes of each packet to a :cpp:class:`midi2::ci::ci_dispatcher`. To
send one, write it with :cpp:func:`midi2::ci::create_message` and split the bytes into
packets. This program is a MIDI-CI responder that answers Inquiry: Endpoint:

.. literalinclude:: ../examples/howto_ci_over_ump.cpp
   :language: cpp
   :start-after: SPDX-License-Identifier: MIT

Output:

.. code-block:: text

   inquiry: 30167E7F 0D720210 30267159 02566848 30320000 00000000
   Inquiry: Endpoint from MUID 0x567890, status 0
   reply: 30167E7F 0D730256 30266848 00107159 30260200 07004D49 30354449 322D3100

Receiving
---------

* The UMP dispatcher's ``data64`` handlers pass the bytes of each packet to
  ``device::sysex7()``. A ``sysex7_in_1`` or ``sysex7_start`` packet begins a new
  message, so it calls ``reset()`` first. Without that call, the MIDI-CI dispatcher would
  ignore everything after the first message.
* Packets on other groups are ignored. If several groups carry MIDI-CI, give each one its
  own MIDI-CI dispatcher, because the bytes of messages on different groups can be
  interleaved.
* The MIDI-CI dispatcher's constructor takes the Device ID that it records in the header
  passed to each handler, the group that it passes to ``check_muid()``, and the
  configuration.
* ``check_muid()`` is called with the Destination MUID of each message that isn't sent
  to the broadcast MUID. This one accepts only messages for this device on its MIDI-CI
  group.
* The dispatcher's buffer must hold the largest message body that the device accepts.
  Here, it is the same as the largest message the device sends, ``max_sysex_size``. A
  message that doesn't fit is reported to the ``buffer_overflow()`` handler and
  discarded. :doc:`exchange-property-data` shows how to receive Property Exchange
  messages that are larger than the buffer.

Sending
-------

* ``create_message()`` writes to a range given as an iterator and a sentinel. It returns
  an iterator past the last byte written. If the message doesn't fit, it returns the end
  of the range, but a message that exactly fills the range does too. So the buffer is
  one byte larger than the largest message, and reaching the end means that the message
  is too large.
* ``send_sysex7()`` splits the bytes into packets, as described in
  :doc:`send-and-receive-sysex7`. The projection passed to ``data()`` converts each
  ``std::byte`` to the integer type of a packet's data bytes.
* The reply's header has this device's MUID as its Source MUID (``remote_muid``) and the
  inquirer's MUID as its Destination MUID (``local_muid``).
* :cpp:struct:`midi2::ci::endpoint_reply` refers to its information through a
  ``std::span<b7 const>``: the bytes must outlive the call to ``create_message()``.

The device
----------

``device`` holds a pointer to itself in its MIDI-CI dispatcher's context, so it deletes
its copy and move operations. The UMP dispatcher's context is a pointer to ``dev``.
:doc:`share-state-with-handlers` describes the other ways to provide a context.
