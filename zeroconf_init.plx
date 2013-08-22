#!/usr/bin/perl
#zeroconf_init.plx
use warnings;
use strict;
use v5.10.0;
no warnings 'portable';  # Support for 64-bit ints required

=introcomment
This determines which of the active recording units should attain master status, and will be run during boot sequenze after the hostname has been checked and set.
=cut

my $status; # am I master or slave.

say "I rock";
say "and other stuff";
# #
# 
# #
sub request_zeroconf_ip {
	# Aquire valid ipv4LL address via avahi.
	system "avahi-autoipd -D -w";
}

# #						
# sub rutine to determine which device is master
# and whether this device is it				
# #					
sub determine_master {
	#browse for devices running the avahi daemon:
	my @avahi_result = `avahi-browse -at`;
	#setup containers to store mac addresses and the four final digits of them.
	my @valid_mac;
	foreach (@avahi_result) {
		if (/eth0.*Workstation/ and !/IPv6/) {
			s/\].*//;	#remove everything after mac address
			s/^.*\[|://g;	#remove everything before mac address and remove ':' chars.
			chomp $_;
			push @valid_mac, $_;
			#s/^\w{6}//; 	#remove the first 6 digits (32bit compatibility)
			#push @valid_mac_final_six, $_;
		} 
	}
	say "Valid mac adresses found";
	say "$_" foreach @valid_mac;
	#say "The six final bytes are(in hex)";
	#say "$_"foreach @valid_mac_final_six;
	say "Sorting the valid mac address list";
	#setup sorting rule
	sub by_string_as_hex{
		if (hex($a) < hex($b)){-1} elsif (hex($a) > hex($b)){1} else {0}
	}
	@valid_mac = sort by_string_as_hex @valid_mac;
	say "$_" foreach @valid_mac;

	# Next the script figures out whether it has the lowest mac 
	# address if it does it will assign itself as master
	my @ifconfig = `ifconfig eth0`;
	$_ = $ifconfig[0];
	s/^.*(HWaddr\s+)|:|\s*//g; ## remove everything but the MAC address.
	#return a string that indicates whether it's master or slave:
	if($_ eq $valid_mac[0]){"master"} else {"slave"};
}

sub stop_master_service{
	system "ip addr del 192.168.0.1/24 dev eth0";
	#Stop the catalyst backend, apache service etc.
}



