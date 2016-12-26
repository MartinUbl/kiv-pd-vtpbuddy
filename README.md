# VTPBuddy
VTPBuddy is not your friend. It's your buddy.

Semestral work for KIV/PD - VTPv2 daemon implementation for VLAN database versioning, exporting and backup.

## Prerequisites

* CMake (version 2.8+)
* gcc (version 4.8+)
* make
* subversion or git

## Installation

To compile this project, just navigate to root directory of this repository and use following commands:

```
cmake .
make
```

To install VTPBuddy as service on systemd-based system, you may want to enter command

```
make install
```

Before launching service, be sure to edit file /etc/vtpbuddy.cfg with your desired configuration

