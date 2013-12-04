This project consists of two parts.

It's designed to be deployed on a Raspberry Pi.

Part1:
Perl scripts that auto-configures the networking, and ranks every connected device via an zero config, Avahi scheme. A master will be selected logically by comparing MAC addresses.

Part 2:
A data grapper(C++), that graps data from a USBDUX device, and places it in a cirular buffer. The user can take snapshots of the buffer via a named pipe.

Author: Soeren V. Joergensen, svjo@mmmi.sdu.dk
Organization: MMMI, University of Sourthern Denmark.
