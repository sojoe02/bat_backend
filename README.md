#PROJECT DESCRIPTION:

This project consists of two parts.

It's designed to be deployed on a Raspberry Pi.

Part 2:
Perl scripts that auto-configures the networking, and ranks every connected device via an zero config, Avahi scheme. A master will be selected logically by comparing MAC addresses.

Part1:
A data grapper(C++), that graps data from a USBDUX device, and places it in a cirular buffer. The user can take snapshots of the buffer via a named pipe.

#PART 1:

##NOTES:

###Rasberry Pi:
* External 2.5" drive need external power supply - needs to be tested more thoroughly.
* The Pi can't allways boot with external drive attached, so it needs to be attached post boot.
* SD card controller apparently use the USB bus bandwidth, which can cause problems with loading data to both an external USB drive and SD card during  sampling above 1.5[Mz].
* Ethernet controller also uses the USB bus.


##TESTING RESULTS:

###Test 1:

First soak test, to test stability of the system under maximum load.

####Parameters:

	Open lid.
	18[hours] continous 3[Mhz] sampling.
	5.30[hour.min] contigous 20[s] snapshots to external 3.5" drive (900Gb).
	
####Result:

	Pass

###Test 2:

Second test is a closed lid temperature test, on 100 snapshots and continous sample.

####Parameters:

	Closed lid.
	100 115[MB] contigous snapshots.
	
####Result:
	
	Min SOC temperature 69C (Start)
	Max SOC temperature 75C (End)
	Pass
	


#Author

Soeren V. Joergensen, svjo@mmmi.sdu.dk
John Hallam.

MMMI, University of Sourthern Denmark.

