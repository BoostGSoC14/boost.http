# Boost.Http

This library aims to implement a core HTTP server for Boost that can be used
from resource-constrained devices to powerful machines that have plenty of
resources to make use of and can speed up the server (such as extra ram
available to pools and extra cpus available to multithreaded servers).

A second aim of the library is to support several http transport mechanisms, but
continue to expose http power (100-continue, chunked entities, upgrade, ...).
Thanks to the first requirement, runtime polymorphism is avoided, but support
for runtime polymorphism (same handler replying from an embeddedable server and
CoAPP) is planned for later.

This library is being developed as part of GSoC 2014. Read the [full proposal
here](https://github.com/vinipsmaker/gsoc2014-boost).

# LICENSE

This library is licensed under the terms of the Boost Software License, version
1.0. You can find a copy of the license with this library.

# Building

## Dependencies

* Boost Build for build
* Boost libraries

## Documentation

...
