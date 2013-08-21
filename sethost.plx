#!/usr/bin/perl
#sethost.plx
use warnings;
use strict;
=introcomment
This small script sets the hostname of the recording unit if it's not equal to it's mac address and reboots the unit.
=cut

#retrieve mac address via ifconfig:
my @ifconfig = `ifconfig eth0`;
$_ = $ifconfig[0];
s/^.*(HWaddr\s+)|://g;

if `hostname` ne $_ {
#set hostname to mac address, to ensure uniqueness:
	`echo "$_" > /etc/hostname`;
	`reboot`;
}
