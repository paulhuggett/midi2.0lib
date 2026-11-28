Choose message handlers at run time
===================================

A dispatcher's configuration usually names its backends' types, so the handlers are
fixed when the program is compiled. To choose them while the program runs, give the
configuration a reference to one of the ``_pure`` backends, whose handlers are pure
virtual functions, and derive your handlers from the matching ``_base`` backend, which
implements every handler to do nothing:

.. literalinclude:: ../examples/howto_backend_base.cpp
   :language: cpp
   :start-after: SPDX-License-Identifier: MIT

Output:

.. code-block:: console

   $ ./howto_backend_base
   note on 60
   note on 64
   $ ./howto_backend_base count
   1 note held

How it works
------------

* :cpp:struct:`m2cvm_base\<state> <midi2::ump::dispatcher_backend::m2cvm_base>`
  derives from :cpp:struct:`m2cvm_pure\<state> <midi2::ump::dispatcher_backend::m2cvm_pure>`
  and overrides every handler with one that does nothing. ``logger`` and ``counter``
  override only the handlers that they need.
* The ``m2cvm`` member of ``config`` is a reference to the ``_pure`` class. The
  dispatcher's concepts accept it, because the handlers can be called through the
  reference. Each call is a virtual function call.
* Each handler of a ``_pure`` or ``_base`` backend is a non-static member function, so
  the backend object can hold state of its own, like ``counter::notes``.
* The other groups of messages use ``_null`` backends, which cost nothing. Mix the kinds
  of backend however you like.

There is a ``_pure`` and a ``_base`` class for every group of UMP messages, in
``midi2::ump::dispatcher_backend``, and for every group of MIDI-CI messages, in
``midi2::ci::dispatcher_backend``. The library's unit tests use the ``_pure`` classes to
define mock objects.

The ``function`` backends, used in the :doc:`tutorial <../tutorial>`, also let you
choose handlers at run time, by installing a different ``std::function<>``. Unlike a
``std::function<>``, a class derived from a ``_base`` backend never allocates memory.
