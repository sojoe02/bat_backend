#!/usr/bin/perl
use warnings;
use strict;
use v5.10.0;
=introcomment
This is a script that is designed to test the adc conversion and collect relevant system data . Such as CPU utilization and CPU temperature.
=cut

#setup testing variables:
my $max_temp = 85; #Celcius
my $FIFO = '/home/pi/grapper.cmd';
my $update_freq = 5; #seconds
my $max_run_time = 3600*10 #seconds

#read start temperature:
chomp(my $temp = `less /sys/class/thermal/thermal_zone0/temp`);
my $run_time = 0;

#start the adc program and wait for the pipe to appear:
system("adc/build/adc_grapper", "$FIFO");

say "Waiting on adc_grapper to create pipe interface:"
#open the pipe and send start_rec command:
open (FIFO, ">$FIFO") || die "can't open $FIFO: $!";

#start the sampling.
print FIFO "start_rec"

#start the simple serial snapshotting thread.


#monitor the system and stop it if it gets too hot or runtime is over:
while($temp < $max_temp or $run_time < $max_run_time){
	
	$temp = `less /sys/class/thermal/thermal_zone0/temp`;

	$run_time = $update_freq + $runtime;	
	sleep($update_freq);

}
