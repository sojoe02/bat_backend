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
my $update_freq = 30; #seconds
my $max_run_time = 25;#3600*10; #seconds

#setup the data collection file(s)
my @time = localtime(time);
my $time_string = sprintf("%X", join('',@time));
#$time_string = sprintf("%X",$time_string); #convert it to HEX

say "$time_string";

my $data_temp = "temp.data.$time_string"; #for temperature.

#open the data collection file(s), create they don't exist:
open FILE_temp, ">$data_temp" or die $!;


#read start temperature:
chomp(my $temp = `less /sys/class/thermal/thermal_zone0/temp`);
my $run_time = 0;

#start the adc program and wait for the pipe to appear:
#"adc/build/adc_grapper $FIFO&" ;

#say "Waiting on adc_grapper to create pipe interface:"
#open the pipe and send start_rec command:
open (FIFO, ">$FIFO") || die "can't open $FIFO: $!";

sleep(1);
#start the simple serial snapshotting thread.
print FIFO "start_rec\n";
sleep(1);
print FIFO "simple_snap 10\n";

#monitor the system and stop it if it gets too hot or runtime is over:
while($temp < $max_temp or $run_time < $max_run_time){
		
	chomp($temp = `less /sys/class/thermal/thermal_zone0/temp`);
	print FILE_temp $temp, " ";
	say "$temp";
	$run_time = $update_freq + $run_time;	
	sleep($update_freq);
}

#close the various files and pipes:
close FILE_temp;
#
#close FIFO
print FIFO "exit";
