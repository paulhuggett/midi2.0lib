Exchange property data
======================

The Property Exchange messages that carry data (Get, Set, Subscription, and Notify, and
their replies) have a JSON header and property data, which together can be much larger
than any other MIDI-CI message. A dispatcher whose configuration has a
``property_exchange_stream`` backend passes these messages to that backend in pieces, so
they can be larger than the dispatcher's buffer. This program builds a Reply to Get
Property Data, and receives it with a dispatcher whose buffer is only 16 bytes:

.. literalinclude:: ../examples/howto_pe.cpp
   :language: cpp
   :start-after: SPDX-License-Identifier: MIT

Output:

.. code-block:: text

   start: request 1, 14 bytes of header
     header piece of 14 bytes
   header: {"status":200}
   chunk 1 of 1, 83 bytes of property data
     property data piece of 16 bytes
     property data piece of 16 bytes
     property data piece of 16 bytes
     property data piece of 16 bytes
     property data piece of 16 bytes
     property data piece of 3 bytes
   property data: {"manufacturerId":[125,0,0],"familyId":[1,0],"modelId":[2,0],"versionId":[1,0,0,0]}
   end

Building a message
------------------

* ``midi2::ci::property_exchange::get_reply`` and the other six message types are
  specializations of the class template :cpp:class:`midi2::ci::property_exchange::property_exchange`.
  ``make()`` takes the chunk information, the Request ID, the header data, and the
  property data.
* The header and property data are spans that refer to your memory: the message doesn't
  copy them. The header is a ``std::span<char const>``; the property data is a
  ``std::span<std::byte const>``, because it may not be text if the header's
  ``mutualEncoding`` property says so.
* ``create_message()`` writes the message, filling in the header and data lengths.
* To send property data in several chunks, write a message for each chunk, numbered
  from 1.

Receiving a message in pieces
-----------------------------

:cpp:struct:`midi2::ci::function_stream_config` adds a ``property_exchange_stream``
backend to :cpp:struct:`midi2::ci::function_config`. For each message, the dispatcher
calls its handlers in this order:

#. ``start()``, once, with the message type, the Request ID, and the length of the
   header data.
#. ``header_data()``, zero or more times, with the next piece of the header.
#. ``chunk()``, once, with the chunk information and the length of the property data in
   this chunk.
#. ``property_data()``, zero or more times, with the next piece of the property data.
#. ``end()``, once.

Each piece is at most the size of the buffer. If the message is abandoned after
``start()``, for example because ``reset()`` is called, ``abort()`` is called instead of
``end()``. The handlers are called as soon as each piece arrives, so you can process the
data as it streams past, for example by giving it to an incremental JSON parser. The
``pe_viewer`` and ``pe_zlib_viewer`` demo programs pass the pieces to the
`peejay <https://github.com/paulhuggett/peejay>`_ JSON parser, and the second also
decodes Mcoded7 and decompresses the data as it arrives.

The other Property Exchange messages (Inquiry: Property Exchange Capabilities and its
reply) are small. They are still passed whole to the ``property_exchange`` backend.

Without a ``property_exchange_stream`` backend, the seven messages are passed whole to the
``get()``, ``get_reply()``, and other handlers of the ``property_exchange`` backend, and
each message must fit in the buffer.
