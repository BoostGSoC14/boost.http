# Boost.Http

```
 ,ggggggggggg,                                    ,ggg,        gg
dP"""88""""""Y8,                              I8 dP""Y8b       88  I8     I8
Yb,  88      `8b                              I8 Yb, `88       88  I8     I8
 `"  88      ,8P                           8888888`"  88       8888888888888888
     88aaaad8P"                               I8      88aaaaaaa88  I8     I8
     88""""Y8ba   ,ggggg,   ,ggggg,   ,g,     I8      88"""""""88  I8     I8  gg,gggg,
     88      `8b dP"  "Y8ggdP"  "Y8gg,8'8,    I8      88       88  I8     I8  I8P"  "Yb
     88      ,8Pi8'    ,8Ii8'    ,8I,8'  Yb  ,I8,     88       88 ,I8,   ,I8, I8'    ,8i
     88_____,d8,d8,   ,d8,d8,   ,d8,8'_   8),d88b,    88       Y8,d88b, ,d88b,I8 _  ,d8'
    88888888P" P"Y8888P" P"Y8888P" P' "YY8P88P""Y8    88       `Y8P""Y8 8P""YPI8 YY88888P
                                                                              I8
                                                                              I8
                                                                              I8
                                                                              I8
                                                                              I8
                                                                              I8
```

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

* CMake for build
* Boost libraries

## Documentation

...
