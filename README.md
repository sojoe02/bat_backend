PROJECT DESCRIPTION:

This project consists of two parts.

It's designed to be deployed on a Raspberry Pi.

Part1:
Perl scripts that auto-configures the networking, and ranks every connected device via an zero config, Avahi scheme. A master will be selected logically by comparing MAC addresses.

Part 2:
A data grapper(C++), that graps data from a USBDUX device, and places it in a cirular buffer. The user can take snapshots of the buffer via a named pipe.

TESTING RESULTS:
----------------------------------------
ADC soak test 1:

Parameters:

	18[hours] continous 3[Mhz] sampling.
	5.30[hour.min] contigous 20[s] snapshots to external 3.5" drive (900Gb).
	
Result:

	Pass
----------------------------------------



Author: Soeren V. Joergensen, svjo@mmmi.sdu.dk
Organization: MMMI, University of Sourthern Denmark.
