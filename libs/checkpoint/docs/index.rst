..
    Copyright (c) 2019 The STE||AR-Group

    SPDX-License-Identifier: BSL-1.0
    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

.. _libs_checkpoint:

==========
checkpoint
==========

A common need of users is to periodically backup an application. This practice
provides resiliency and potential restart points in code. We have developed the
concept of a ``checkpoint`` to support this use case.

Found in ``hpx/util/checkpoint.hpp``, ``checkpoint``\ s are defined as objects
which hold a serialized version of an object or set of objects at a particular
moment in time. This representation can be stored in memory for later use or it
can be written to disk for storage and/or recovery at a later point. In order to
create and fill this object with data we use a function called
``save_checkpoint``. In code the function looks like this::

    hpx::future<hpx::util::checkpoint> hpx::util::save_checkpoint(a, b, c, ...);

``save_checkpoint`` takes arbitrary data containers such as int, double, float,
vector, and future and serializes them into a newly created ``checkpoint``
object. This function returns a ``future`` to a ``checkpoint`` containing the
data. Let us look a simple use case below::

    using hpx::util::checkpoint;
    using hpx::util::save_checkpoint;

    std::vector<int> vec{1,2,3,4,5};
    hpx::future<checkpoint> save_checkpoint(vec);

Once the future is ready the checkpoint object will contain the ``vector``
``vec`` and its five elements.

It is also possible to modify the launch policy used by ``save_checkpoint``.
This is accomplished by passing a launch policy as the first argument. It is
important to note that passing ``hpx::launch::sync`` will cause
``save_checkpoint`` to return a ``checkpoint`` instead of a ``future`` to a
``checkpoint``. All other policies passed to ``save_checkpoint`` will return a
``future`` to a ``checkpoint``.

Sometimes ``checkpoint`` s must be declared before they are used.
``save_checkpoint`` allows users to move pre-created ``checkpoint`` s into the
function as long as they are the first container passing into the function (In
the case where a launch policy is used, the ``checkpoint`` will immediately
follow the launch policy). An example of these features can be found below:

.. literalinclude:: ../../../../libs/tests/unit/checkpoint.cpp
   :language: c++
   :lines: 27-38

Now that we can create ``checkpoint`` s we now must be able to restore the
objects they contain into memory. This is accomplished by the function
``restore_checkpoint``. This function takes a ``checkpoint`` and fills its data
into the containers it is provided. It is important to remember that the
containers must be ordered in the same way they were placed into the
``checkpoint``. For clarity see the example below:

.. literalinclude:: ../../../../libs/tests/unit/checkpoint.cpp
   :language: c++
   :lines: 41-49

The core utility of ``checkpoint`` is in its ability to make certain data
persistent. Often this means that the data is needed to be stored in an object,
such as a file, for later use. For these cases we have provided two solutions:
stream operator overloads and access iterators.

We have created the two stream overloads
``operator<<`` and ``operator>>`` to stream data
out of and into ``checkpoint``. You can see an
example of the overloads in use below:

.. literalinclude:: ../../../../libs/tests/unit/checkpoint.cpp
   :language: c++
   :lines: 176-186

This is the primary way to move data into and out of a ``checkpoint``. It is
important to note, however, that users should be cautious when using a stream
operator to load data an another function to remove it (or vice versa). Both
``operator<<`` and ``operator>>`` rely on a ``.write()`` and a ``.read()``
function respectively. In order to know how much data to read from the
``std::istream``, the ``operator<<`` will write the size of the ``checkpoint``
before writing the ``checkpoint`` data. Correspondingly, the ``operator>>`` will
read the size of the stored data before reading the data into new instance of
``checkpoint``. As long as the user employs the ``operator<<`` and
``operator>>`` to stream the data this detail can be ignored.

.. important::

   Be careful when mixing ``operator<<`` and ``operator>>`` with other
   facilities to read and write to a ``checkpoint``. ``operator<<`` writes and
   extra variable and ``operator>>`` reads this variable back separately. Used
   together the user will not encounter any issues and can safely ignore this
   detail.

Users may also move the data into and out of a ``checkpoint`` using the exposed
``.begin()`` and ``.end()`` iterators. An example of this use case is
illustrated below.

.. literalinclude:: ../../../../libs/tests/unit/checkpoint.cpp
   :language: c++
   :lines: 129-150

Checkpointing Components
------------------------

``save_checkpoint`` and ``restore_checkpoint`` are also able to store components
inside ``checkpoint``s. This can be done in one of two ways. First a client of
the component can be passed to ``save_checkpoint``. When the user wishes to
resurrect the component she can pass a client instance to ``restore_checkpoint``.

This technique is demonstrated below:

.. literalinclude:: ../../../../libs/tests/unit/checkpoint.cpp
   :language: c++
   :lines: 143-144

The second way a user can save a component is by passing a ``shared_ptr`` to the
component to ``save_checkpoint``. This component can be resurrected by creating
a new instance of the component type and passing a ``shared_ptr`` to the new
instance to ``restore_checkpoint``. An example can be found below:

This technique is demonstrated below:

.. literalinclude:: ../../../../libs/tests/unit/checkpoint.cpp
   :language: c++
   :lines: 113-126


