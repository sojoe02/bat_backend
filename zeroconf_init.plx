#!/usr/bin/perl
#zeroconf_init.plx
use warnings;
use strict;

=introcomment
This determines which of the active recording units should attain master status, and will be run during boot sequenze after the hostname has been checked and set.
=cut

#browse for devices running the avahi daemon:
my @avahi_result = `avahi-browse -at`;
#setup containers to store mac addresses and the four final digits of them.
my @valid_mac;
my @valid_mac_final;

foreach (@avahi_result) {
	if (/eth0.*Workstation/ & !/IPv6/) {
		s/\].*//;	#remove everything after mac address
		s/^.*\[|://g;	#remove everything before mac address and remove ':' chars.
		chomp $_;
		push @valid_mac, hex($_);
		s/^\w{8}//; 	#remove the first 8 digits
		push @valid_mac_final, hex($_);
	} 
}


print "valid mac adresses found:\n";

foreach(@valid_mac){
	print "$_\n";
}

print "The four final digits are:\n";

foreach(@valid_mac_final){
	print "$_\n";
}

#print $valid_mac_final[1] + $valid_mac_final[0], "\n";
#my $number = $valid_mac[0] + $valid_mac[1];
#print $number, "\n";
